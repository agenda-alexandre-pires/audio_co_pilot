#pragma once

#include "../JuceHeader.h"
#include "DeviceManager.h"
#include "DeviceStateModel.h"

/**
 * AudioEngine
 * 
 * High-level audio engine interface.
 * Provides metering and audio processing coordination.
 */
class AudioEngine
{
public:
    AudioEngine(DeviceManager& deviceManager, DeviceStateModel& stateModel);
    ~AudioEngine() = default;
    
    // Engine control
    bool initialize();
    void shutdown();
    
    // Metering access
    float getChannelRMS(int channelIndex, bool isInput) const;
    float getChannelPeak(int channelIndex, bool isInput) const;
    
    // Device access
    DeviceManager& getDeviceManager() { return deviceManager; }
    const DeviceManager& getDeviceManager() const { return deviceManager; }
    
private:
    DeviceManager& deviceManager;
    DeviceStateModel& stateModel;
};
