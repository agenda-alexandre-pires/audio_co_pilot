#include "AntiMaskingProcessor.h"

namespace AudioCoPilot
{
void AntiMaskingProcessor::prepare (double sampleRate, int blockSize) noexcept
{
    sr = sampleRate;
    for (auto& a : analyzers)
        a.prepare (sampleRate);

    masking.prepare (sampleRate);
    reset();

    // update ~10 Hz
    const double updateSeconds = 0.1;
    const double blocksPerUpdate = (sampleRate * updateSeconds) / (double) juce::jmax (1, blockSize);
    updateIntervalBlocks = juce::jmax (1, (int) std::round (blocksPerUpdate));
    prepared = true;
}

void AntiMaskingProcessor::reset() noexcept
{
    for (auto& s : cachedSpectra) s.fill (-100.0f);
    masking.resetAverage();
    updateCounter = 0;
}

void AntiMaskingProcessor::processBlock (const float* const* selectedChannelData,
                                        int selectedCount,
                                        int numSamples) noexcept
{
    if (! prepared) return;
    if (selectedChannelData == nullptr) return;
    if (selectedCount < 2 || selectedCount > MaxSelectedChannels) return;
    if (numSamples <= 0) return;

    // Per selected channel Bark analysis (RT-safe)
    for (int ch = 0; ch < selectedCount; ++ch)
    {
        const float* ptr = selectedChannelData[ch];
        if (ptr == nullptr) continue;
        analyzers[(size_t) ch].processBlock (ptr, numSamples);
        cachedSpectra[(size_t) ch] = analyzers[(size_t) ch].getSmoothedBarkLevelsDb();
    }

    ++updateCounter;
    if (updateCounter < updateIntervalBlocks)
        return;
    updateCounter = 0;

    // Setup masking calculator
    const int tIdx = juce::jlimit (0, selectedCount - 1, targetIndex);
    masking.setTargetSpectrumDb (cachedSpectra[(size_t) tIdx]);

    int maskerWrite = 0;
    for (int ch = 0; ch < selectedCount; ++ch)
    {
        if (ch == tIdx) continue;
        if (maskerWrite >= 3) break;

        masking.setMaskerSpectrumDb (maskerWrite, cachedSpectra[(size_t) ch]);
        masking.setMaskerEnabled (maskerWrite, maskerEnabled[(size_t) maskerWrite]);
        ++maskerWrite;
    }

    // Disable remaining
    for (; maskerWrite < 3; ++maskerWrite)
        masking.setMaskerEnabled (maskerWrite, false);

    const auto r = masking.calculate();
    masking.pushToAverage (r);
}
}

