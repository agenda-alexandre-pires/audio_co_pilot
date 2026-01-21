#pragma once

#include "../../JuceHeader.h"
#include <array>

namespace AudioCoPilot
{
class SpreadingFunction
{
public:
    static constexpr int NumBands = 24;

    SpreadingFunction();

    // spreading[maskedBand][maskingBand] in dB attenuation
    const std::array<std::array<float, NumBands>, NumBands>& getMatrix() const noexcept { return spreadingDb; }

    void apply (const std::array<float, NumBands>& inputLevelsDb,
                std::array<float, NumBands>& outMaskingDb) const noexcept;

private:
    std::array<std::array<float, NumBands>, NumBands> spreadingDb {};

    float calculateSpreadingDb (int maskingBand, int maskedBand, float maskingLevelDb) const noexcept;
    float slopeDbPerBark (float dz, float levelDb) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpreadingFunction)
};
}

