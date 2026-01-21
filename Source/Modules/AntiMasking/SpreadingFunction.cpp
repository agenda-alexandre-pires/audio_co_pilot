#include "SpreadingFunction.h"
#include <cmath>

namespace AudioCoPilot
{
SpreadingFunction::SpreadingFunction()
{
    for (int masked = 0; masked < NumBands; ++masked)
    {
        for (int masking = 0; masking < NumBands; ++masking)
        {
            if (masked == masking)
                spreadingDb[(size_t) masked][(size_t) masking] = 0.0f;
            else
                spreadingDb[(size_t) masked][(size_t) masking] = calculateSpreadingDb (masking, masked, 60.0f);
        }
    }
}

float SpreadingFunction::slopeDbPerBark (float dz, float levelDb) const noexcept
{
    // Simple psychoacoustic-inspired shape (upward masking stronger).
    if (dz < 0.0f)
        return 27.0f;

    // Downward masking depends on level; keep stable/limited.
    // (This is a pragmatic model; we can refine later.)
    return 24.0f + 0.1f * juce::jlimit (0.0f, 80.0f, levelDb);
}

float SpreadingFunction::calculateSpreadingDb (int maskingBand, int maskedBand, float maskingLevelDb) const noexcept
{
    const float dz = (float) (maskedBand - maskingBand);
    const float slope = slopeDbPerBark (dz, maskingLevelDb);
    const float attenuation = slope * std::abs (dz);
    return juce::jlimit (0.0f, 100.0f, attenuation);
}

void SpreadingFunction::apply (const std::array<float, NumBands>& inputLevelsDb,
                               std::array<float, NumBands>& outMaskingDb) const noexcept
{
    // For each masked band, take max effective masker level (level - attenuation).
    for (int masked = 0; masked < NumBands; ++masked)
    {
        float maxLevel = -100.0f;
        for (int masking = 0; masking < NumBands; ++masking)
        {
            const float eff = inputLevelsDb[(size_t) masking] - spreadingDb[(size_t) masked][(size_t) masking];
            if (eff > maxLevel)
                maxLevel = eff;
        }
        outMaskingDb[(size_t) masked] = maxLevel;
    }
}
}

