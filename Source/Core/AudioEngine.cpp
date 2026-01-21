#include "AudioEngine.h"

AudioEngine::AudioEngine(DeviceManager& deviceManager, DeviceStateModel& stateModel)
    : deviceManager(deviceManager)
    , stateModel(stateModel)
{
}

bool AudioEngine::initialize()
{
    // Engine is initialized through DeviceManager
    return true;
}

void AudioEngine::shutdown()
{
    // Cleanup handled by DeviceManager
}

float AudioEngine::getChannelRMS(int channelIndex, bool isInput) const
{
    auto channels = isInput ? stateModel.getInputChannels() : stateModel.getOutputChannels();
    
    if (channelIndex >= 0 && channelIndex < static_cast<int>(channels.size()))
    {
        return channels[channelIndex].rmsLevel;
    }
    
    return 0.0f;
}

float AudioEngine::getChannelPeak(int channelIndex, bool isInput) const
{
    auto channels = isInput ? stateModel.getInputChannels() : stateModel.getOutputChannels();
    
    if (channelIndex >= 0 && channelIndex < static_cast<int>(channels.size()))
    {
        return channels[channelIndex].peakLevel;
    }
    
    return 0.0f;
}
