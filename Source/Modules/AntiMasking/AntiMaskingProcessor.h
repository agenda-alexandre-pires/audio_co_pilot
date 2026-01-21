#pragma once

#include "../../JuceHeader.h"
#include "BarkAnalyzer.h"
#include "MaskingCalculator.h"
#include <array>

namespace AudioCoPilot
{
class AntiMaskingProcessor
{
public:
    static constexpr int MaxSelectedChannels = 4; // 1 target + up to 3 maskers

    void prepare (double sampleRate, int blockSize) noexcept;
    void reset() noexcept;

    // Expects mono arrays per selected channel, same numSamples
    // selectedCount in [2..4]
    void processBlock (const float* const* selectedChannelData,
                       int selectedCount,
                       int numSamples) noexcept;

    void setTargetIndex (int idx) noexcept { targetIndex = juce::jlimit (0, MaxSelectedChannels - 1, idx); }
    void setMaskerEnabled (int maskerIdx, bool enabled) noexcept
    {
        if (maskerIdx < 0 || maskerIdx >= 3) return;
        maskerEnabled[(size_t) maskerIdx] = enabled;
    }

    const MaskingAnalysisResult& getAveragedResult() const noexcept { return masking.getAveraged(); }
    const std::array<float, 24>& getTargetSpectrumDb() const noexcept { return cachedSpectra[(size_t) targetIndex]; }
    const std::array<float, 24>& getSpectrumDbForSelectedIndex (int idx) const noexcept
    {
        idx = juce::jlimit (0, MaxSelectedChannels - 1, idx);
        return cachedSpectra[(size_t) idx];
    }

private:
    bool prepared { false };
    double sr { 48000.0 };
    int updateIntervalBlocks { 1 };
    int updateCounter { 0 };

    // up to 4 analyzers
    std::array<BarkAnalyzer, MaxSelectedChannels> analyzers {};
    std::array<std::array<float, 24>, MaxSelectedChannels> cachedSpectra {};

    MaskingCalculator masking {};
    int targetIndex { 0 };
    std::array<bool, 3> maskerEnabled { true, false, false };
};
}

