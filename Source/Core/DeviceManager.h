#pragma once

#include "../JuceHeader.h"
#include "DeviceStateModel.h"

/**
 * DeviceManager
 * 
 * Authoritative controller for audio device selection and management.
 * Single source of truth for device state.
 * 
 * Thread-safe device switching with atomic operations.
 */
class DeviceManager : public juce::AudioIODeviceCallback,
                      public juce::ChangeListener,
                      public juce::ChangeBroadcaster
{
public:
    DeviceManager(DeviceStateModel& stateModel);
    ~DeviceManager() override;
    
    // Device Management
    juce::StringArray getAvailableInputDevices() const;
    juce::StringArray getAvailableOutputDevices() const;
    juce::StringArray getAvailableFullDuplexDevices() const; // For compatibility
    
    // Separate input/output selection (like macOS)
    bool selectInputDevice(const juce::String& inputDeviceName);
    bool selectOutputDevice(const juce::String& outputDeviceName);
    
    juce::String getCurrentInputDeviceName() const;
    juce::String getCurrentOutputDeviceName() const;
    bool isDeviceActive() const;
    
    // AudioIODeviceCallback (called from audio thread)
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;
    
    // ChangeListener (for device manager changes)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // Access to AudioDeviceManager (for UI components that need device info)
    juce::AudioDeviceManager& getAudioDeviceManager() { return audioDeviceManager; }
    const juce::AudioDeviceManager& getAudioDeviceManager() const { return audioDeviceManager; }

    // Expose audio callback subscription for analysis modules (Phase 2+)
    void addAudioCallback (juce::AudioIODeviceCallback* cb) { audioDeviceManager.addAudioCallback (cb); }
    void removeAudioCallback (juce::AudioIODeviceCallback* cb) { audioDeviceManager.removeAudioCallback (cb); }
    
    // Legacy method for compatibility (maps to input device)
    bool selectDevice(const juce::String& deviceName) { return selectInputDevice(deviceName); }
    juce::String getCurrentDeviceName() const { return getCurrentInputDeviceName(); }
    void switchDevice(const juce::String& newDeviceName) { selectInputDevice(newDeviceName); }
    
private:
    void initializeDeviceManager();
    void updateDeviceInfo();
    void releaseCurrentDevice();
    
    DeviceStateModel& stateModel;
    juce::AudioDeviceManager audioDeviceManager;
    
    juce::String currentInputDeviceName;
    juce::String currentOutputDeviceName;
    std::atomic<bool> deviceIsActive{false};
    std::atomic<bool> isSwitching{false};
    std::atomic<bool> isShuttingDown{false};
    
    juce::CriticalSection deviceSwitchLock;
    
    // Metering data (lock-free, updated from audio thread)
    struct MeterData
    {
        std::vector<float> inputRMS;
        std::vector<float> inputPeak;
        std::vector<float> outputRMS;
        std::vector<float> outputPeak;
    };
    
    MeterData meterData;
    static constexpr int maxChannels = 32;
};
