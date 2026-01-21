#include "DeviceStateModel.h"

DeviceStateModel::DeviceStateModel()
{
    inputChannels.resize(64);  // Support up to 64 channels
    outputChannels.resize(64);
}

DeviceStateModel::DeviceInfo DeviceStateModel::getCurrentDeviceInfo() const
{
    juce::ScopedLock lock(deviceInfoLock);
    return currentDeviceInfo;
}

void DeviceStateModel::setCurrentDeviceInfo(const DeviceInfo& info)
{
    {
        juce::ScopedLock lock(deviceInfoLock);
        currentDeviceInfo = info;
    }
    sendChangeMessage();
}

void DeviceStateModel::updateChannelLevel(int channelIndex, bool isInput, float rms, float peak)
{
    juce::ScopedLock lock(deviceInfoLock);
    auto& channels = isInput ? inputChannels : outputChannels;
    
    if (channelIndex >= 0 && channelIndex < static_cast<int>(channels.size()))
    {
        channels[channelIndex].rmsLevel = rms;
        channels[channelIndex].peakLevel = peak;
        channels[channelIndex].isActive = (rms > 0.001f || peak > 0.001f);
        
        // Debug: Log first few updates to verify data is coming
        static int updateCount = 0;
        if (updateCount < 10 && channelIndex == 0 && isInput)
        {
            juce::Logger::writeToLog("Update ch0 input - RMS: " + juce::String(rms) + " Peak: " + juce::String(peak));
            updateCount++;
        }
    }
}

std::vector<DeviceStateModel::ChannelInfo> DeviceStateModel::getInputChannels() const
{
    return inputChannels;
}

std::vector<DeviceStateModel::ChannelInfo> DeviceStateModel::getOutputChannels() const
{
    return outputChannels;
}

void DeviceStateModel::setChannelCounts(int numInputs, int numOutputs)
{
    // Cap at 64 channels
    int cappedInputs = juce::jmin(numInputs, 64);
    int cappedOutputs = juce::jmin(numOutputs, 64);
    
    // Log for debugging
    juce::Logger::writeToLog("setChannelCounts: Inputs=" + juce::String(cappedInputs) + 
                             " Outputs=" + juce::String(cappedOutputs) + 
                             " (original: " + juce::String(numInputs) + "/" + juce::String(numOutputs) + ")");
    
    int oldInputs = numInputChannels.load();
    int oldOutputs = numOutputChannels.load();
    
    numInputChannels.store(cappedInputs);
    numOutputChannels.store(cappedOutputs);
    
    // Ensure vectors are large enough (always resize to 64 to support all channels)
    if (64 > static_cast<int>(inputChannels.size()))
    {
        inputChannels.resize(64);
    }
    if (64 > static_cast<int>(outputChannels.size()))
    {
        outputChannels.resize(64);
    }
    
    // CRITICAL: If device changed (channel count changed), clear ALL channels first
    // This prevents freeze when switching devices
    if (cappedInputs != oldInputs || cappedOutputs != oldOutputs)
    {
        // Clear all channels first
        for (int i = 0; i < static_cast<int>(inputChannels.size()); ++i)
        {
            inputChannels[i].rmsLevel = 0.0f;
            inputChannels[i].peakLevel = 0.0f;
            inputChannels[i].isActive = false;
        }
        
        for (int i = 0; i < static_cast<int>(outputChannels.size()); ++i)
        {
            outputChannels[i].rmsLevel = 0.0f;
            outputChannels[i].peakLevel = 0.0f;
            outputChannels[i].isActive = false;
        }
    }
    else
    {
        // Only reset channels beyond the count
        for (int i = cappedInputs; i < static_cast<int>(inputChannels.size()); ++i)
        {
            inputChannels[i].rmsLevel = 0.0f;
            inputChannels[i].peakLevel = 0.0f;
            inputChannels[i].isActive = false;
        }
        
        for (int i = cappedOutputs; i < static_cast<int>(outputChannels.size()); ++i)
        {
            outputChannels[i].rmsLevel = 0.0f;
            outputChannels[i].peakLevel = 0.0f;
            outputChannels[i].isActive = false;
        }
    }
    
    sendChangeMessage();
}

int DeviceStateModel::getNumInputChannels() const
{
    return numInputChannels.load();
}

int DeviceStateModel::getNumOutputChannels() const
{
    return numOutputChannels.load();
}
