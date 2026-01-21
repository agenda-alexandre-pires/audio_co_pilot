#pragma once

#include "../../JuceHeader.h"
#include "SpreadingFunction.h"
#include <array>

namespace AudioCoPilot
{
struct BandMaskingResult
{
    float targetLevelDb { -100.0f };
    float maskingThresholdDb { -100.0f }; // combined
    float smrDb { 0.0f };
    float audibility01 { 1.0f }; // 0 masked, 1 clear
    int dominantMasker { -1 }; // 0..2
    std::array<float, 3> maskerThresholdsDb { -100.0f, -100.0f, -100.0f };
};

struct MaskingAnalysisResult
{
    std::array<BandMaskingResult, 24> bands {};
    float overallAudibility01 { 1.0f };
    int criticalBandCount { 0 };
    std::array<float, 24> targetSpectrumDb {};
    std::array<float, 24> combinedThresholdDb {};
};

class MaskingCalculator
{
public:
    static constexpr int NumBands = 24;
    static constexpr int MaxMaskers = 3;

    void prepare (double sampleRate) noexcept;

    void setTargetSpectrumDb (const std::array<float, NumBands>& spectrumDb) noexcept;
    void setMaskerSpectrumDb (int maskerIndex, const std::array<float, NumBands>& spectrumDb) noexcept;
    void setMaskerEnabled (int maskerIndex, bool enabled) noexcept;

    MaskingAnalysisResult calculate() const noexcept;

    // 5s running average @ ~10 Hz update (can be adjusted by controller)
    void resetAverage() noexcept;
    void pushToAverage (const MaskingAnalysisResult& r) noexcept;
    const MaskingAnalysisResult& getAveraged() const noexcept { return averaged; }

private:
    double sr { 48000.0 };

    SpreadingFunction spreading;

    std::array<float, NumBands> targetDb {};
    std::array<std::array<float, NumBands>, MaxMaskers> maskersDb {};
    std::array<bool, MaxMaskers> maskerEnabled { false, false, false };

    std::array<std::array<float, NumBands>, MaxMaskers> maskersWithSpreadingDb {};

    static constexpr float MaskingOffsetDb = 5.5f; // threshold below masker effective level

    // average ring
    static constexpr int AvgBufferSize = 50;
    std::array<MaskingAnalysisResult, AvgBufferSize> avgBuf {};
    int avgIndex { 0 };
    int avgCount { 0 };
    MaskingAnalysisResult averaged {};

    static float audibilityFromSmr (float smrDb) noexcept;
};
}

