#include "RTAController.h"

namespace AudioCoPilot
{

RTAController::RTAController(DeviceManager& dm)
    : deviceManager(dm)
{
}

RTAController::~RTAController()
{
    deactivate();
}

void RTAController::activate()
{
    if (active.load()) return;
    
    active.store(true);
    deviceManager.addAudioCallback(this);
}

void RTAController::deactivate()
{
    if (!active.load()) return;
    
    active.store(false);
    deviceManager.removeAudioCallback(this);
}

void RTAController::setResolution(RTAResolution res)
{
    processor.setResolution(res);
}

RTAResolution RTAController::getResolution() const
{
    return processor.getResolution();
}

std::vector<float> RTAController::getLevels(int channelIndex)
{
    // Return copy of the levels for safety
    return processor.getLevels(channelIndex);
}

std::vector<float> RTAController::getFrequencies()
{
    return processor.getFrequencies();
}

void RTAController::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device)
        processor.prepare(device->getCurrentSampleRate());
}

void RTAController::audioDeviceStopped()
{
    processor.reset();
}

void RTAController::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                     int numInputChannels,
                                                     float* const* outputChannelData,
                                                     int numOutputChannels,
                                                     int numSamples,
                                                     const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(outputChannelData, numOutputChannels, context);
    
    if (!active.load()) return;
    
    // Feed inputs to processor
    // Pass up to 2 channels
    processor.processBlock(inputChannelData, numInputChannels, numSamples);
}

void RTAController::audioDeviceError(const juce::String& errorMessage)
{
    juce::ignoreUnused(errorMessage);
}

}
