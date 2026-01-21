#include "DeviceManager.h"
#include <algorithm>
#include <cmath>

DeviceManager::DeviceManager(DeviceStateModel& stateModel)
    : stateModel(stateModel)
{
    meterData.inputRMS.resize(maxChannels, 0.0f);
    meterData.inputPeak.resize(maxChannels, 0.0f);
    meterData.outputRMS.resize(maxChannels, 0.0f);
    meterData.outputPeak.resize(maxChannels, 0.0f);
    
    initializeDeviceManager();
}

DeviceManager::~DeviceManager()
{
    // Mark as shutting down to prevent callbacks
    isShuttingDown.store(true);
    
    // Remove change listener first to prevent callbacks during destruction
    audioDeviceManager.removeChangeListener(this);
    
    // Remove audio callback
    audioDeviceManager.removeAudioCallback(this);
    
    // Close device (only if on message thread)
    if (juce::MessageManager::getInstance()->isThisTheMessageThread())
    {
        audioDeviceManager.closeAudioDevice();
    }
    
    deviceIsActive.store(false);
}

void DeviceManager::initializeDeviceManager()
{
    audioDeviceManager.addChangeListener(this);
    
    // Don't auto-select device - let user choose
}

juce::StringArray DeviceManager::getAvailableInputDevices() const
{
    juce::StringArray devices;
    auto& deviceTypes = const_cast<DeviceManager*>(this)->audioDeviceManager.getAvailableDeviceTypes();
    
    for (auto* deviceType : deviceTypes)
    {
        deviceType->scanForDevices();
        auto inputDevices = deviceType->getDeviceNames(true);
        for (const auto& device : inputDevices)
        {
            if (!devices.contains(device))
                devices.add(device);
        }
    }
    
    return devices;
}

juce::StringArray DeviceManager::getAvailableOutputDevices() const
{
    juce::StringArray devices;
    auto& deviceTypes = const_cast<DeviceManager*>(this)->audioDeviceManager.getAvailableDeviceTypes();
    
    for (auto* deviceType : deviceTypes)
    {
        deviceType->scanForDevices();
        auto outputDevices = deviceType->getDeviceNames(false);
        for (const auto& device : outputDevices)
        {
            if (!devices.contains(device))
                devices.add(device);
        }
    }
    
    return devices;
}

juce::StringArray DeviceManager::getAvailableFullDuplexDevices() const
{
    juce::StringArray devices;
    auto inputDevices = getAvailableInputDevices();
    auto outputDevices = getAvailableOutputDevices();
    
    // Full-duplex devices are those that appear in both lists
    for (const auto& device : inputDevices)
    {
        if (outputDevices.contains(device) && !devices.contains(device))
            devices.add(device);
    }
    
    return devices;
}

bool DeviceManager::selectInputDevice(const juce::String& inputDeviceName)
{
    if (isSwitching.load() || isShuttingDown.load())
        return false;
    
    // Must be called from message thread
    if (!juce::MessageManager::getInstance()->isThisTheMessageThread())
    {
        juce::MessageManager::callAsync([this, inputDeviceName]()
        {
            selectInputDevice(inputDeviceName);
        });
        return true;
    }
    
    juce::ScopedLock lock(deviceSwitchLock);
    isSwitching.store(true);
    
    // Remove callback BEFORE switching (JUCE will handle device stop/start internally)
    // This is critical to prevent callbacks during device switching
    audioDeviceManager.removeAudioCallback(this);
    deviceIsActive.store(false);
    
    // CRITICAL: Clear all meter data from previous device to prevent freeze
    for (int i = 0; i < maxChannels; ++i)
    {
        stateModel.updateChannelLevel(i, true, 0.0f, 0.0f);
        stateModel.updateChannelLevel(i, false, 0.0f, 0.0f);
    }
    
    // Get current setup - JUCE's setAudioDeviceSetup will handle stopDevice() internally
    auto setup = audioDeviceManager.getAudioDeviceSetup();
    
    // Set input device - CRITICAL: Must explicitly enable input channels
    if (inputDeviceName.isNotEmpty())
    {
        setup.inputDeviceName = inputDeviceName;
        setup.inputChannels.clear();
        
        // Get available channels from the device type
        auto& deviceTypes = audioDeviceManager.getAvailableDeviceTypes();
        int numInputChannels = 0;
        
        for (auto* deviceType : deviceTypes)
        {
            deviceType->scanForDevices();
            auto inputDevices = deviceType->getDeviceNames(true);
            if (inputDevices.contains(inputDeviceName))
            {
                // Create a temporary device to query available channels
                auto* testDevice = deviceType->createDevice(inputDeviceName, inputDeviceName);
                if (testDevice != nullptr)
                {
                    auto channelNames = testDevice->getInputChannelNames();
                    numInputChannels = channelNames.size();
                    
                    if (numInputChannels > 0)
                    {
                        // CRITICAL: Enable ALL available input channels explicitly
                        setup.inputChannels.setRange(0, numInputChannels, true);
                        setup.useDefaultInputChannels = false;
                        juce::Logger::writeToLog("✓ Explicitly enabling " + juce::String(numInputChannels) + 
                                                 " input channels for " + inputDeviceName);
                    }
                    delete testDevice;
                    break;
                }
            }
        }
        
        if (numInputChannels == 0)
        {
            juce::Logger::writeToLog("⚠ WARNING: No input channels found for " + inputDeviceName);
        }
    }
    else
    {
        setup.inputDeviceName = juce::String();
        setup.useDefaultInputChannels = false;
        setup.inputChannels.clear();
    }
    
    // User must explicitly select output - don't auto-select
    // If no output is selected, JUCE will handle it (may use system default or fail gracefully)
    
    juce::Logger::writeToLog("About to setAudioDeviceSetup - Input: " + setup.inputDeviceName + 
                             " Output: " + setup.outputDeviceName);
    
    // IMPORTANT: Re-add callback BEFORE setAudioDeviceSetup
    // JUCE's setAudioDeviceSetup will call audioDeviceAboutToStart on all registered callbacks
    // If we add it after, we might miss the audioDeviceAboutToStart call
    audioDeviceManager.addAudioCallback(this);
    juce::Logger::writeToLog("Callback added before setAudioDeviceSetup");
    
    // Apply setup - JUCE will:
    // 1. Call stopDevice() internally
    // 2. Delete/create device if needed
    // 3. Open and start the device
    // 4. Call audioDeviceAboutToStart on all registered callbacks (including us)
    juce::String error = audioDeviceManager.setAudioDeviceSetup(setup, true);
    
    juce::Logger::writeToLog("setAudioDeviceSetup returned - Error: " + (error.isEmpty() ? "none" : error));
    
    // CRITICAL: Reset isSwitching BEFORE updateDeviceInfo to prevent freeze
    isSwitching.store(false);
    
    if (error.isEmpty())
    {
        currentInputDeviceName = inputDeviceName;
        
        // Verify device is running
        auto* device = audioDeviceManager.getCurrentAudioDevice();
        if (device != nullptr && device->isOpen())
        {
            deviceIsActive.store(true);
            juce::Logger::writeToLog("Input device selected successfully: " + inputDeviceName);
        }
        else
        {
            juce::Logger::writeToLog("ERROR: Device selected but not open!");
        }
        
        updateDeviceInfo();
        return true;
    }
    else
    {
        juce::Logger::writeToLog("ERROR setting input device: " + error);
        return false;
    }
}

bool DeviceManager::selectOutputDevice(const juce::String& outputDeviceName)
{
    if (isSwitching.load() || isShuttingDown.load())
        return false;
    
    // Must be called from message thread
    if (!juce::MessageManager::getInstance()->isThisTheMessageThread())
    {
        juce::MessageManager::callAsync([this, outputDeviceName]()
        {
            selectOutputDevice(outputDeviceName);
        });
        return true;
    }
    
    juce::ScopedLock lock(deviceSwitchLock);
    isSwitching.store(true);
    
    // Remove callback BEFORE switching (JUCE will handle device stop/start internally)
    // This is critical to prevent callbacks during device switching
    audioDeviceManager.removeAudioCallback(this);
    deviceIsActive.store(false);
    
    // CRITICAL: Clear all meter data from previous device to prevent freeze
    for (int i = 0; i < maxChannels; ++i)
    {
        stateModel.updateChannelLevel(i, true, 0.0f, 0.0f);
        stateModel.updateChannelLevel(i, false, 0.0f, 0.0f);
    }
    
    // Get current setup - JUCE's setAudioDeviceSetup will handle stopDevice() internally
    auto setup = audioDeviceManager.getAudioDeviceSetup();
    
    // Set output device - CRITICAL: Must explicitly enable output channels
    if (outputDeviceName.isNotEmpty())
    {
        setup.outputDeviceName = outputDeviceName;
        setup.outputChannels.clear();
        
        // Get available channels from the device type
        auto& deviceTypes = audioDeviceManager.getAvailableDeviceTypes();
        int numOutputChannels = 0;
        
        for (auto* deviceType : deviceTypes)
        {
            deviceType->scanForDevices();
            auto outputDevices = deviceType->getDeviceNames(false);
            if (outputDevices.contains(outputDeviceName))
            {
                // Create a temporary device to query available channels
                auto* testDevice = deviceType->createDevice(outputDeviceName, outputDeviceName);
                if (testDevice != nullptr)
                {
                    auto channelNames = testDevice->getOutputChannelNames();
                    numOutputChannels = channelNames.size();
                    
                    if (numOutputChannels > 0)
                    {
                        // CRITICAL: Enable ALL available output channels (like input)
                        setup.outputChannels.setRange(0, numOutputChannels, true);
                        setup.useDefaultOutputChannels = false;
                        juce::Logger::writeToLog("✓ Explicitly enabling " + juce::String(numOutputChannels) + 
                                                 " output channels for " + outputDeviceName);
                    }
                    delete testDevice;
                    break;
                }
            }
        }
        
        if (numOutputChannels == 0)
        {
            juce::Logger::writeToLog("⚠ WARNING: No output channels found for " + outputDeviceName);
        }
    }
    else
    {
        setup.outputDeviceName = juce::String();
        setup.useDefaultOutputChannels = false;
        setup.outputChannels.clear();
    }
    
    // User must explicitly select input - don't auto-select
    // Keep current input device if already selected
    
    // IMPORTANT: Re-add callback BEFORE setAudioDeviceSetup
    // JUCE's setAudioDeviceSetup will call audioDeviceAboutToStart on all registered callbacks
    // If we add it after, we might miss the audioDeviceAboutToStart call
    audioDeviceManager.addAudioCallback(this);
    
    // Apply setup - JUCE will:
    // 1. Call stopDevice() internally
    // 2. Delete/create device if needed
    // 3. Open and start the device
    // 4. Call audioDeviceAboutToStart on all registered callbacks (including us)
    juce::String error = audioDeviceManager.setAudioDeviceSetup(setup, true);
    
    // CRITICAL: Reset isSwitching BEFORE updateDeviceInfo to prevent freeze
    isSwitching.store(false);
    
    if (error.isEmpty())
    {
        currentOutputDeviceName = outputDeviceName;
        
        // Verify device is running
        auto* device = audioDeviceManager.getCurrentAudioDevice();
        if (device != nullptr && device->isOpen())
        {
            deviceIsActive.store(true);
            juce::Logger::writeToLog("Output device selected successfully: " + outputDeviceName);
        }
        else
        {
            juce::Logger::writeToLog("ERROR: Device selected but not open!");
        }
        
        updateDeviceInfo();
        return true;
    }
    else
    {
        juce::Logger::writeToLog("ERROR setting output device: " + error);
        return false;
    }
}

juce::String DeviceManager::getCurrentInputDeviceName() const
{
    auto* device = audioDeviceManager.getCurrentAudioDevice();
    if (device != nullptr)
    {
        auto setup = audioDeviceManager.getAudioDeviceSetup();
        return setup.inputDeviceName;
    }
    return currentInputDeviceName;
}

juce::String DeviceManager::getCurrentOutputDeviceName() const
{
    auto* device = audioDeviceManager.getCurrentAudioDevice();
    if (device != nullptr)
    {
        auto setup = audioDeviceManager.getAudioDeviceSetup();
        return setup.outputDeviceName;
    }
    return currentOutputDeviceName;
}

bool DeviceManager::isDeviceActive() const
{
    return deviceIsActive.load();
}

void DeviceManager::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                      int numInputChannels,
                                                      float* const* outputChannelData,
                                                      int numOutputChannels,
                                                      int numSamples,
                                                      const juce::AudioIODeviceCallbackContext& context)
{
    // Real-time safe: no allocations, no locks
    
    if (isShuttingDown.load())
        return;
    
    // Real-time safe: no logging in audio callback
    
    // Calculate RMS and Peak for inputs FIRST (before clearing outputs)
    for (int ch = 0; ch < numInputChannels && ch < maxChannels; ++ch)
    {
        if (inputChannelData[ch] != nullptr)
        {
            float sumSquared = 0.0f;
            float peak = 0.0f;
            
            for (int i = 0; i < numSamples; ++i)
            {
                const float sample = inputChannelData[ch][i];
                sumSquared += sample * sample;
                peak = std::max(peak, std::abs(sample));
            }
            
            float rms = std::sqrt(sumSquared / static_cast<float>(numSamples));
            meterData.inputRMS[ch] = rms;
            meterData.inputPeak[ch] = peak;
            
            // Update state model (atomic, lock-free)
            stateModel.updateChannelLevel(ch, true, rms, peak);
        }
        else
        {
            // Channel is null, set to zero
            stateModel.updateChannelLevel(ch, true, 0.0f, 0.0f);
        }
    }
    
    // Clear unused input channels
    for (int ch = numInputChannels; ch < maxChannels; ++ch)
    {
        stateModel.updateChannelLevel(ch, true, 0.0f, 0.0f);
    }
    
    // Clear all output channels (no loopback/passthrough)
    // In a professional audio analysis app, we only monitor inputs
    for (int ch = 0; ch < numOutputChannels; ++ch)
    {
        if (outputChannelData[ch] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
    }
    
    // Output channels are always zero (we don't play anything)
    // Set all output meters to zero
    for (int ch = 0; ch < numOutputChannels && ch < maxChannels; ++ch)
    {
        meterData.outputRMS[ch] = 0.0f;
        meterData.outputPeak[ch] = 0.0f;
        stateModel.updateChannelLevel(ch, false, 0.0f, 0.0f);
    }
    
    // Clear unused output channels
    for (int ch = numOutputChannels; ch < maxChannels; ++ch)
    {
        stateModel.updateChannelLevel(ch, false, 0.0f, 0.0f);
    }
}

void DeviceManager::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    // CRITICAL: Only ignore if shutting down, NOT if switching
    // During switching, we WANT this callback to configure the new device
    if (isShuttingDown.load())
        return;
    
    if (device == nullptr)
    {
        juce::Logger::writeToLog("audioDeviceAboutToStart: device is null!");
        return;
    }
    
    // Double-check device is still valid
    if (audioDeviceManager.getCurrentAudioDevice() != device)
    {
        juce::Logger::writeToLog("audioDeviceAboutToStart: device mismatch!");
        return;
    }
    
    // Get active channels (not just available)
    int numInputs = device->getActiveInputChannels().countNumberOfSetBits();
    int numOutputs = device->getActiveOutputChannels().countNumberOfSetBits();
    
    // Log channel names for debugging
    auto inputNames = device->getInputChannelNames();
    auto outputNames = device->getOutputChannelNames();
    
    juce::Logger::writeToLog("Device about to start:");
    juce::Logger::writeToLog("  Device: " + device->getName());
    juce::Logger::writeToLog("  Input channels active: " + juce::String(numInputs) + " / " + juce::String(inputNames.size()));
    juce::Logger::writeToLog("  Output channels active: " + juce::String(numOutputs) + " / " + juce::String(outputNames.size()));
    juce::Logger::writeToLog("  Active input mask: " + device->getActiveInputChannels().toString(2));
    juce::Logger::writeToLog("  Active output mask: " + device->getActiveOutputChannels().toString(2));
    
    // If no active channels, use available channels
    if (numInputs == 0)
    {
        numInputs = inputNames.size();
        juce::Logger::writeToLog("  WARNING: No active input channels, using available count: " + juce::String(numInputs));
    }
    if (numOutputs == 0)
    {
        numOutputs = outputNames.size();
        juce::Logger::writeToLog("  WARNING: No active output channels, using available count: " + juce::String(numOutputs));
    }
    
    stateModel.setChannelCounts(numInputs, numOutputs);

    // Notify subscribers (modules/UI) that device/channel state changed
    sendChangeMessage();
    
    // Always update device info and set active flag
    auto setup = audioDeviceManager.getAudioDeviceSetup();
    currentInputDeviceName = setup.inputDeviceName;
    currentOutputDeviceName = setup.outputDeviceName;
    
    updateDeviceInfo();
    deviceIsActive.store(true);
    
    juce::Logger::writeToLog("  Device activated with " + juce::String(numInputs) + " inputs, " + juce::String(numOutputs) + " outputs");
}

void DeviceManager::audioDeviceStopped()
{
    // Ignore if shutting down or switching (we're handling it)
    if (isShuttingDown.load() || isSwitching.load())
        return;
    
    deviceIsActive.store(false);
    
    // Clear all meters
    for (int i = 0; i < maxChannels; ++i)
    {
        stateModel.updateChannelLevel(i, true, 0.0f, 0.0f);
        stateModel.updateChannelLevel(i, false, 0.0f, 0.0f);
    }
}

void DeviceManager::audioDeviceError(const juce::String& errorMessage)
{
    // Ignore errors during shutdown or switching
    if (isShuttingDown.load() || isSwitching.load())
        return;
    
    juce::Logger::writeToLog("Audio Device Error: " + errorMessage);
    deviceIsActive.store(false);
}

void DeviceManager::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioDeviceManager)
    {
        // Ignore callbacks during shutdown or switching
        if (isShuttingDown.load() || isSwitching.load())
            return;
        
        // Use async call to avoid crashes during device changes
        juce::MessageManager::callAsync([this]()
        {
            if (!isShuttingDown.load() && !isSwitching.load())
            {
                updateDeviceInfo();
                sendChangeMessage();
            }
        });
    }
}

void DeviceManager::updateDeviceInfo()
{
    // Don't update if shutting down or switching
    if (isShuttingDown.load() || isSwitching.load())
        return;
    
    auto* device = audioDeviceManager.getCurrentAudioDevice();
    
    if (device != nullptr && !isShuttingDown.load())
    {
        DeviceStateModel::DeviceInfo info;
        info.deviceName = device->getName();
        info.deviceType = device->getTypeName();
        info.numInputChannels = device->getActiveInputChannels().countNumberOfSetBits();
        info.numOutputChannels = device->getActiveOutputChannels().countNumberOfSetBits();
        
        if (info.numInputChannels == 0)
            info.numInputChannels = device->getInputChannelNames().size();
        if (info.numOutputChannels == 0)
            info.numOutputChannels = device->getOutputChannelNames().size();
        
        info.sampleRate = device->getCurrentSampleRate();
        info.bufferSize = device->getCurrentBufferSizeSamples();
        info.isActive = deviceIsActive.load();
        
        stateModel.setCurrentDeviceInfo(info);
        
        // CRITICAL: Also update channel counts immediately when device info changes
        // This ensures meters appear right away when a device is selected
        // Use actual channel names count if active channels is 0
        int actualInputs = (info.numInputChannels > 0) ? info.numInputChannels : device->getInputChannelNames().size();
        int actualOutputs = (info.numOutputChannels > 0) ? info.numOutputChannels : device->getOutputChannelNames().size();
        
        juce::Logger::writeToLog("updateDeviceInfo: Setting channels - Inputs=" + juce::String(actualInputs) + 
                                 " Outputs=" + juce::String(actualOutputs));
        
        stateModel.setChannelCounts(actualInputs, actualOutputs);

        // Notify modules/UI (safe: this runs on message thread here)
        sendChangeMessage();
    }
}

void DeviceManager::releaseCurrentDevice()
{
    // This is now handled in selectInputDevice/selectOutputDevice
}