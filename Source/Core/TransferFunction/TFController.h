#pragma once

#include "../../JuceHeader.h"
#include "../DeviceManager.h"
#include "TFProcessor.h"
#include <atomic>

/**
 * TFController
 * 
 * Controller for Transfer Function module.
 * Integrates with Phase 1 DeviceManager without owning audio devices.
 */
class TFController : public juce::AudioIODeviceCallback,
                     public juce::ChangeListener,
                     public juce::ChangeBroadcaster
{
public:
    TFController(DeviceManager& deviceManager);
    ~TFController() override;
    
    // Setup/teardown
    void activate();
    void deactivate();
    
    // Channel selection
    void setReferenceChannel(int channelIndex);
    void setMeasurementChannel(int channelIndex);
    int getReferenceChannel() const { return referenceChannel.load(); }
    int getMeasurementChannel() const { return measurementChannel.load(); }
    
    // Get available channels from Phase 1 device
    int getAvailableInputChannels() const;
    
    // Get current device (for UI)
    juce::AudioIODevice* getCurrentAudioDevice() const;
    
    // Audio callback (from DeviceManager)
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                         int numInputChannels,
                                         float* const* outputChannelData,
                                         int numOutputChannels,
                                         int numSamples,
                                         const juce::AudioIODeviceCallbackContext& context) override;
    
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;
    
    // Change listener (for device changes)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // Get processor for UI
    TFProcessor& getProcessor() { return processor; }
    
private:
    void updateProcessorSettings();
    
    DeviceManager& deviceManager;
    TFProcessor processor;
    
    std::atomic<int> referenceChannel{0};
    std::atomic<int> measurementChannel{1};
    std::atomic<bool> isActive{false};
    
    int currentFFTSize{2048};
    double currentSampleRate{44100.0};
};
