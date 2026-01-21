#pragma once

#include "../JuceHeader.h"
#include <atomic>
#include <vector>

/**
 * DeviceStateModel
 * 
 * Thread-safe shared state model for audio device information.
 * Used by UI and Audio threads for safe communication.
 */
class DeviceStateModel : public juce::ChangeBroadcaster
{
public:
    struct ChannelInfo
    {
        float rmsLevel{0.0f};
        float peakLevel{0.0f};
        bool isActive{false};
    };
    
    struct DeviceInfo
    {
        juce::String deviceName;
        juce::String deviceType;
        int numInputChannels{0};
        int numOutputChannels{0};
        double sampleRate{44100.0};
        int bufferSize{512};
        bool isActive{false};
    };
    
    DeviceStateModel();
    ~DeviceStateModel() override = default;
    
    // Device Info (read-only from UI thread, written from audio thread)
    DeviceInfo getCurrentDeviceInfo() const;
    void setCurrentDeviceInfo(const DeviceInfo& info);
    
    // Channel Meters (lock-free, atomic updates)
    void updateChannelLevel(int channelIndex, bool isInput, float rms, float peak);
    std::vector<ChannelInfo> getInputChannels() const;
    std::vector<ChannelInfo> getOutputChannels() const;
    
    // Thread-safe channel count updates
    void setChannelCounts(int numInputs, int numOutputs);
    int getNumInputChannels() const;
    int getNumOutputChannels() const;
    
private:
    mutable juce::CriticalSection deviceInfoLock;
    DeviceInfo currentDeviceInfo;
    
    std::vector<ChannelInfo> inputChannels;
    std::vector<ChannelInfo> outputChannels;
    
    std::atomic<int> numInputChannels{0};
    std::atomic<int> numOutputChannels{0};
};
