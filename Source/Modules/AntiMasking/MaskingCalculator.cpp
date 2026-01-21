#include "MaskingCalculator.h"
#include <cmath>

namespace AudioCoPilot
{
void MaskingCalculator::prepare (double sampleRate) noexcept
{
    sr = sampleRate;
    targetDb.fill (-100.0f);
    for (auto& m : maskersDb) m.fill (-100.0f);
    for (auto& m : maskersWithSpreadingDb) m.fill (-100.0f);
    resetAverage();
}

void MaskingCalculator::setTargetSpectrumDb (const std::array<float, NumBands>& spectrumDb) noexcept
{
    targetDb = spectrumDb;
}

void MaskingCalculator::setMaskerSpectrumDb (int maskerIndex, const std::array<float, NumBands>& spectrumDb) noexcept
{
    if (maskerIndex < 0 || maskerIndex >= MaxMaskers) return;
    maskersDb[(size_t) maskerIndex] = spectrumDb;
    spreading.apply (spectrumDb, maskersWithSpreadingDb[(size_t) maskerIndex]);
}

void MaskingCalculator::setMaskerEnabled (int maskerIndex, bool enabled) noexcept
{
    if (maskerIndex < 0 || maskerIndex >= MaxMaskers) return;
    maskerEnabled[(size_t) maskerIndex] = enabled;
}

static float pow10f_fast (float db10) noexcept
{
    // db10 is in dB (10*log10(power)). Power = 10^(db/10)
    return std::pow (10.0f, db10 * 0.1f);
}

MaskingAnalysisResult MaskingCalculator::calculate() const noexcept
{
    MaskingAnalysisResult out {};
    out.targetSpectrumDb = targetDb;

    float audSum = 0.0f;
    int critical = 0;

    for (int b = 0; b < NumBands; ++b)
    {
        auto& br = out.bands[(size_t) b];
        br.targetLevelDb = targetDb[(size_t) b];

        // Combine maskers in power domain, using their spreaded levels - offset
        float combinedPower = 0.0f;
        float bestMaskerTh = -100.0f;
        int bestMasker = -1;

        for (int m = 0; m < MaxMaskers; ++m)
        {
            if (! maskerEnabled[(size_t) m])
            {
                br.maskerThresholdsDb[(size_t) m] = -100.0f;
                continue;
            }

            const float thDb = (maskersWithSpreadingDb[(size_t) m][(size_t) b]) - MaskingOffsetDb;
            br.maskerThresholdsDb[(size_t) m] = thDb;

            combinedPower += pow10f_fast (thDb);

            if (thDb > bestMaskerTh)
            {
                bestMaskerTh = thDb;
                bestMasker = m;
            }
        }

        const float thCombined = (combinedPower > 1.0e-12f) ? (10.0f * std::log10 (combinedPower)) : -100.0f;
        out.combinedThresholdDb[(size_t) b] = thCombined;

        br.maskingThresholdDb = thCombined;
        br.smrDb = br.targetLevelDb - br.maskingThresholdDb;
        br.audibility01 = audibilityFromSmr (br.smrDb);
        br.dominantMasker = bestMasker;

        audSum += br.audibility01;
        if (br.audibility01 < 0.5f) ++critical;
    }

    out.overallAudibility01 = audSum / (float) NumBands;
    out.criticalBandCount = critical;
    return out;
}

float MaskingCalculator::audibilityFromSmr (float smrDb) noexcept
{
    // Piecewise linear mapping:
    // smr <= -10 dB -> 0, smr >= +10 dB -> 1
    const float a = (smrDb + 10.0f) / 20.0f;
    return juce::jlimit (0.0f, 1.0f, a);
}

void MaskingCalculator::resetAverage() noexcept
{
    avgIndex = 0;
    avgCount = 0;
    averaged = MaskingAnalysisResult {};
}

void MaskingCalculator::pushToAverage (const MaskingAnalysisResult& r) noexcept
{
    avgBuf[(size_t) avgIndex] = r;
    avgIndex = (avgIndex + 1) % AvgBufferSize;
    avgCount = juce::jmin (avgCount + 1, AvgBufferSize);

    // accumulate
    MaskingAnalysisResult acc {};
    for (int i = 0; i < avgCount; ++i)
    {
        const auto& s = avgBuf[(size_t) i];
        acc.overallAudibility01 += s.overallAudibility01;
        acc.criticalBandCount += s.criticalBandCount;
        for (int b = 0; b < NumBands; ++b)
        {
            acc.targetSpectrumDb[(size_t) b] += s.targetSpectrumDb[(size_t) b];
            acc.combinedThresholdDb[(size_t) b] += s.combinedThresholdDb[(size_t) b];
            acc.bands[(size_t) b].audibility01 += s.bands[(size_t) b].audibility01;
            acc.bands[(size_t) b].smrDb += s.bands[(size_t) b].smrDb;
        }
    }

    const float inv = (avgCount > 0) ? (1.0f / (float) avgCount) : 1.0f;
    averaged = MaskingAnalysisResult {};
    averaged.overallAudibility01 = acc.overallAudibility01 * inv;
    averaged.criticalBandCount = (int) std::round ((float) acc.criticalBandCount * inv);

    for (int b = 0; b < NumBands; ++b)
    {
        averaged.targetSpectrumDb[(size_t) b] = acc.targetSpectrumDb[(size_t) b] * inv;
        averaged.combinedThresholdDb[(size_t) b] = acc.combinedThresholdDb[(size_t) b] * inv;
        averaged.bands[(size_t) b].audibility01 = acc.bands[(size_t) b].audibility01 * inv;
        averaged.bands[(size_t) b].smrDb = acc.bands[(size_t) b].smrDb * inv;
        averaged.bands[(size_t) b].targetLevelDb = averaged.targetSpectrumDb[(size_t) b];
        averaged.bands[(size_t) b].maskingThresholdDb = averaged.combinedThresholdDb[(size_t) b];
    }
}
}

