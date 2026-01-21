#include "AntiMaskingController.h"

namespace AudioCoPilot
{
AntiMaskingController::AntiMaskingController (DeviceManager& dm)
    : deviceManager (dm)
{
    deviceManager.addChangeListener (this);
}

AntiMaskingController::~AntiMaskingController()
{
    deactivate();
    deviceManager.removeChangeListener (this);
}

void AntiMaskingController::activate()
{
    if (active.exchange (true))
        return;

    updateSettingsFromDevice();
    deviceManager.addAudioCallback (this);

    workerShouldRun.store (true);
    worker = std::thread ([this] { workerLoop(); });
}

void AntiMaskingController::deactivate()
{
    if (! active.exchange (false))
        return;

    deviceManager.removeAudioCallback (this);

    workerShouldRun.store (false);
    dataReady.signal();
    if (worker.joinable())
        worker.join();

    processor.reset();
}

int AntiMaskingController::getAvailableInputChannels() const
{
    auto* device = deviceManager.getAudioDeviceManager().getCurrentAudioDevice();
    if (device != nullptr)
        return device->getActiveInputChannels().countNumberOfSetBits();
    return 0;
}

void AntiMaskingController::setTargetChannel (int channelIndex)
{
    targetChannel.store (juce::jmax (0, channelIndex));
    processor.reset();
    sendChangeMessage();
}

void AntiMaskingController::setMaskerChannel (int maskerSlot, int channelIndex, bool enabled)
{
    if (maskerSlot < 0 || maskerSlot >= 3)
        return;

    maskerChannels[(size_t) maskerSlot].store (juce::jmax (0, channelIndex));
    maskerEnabled[(size_t) maskerSlot].store (enabled);
    processor.setMaskerEnabled (maskerSlot, enabled);
    processor.reset();
    sendChangeMessage();
}

void AntiMaskingController::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    juce::ignoreUnused (device);
    updateSettingsFromDevice();
    processor.reset();
    sendChangeMessage();
}

void AntiMaskingController::audioDeviceStopped()
{
    processor.reset();
}

void AntiMaskingController::audioDeviceError (const juce::String& errorMessage)
{
    juce::Logger::writeToLog ("AntiMaskingController audio error: " + errorMessage);
    processor.reset();
}

void AntiMaskingController::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source != &deviceManager)
        return;

    updateSettingsFromDevice();

    const int avail = getAvailableInputChannels();
    if (avail > 0)
    {
        if (targetChannel.load() >= avail) targetChannel.store (0);
        for (int i = 0; i < 3; ++i)
            if (maskerChannels[(size_t) i].load() >= avail)
                maskerChannels[(size_t) i].store (juce::jmin (i + 1, avail - 1));
    }

    processor.reset();
    sendChangeMessage();
}

void AntiMaskingController::updateSettingsFromDevice()
{
    auto* device = deviceManager.getAudioDeviceManager().getCurrentAudioDevice();
    if (device == nullptr)
        return;

    currentSampleRate = device->getCurrentSampleRate();
    currentBlockSize = device->getCurrentBufferSizeSamples();
    processor.prepare (currentSampleRate, currentBlockSize);

    // Preallocate rings (2 seconds per channel)
    const int cap = (int) (currentSampleRate * 2.0);
    for (auto& r : ring)
        r.prepare (cap);
}

void AntiMaskingController::audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                                             int numInputChannels,
                                                             float* const* outputChannelData,
                                                             int numOutputChannels,
                                                             int numSamples,
                                                             const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused (context);

    // This app is analysis-only; clear outputs.
    for (int ch = 0; ch < numOutputChannels; ++ch)
        if (auto* out = outputChannelData[ch])
            juce::FloatVectorOperations::clear (out, numSamples);

    if (! active.load())
        return;

    if (inputChannelData == nullptr || numSamples <= 0)
        return;

    const int t = targetChannel.load();
    const int m0 = maskerChannels[0].load();
    const int m1 = maskerChannels[1].load();
    const int m2 = maskerChannels[2].load();

    const int channels[MaxStreams] = { t, m0, m1, m2 };

    for (int s = 0; s < MaxStreams; ++s)
    {
        const int ch = channels[s];
        if (ch < 0 || ch >= numInputChannels)
            continue;

        const float* src = inputChannelData[ch];
        if (src == nullptr)
            continue;

        ring[(size_t) s].push (src, numSamples);
    }

    dataReady.signal();
}

void AntiMaskingController::workerLoop()
{
    // Fixed block size for analysis updates (kept small to avoid UI latency)
    constexpr int workN = 512;

    const float* ptrs[MaxStreams] = {
        workBlock[0].data(),
        workBlock[1].data(),
        workBlock[2].data(),
        workBlock[3].data()
    };

    while (workerShouldRun.load())
    {
        dataReady.wait (10);
        if (! workerShouldRun.load())
            break;

        // Try to pop a full work block from each enabled stream.
        // Minimum required: target + at least 1 masker => 2 streams.
        int selectedCount = 0;

        // target always present (stream 0)
        const int gotT = ring[0].pop (workBlock[0].data(), workN);
        if (gotT < workN)
            continue;

        selectedCount = 1;

        // maskers: only include enabled ones (in order)
        for (int i = 0; i < 3; ++i)
        {
            if (! maskerEnabled[(size_t) i].load())
                continue;

            const int s = i + 1;
            const int got = ring[(size_t) s].pop (workBlock[(size_t) s].data(), workN);
            if (got < workN)
                continue;

            ++selectedCount;
            if (selectedCount >= 4)
                break;
        }

        if (selectedCount < 2)
            continue;

        // The processor expects streams contiguous starting at 0 (target) and then maskers.
        // Our ptrs are fixed; processor will read only first selectedCount.
        processor.setTargetIndex (0);
        processor.processBlock (ptrs, selectedCount, workN);

        // Update latest spectra snapshot for UI
        // Stream 0 = target, streams 1..3 = maskers (as passed into processor)
        for (int i = 0; i < 4; ++i)
            latestSpectraDb[(size_t) i] = processor.getSpectrumDbForSelectedIndex (i);

        sendChangeMessage();
    }
}
}

