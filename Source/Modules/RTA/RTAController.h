#pragma once

#include "../../JuceHeader.h"
#include "../../Core/DeviceManager.h"
#include "RTAProcessor.h"
#include <atomic>

namespace AudioCoPilot
{

class RTAController : public juce::AudioIODeviceCallback,
                      public juce::ChangeBroadcaster
{
public:
    RTAController(DeviceManager& dm);
    ~RTAController() override;

    void activate();
    void deactivate();
    
    // Resolution Control
    void setResolution(RTAResolution res);
    RTAResolution getResolution() const;
    
    // Get display data for UI
    // Returns levels in dB for requested channel (0 or 1 usually)
    std::vector<float> getLevels(int channelIndex);
    std::vector<float> getFrequencies();

    // Audio callbacks
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;
    
    bool isActive() const { return active.load(); }

private:
    DeviceManager& deviceManager;
    RTAProcessor processor;
    
    std::atomic<bool> active { false };
};

}
