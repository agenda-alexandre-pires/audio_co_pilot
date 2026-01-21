#pragma once

#include "../../JuceHeader.h"
#include <array>
#include <utility>

namespace AudioCoPilot
{
class BarkAnalyzer
{
public:
    static constexpr int NumBands = 24;
    static constexpr int FftOrder = 12; // 4096
    static constexpr int FftSize  = 1 << FftOrder;

    BarkAnalyzer();
    ~BarkAnalyzer() = default;

    void prepare (double sampleRate);

    // RT-safe: no allocations
    void processBlock (const float* samples, int numSamples) noexcept;

    const std::array<float, NumBands>& getBarkLevelsDb() const noexcept { return barkLevelsDb; }
    const std::array<float, NumBands>& getSmoothedBarkLevelsDb() const noexcept { return smoothedBarkLevelsDb; }

    static int   hzToBarkBand (float hz) noexcept;
    static float hzToBark (float hz) noexcept;
    static float barkToHz (float bark) noexcept;
    static float getBandCenterHz (int band) noexcept;
    static std::pair<float, float> getBandEdgesHz (int band) noexcept;

    static const std::array<float, 25> BarkEdgesHz;
    static const std::array<float, 24> BarkCentersHz;

private:
    void calculateBinMapping() noexcept;
    void performFft() noexcept;
    void calculateBarkLevels() noexcept;

    double sr { 48000.0 };
    float smoothingCoeff { 0.0f };

    juce::dsp::FFT fft { FftOrder };
    juce::dsp::WindowingFunction<float> window { FftSize, juce::dsp::WindowingFunction<float>::hann };

    std::array<float, FftSize> inputBuffer {};
    int inputBufferIndex { 0 };

    // In-place real FFT buffer (2 * FftSize)
    std::array<float, FftSize * 2> fftBuffer {};

    std::array<int, NumBands> bandStartBin {};
    std::array<int, NumBands> bandEndBin {};

    std::array<float, NumBands> barkLevelsDb {};
    std::array<float, NumBands> smoothedBarkLevelsDb {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BarkAnalyzer)
};
}

