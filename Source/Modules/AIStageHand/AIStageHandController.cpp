#include "AIStageHandController.h"

namespace AudioCoPilot
{

AIStageHandController::AIStageHandController(DeviceManager& dm)
    : deviceManager(dm),
      fifo(dataAvailable),
      analyzer(fifo, dataAvailable, [this](const AIStageHandAlert& alert) { handleAlert(alert); })
{
    resetMeters(maxChannels);
}

AIStageHandController::~AIStageHandController()
{
    deactivate();
}

void AIStageHandController::activate()
{
    if (active.load())
        return;

    active.store(true);
    analyzer.startThread(juce::Thread::Priority::normal);
    deviceManager.addAudioCallback(this);
}

void AIStageHandController::deactivate()
{
    if (!active.load())
        return;

    active.store(false);
    deviceManager.removeAudioCallback(this);
    analyzer.stopThread(1000);
}

void AIStageHandController::resetMeters(int channels)
{
    const int total = juce::jlimit(1, maxChannels, channels);
    for (int i = 0; i < total; ++i)
    {
        rmsValues[(size_t) i].store(0.0f);
        lastAlertMs[(size_t) i].store(0.0);
    }
}

std::vector<float> AIStageHandController::getChannelRms() const
{
    const int total = numInputChannels.load();
    std::vector<float> copy((size_t) total, 0.0f);
    for (int i = 0; i < total; ++i)
        copy[(size_t) i] = rmsValues[(size_t) i].load();
    return copy;
}

std::vector<double> AIStageHandController::getChannelAlertTimesMs() const
{
    const int total = numInputChannels.load();
    std::vector<double> copy((size_t) total, 0.0);
    for (int i = 0; i < total; ++i)
        copy[(size_t) i] = lastAlertMs[(size_t) i].load();
    return copy;
}

juce::StringArray AIStageHandController::getAlertLog() const
{
    const juce::SpinLock::ScopedLockType sl(alertLock);
    return alertLog;
}

void AIStageHandController::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device == nullptr)
        return;

    sampleRate.store(device->getCurrentSampleRate());
    numInputChannels.store(device->getActiveInputChannels().countNumberOfSetBits());
    const int channels = numInputChannels.load();
    const int blockSize = device->getCurrentBufferSizeSamples();

    resetMeters(channels);
    fifo.prepare(channels, blockSize);
    analyzer.setSampleRate(sampleRate.load());
    analyzer.resetState(channels);
}

void AIStageHandController::audioDeviceStopped()
{
    numInputChannels.store(0);
    resetMeters(1);
}

void AIStageHandController::audioDeviceError(const juce::String& errorMessage)
{
    juce::Logger::writeToLog("AIStageHand error: " + errorMessage);
}

void AIStageHandController::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                             int numInputCh,
                                                             float* const* outputChannelData,
                                                             int numOutputChannels,
                                                             int numSamples,
                                                             const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(outputChannelData, numOutputChannels, context);

    if (!active.load())
        return;

    const int channels = juce::jmin(numInputChannels.load(), numInputCh);
    if (channels <= 0 || inputChannelData == nullptr)
        return;

    // Atualiza RMS rápido e lock-free
    for (int ch = 0; ch < channels; ++ch)
    {
        const float* ptr = inputChannelData[ch];
        if (ptr == nullptr)
            continue;

        float sum = 0.0f;
        for (int i = 0; i < numSamples; ++i)
            sum += ptr[i] * ptr[i];

        float rms = std::sqrt(sum / (float) numSamples);
        rmsValues[(size_t) ch].store(rms);
    }

    // Enfileira bloco para análise
    fifo.push(inputChannelData, channels, numSamples);
}

void AIStageHandController::handleAlert(const AIStageHandAlert& alert)
{
    if (alert.channel >= 0 && alert.channel < maxChannels)
        lastAlertMs[(size_t) alert.channel].store(alert.timestampMs);

    const auto ts = juce::Time::getCurrentTime().toString(true, true, true, true);
    juce::String msg;
    msg << "[" << ts << "] - ALERT: Input "
        << (alert.channel + 1) << " at " << juce::String(alert.frequencyHz, 1) << "Hz. AI Suggestion: "
        << alert.suggestion;

    const juce::SpinLock::ScopedLockType sl(alertLock);
    alertLog.add(msg);
    if (alertLog.size() > 50)
        alertLog.removeRange(0, alertLog.size() - 50);
}

} // namespace AudioCoPilot
