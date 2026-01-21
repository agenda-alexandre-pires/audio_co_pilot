#include "TFController.h"

TFController::TFController(DeviceManager& dm)
    : deviceManager(dm)
{
    // Listen to AudioDeviceManager changes (not DeviceManager directly)
    deviceManager.getAudioDeviceManager().addChangeListener(this);
}

TFController::~TFController()
{
    deactivate();
    deviceManager.getAudioDeviceManager().removeChangeListener(this);
}

void TFController::activate()
{
    if (isActive.load())
        return;
    
    // Register as audio callback with Phase 1 DeviceManager's AudioDeviceManager
    deviceManager.getAudioDeviceManager().addAudioCallback(this);
    
    updateProcessorSettings();
    isActive.store(true);
}

void TFController::deactivate()
{
    if (!isActive.load())
        return;
    
    deviceManager.getAudioDeviceManager().removeAudioCallback(this);
    processor.reset();
    isActive.store(false);
}

void TFController::setReferenceChannel(int channelIndex)
{
    referenceChannel.store(channelIndex);
    processor.reset();
}

void TFController::setMeasurementChannel(int channelIndex)
{
    measurementChannel.store(channelIndex);
    processor.reset();
}

int TFController::getAvailableInputChannels() const
{
    auto* device = deviceManager.getAudioDeviceManager().getCurrentAudioDevice();
    if (device == nullptr)
        return 0;
    
    return device->getInputChannelNames().size();
}

juce::AudioIODevice* TFController::getCurrentAudioDevice() const
{
    return deviceManager.getAudioDeviceManager().getCurrentAudioDevice();
}

void TFController::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                    int numInputChannels,
                                                    float* const* outputChannelData,
                                                    int numOutputChannels,
                                                    int numSamples,
                                                    const juce::AudioIODeviceCallbackContext& context)
{
    if (!isActive.load() || numSamples == 0)
        return;
    
    int refCh = referenceChannel.load();
    int measCh = measurementChannel.load();
    
    // Validate channels
    if (refCh < 0 || refCh >= numInputChannels || measCh < 0 || measCh >= numInputChannels)
        return;
    
    // Process reference channel
    if (inputChannelData[refCh] != nullptr)
    {
        processor.processReference(inputChannelData[refCh], numSamples);
    }
    
    // Process measurement channel
    if (inputChannelData[measCh] != nullptr)
    {
        processor.processMeasurement(inputChannelData[measCh], numSamples);
    }
}

void TFController::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device == nullptr)
        return;
    
    updateProcessorSettings();
}

void TFController::audioDeviceStopped()
{
    processor.reset();
}

void TFController::audioDeviceError(const juce::String& errorMessage)
{
    juce::Logger::writeToLog("TFController audio error: " + errorMessage);
    processor.reset();
}

void TFController::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &deviceManager.getAudioDeviceManager())
    {
        // Device changed in Phase 1 - update settings
        updateProcessorSettings();
        
        // Reset channel selections if invalid
        int availableChannels = getAvailableInputChannels();
        if (referenceChannel.load() >= availableChannels)
            referenceChannel.store(0);
        if (measurementChannel.load() >= availableChannels)
            measurementChannel.store(juce::jmin(1, availableChannels - 1));
        
        processor.reset();
        
        // Notify UI
        sendChangeMessage();
    }
}

void TFController::updateProcessorSettings()
{
    auto* device = deviceManager.getAudioDeviceManager().getCurrentAudioDevice();
    if (device == nullptr)
        return;
    
    currentSampleRate = device->getCurrentSampleRate();
    currentFFTSize = 2048;  // Fixed for Phase 2
    
    processor.prepare(currentFFTSize, currentSampleRate);
}
