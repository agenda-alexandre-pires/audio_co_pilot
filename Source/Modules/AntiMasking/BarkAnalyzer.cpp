#include "BarkAnalyzer.h"
#include <cmath>

namespace AudioCoPilot
{
const std::array<float, 25> BarkAnalyzer::BarkEdgesHz = {
    0.0f, 100.0f, 200.0f, 300.0f, 400.0f, 510.0f, 630.0f, 770.0f,
    920.0f, 1080.0f, 1270.0f, 1480.0f, 1720.0f, 2000.0f, 2320.0f,
    2700.0f, 3150.0f, 3700.0f, 4400.0f, 5300.0f, 6400.0f, 7700.0f,
    9500.0f, 12000.0f, 15500.0f
};

const std::array<float, 24> BarkAnalyzer::BarkCentersHz = {
    50.0f, 150.0f, 250.0f, 350.0f, 455.0f, 570.0f, 700.0f, 845.0f,
    1000.0f, 1175.0f, 1375.0f, 1600.0f, 1860.0f, 2160.0f, 2510.0f,
    2925.0f, 3425.0f, 4050.0f, 4850.0f, 5850.0f, 7050.0f, 8600.0f,
    10750.0f, 13750.0f
};

BarkAnalyzer::BarkAnalyzer()
{
    inputBuffer.fill (0.0f);
    fftBuffer.fill (0.0f);
    barkLevelsDb.fill (-100.0f);
    smoothedBarkLevelsDb.fill (-100.0f);
    bandStartBin.fill (0);
    bandEndBin.fill (0);
}

void BarkAnalyzer::prepare (double sampleRate)
{
    sr = sampleRate;
    inputBufferIndex = 0;

    // ~100ms smoothing in “frames” (one FFT hop = FftSize/2)
    const auto hopSeconds = (float) ((FftSize / 2) / sr);
    const auto tauSeconds = 0.1f;
    smoothingCoeff = std::exp (-hopSeconds / tauSeconds);

    calculateBinMapping();

    barkLevelsDb.fill (-100.0f);
    smoothedBarkLevelsDb.fill (-100.0f);
}

void BarkAnalyzer::processBlock (const float* samples, int numSamples) noexcept
{
    if (samples == nullptr || numSamples <= 0)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        inputBuffer[(size_t) inputBufferIndex++] = samples[i];

        if (inputBufferIndex >= FftSize)
        {
            performFft();
            calculateBarkLevels();

            // 50% overlap
            constexpr int half = FftSize / 2;
            for (int n = 0; n < half; ++n)
                inputBuffer[(size_t) n] = inputBuffer[(size_t) (n + half)];

            inputBufferIndex = half;
        }
    }
}

void BarkAnalyzer::calculateBinMapping() noexcept
{
    const float binWidth = (float) sr / (float) FftSize;
    const int maxBin = (FftSize / 2) - 1;

    for (int band = 0; band < NumBands; ++band)
    {
        const float lowFreq = BarkEdgesHz[(size_t) band];
        const float highFreq = BarkEdgesHz[(size_t) (band + 1)];

        int start = (int) std::ceil (lowFreq / binWidth);
        int end   = (int) std::floor (highFreq / binWidth);

        if (end < start)
            end = start;

        start = juce::jlimit (0, maxBin, start);
        end   = juce::jlimit (0, maxBin, end);

        bandStartBin[(size_t) band] = start;
        bandEndBin[(size_t) band]   = end;
    }
}

void BarkAnalyzer::performFft() noexcept
{
    // Copy input + window
    for (int i = 0; i < FftSize; ++i)
        fftBuffer[(size_t) i] = inputBuffer[(size_t) i];

    window.multiplyWithWindowingTable (fftBuffer.data(), FftSize);

    // Second half must exist; clear for safety
    for (int i = 0; i < FftSize; ++i)
        fftBuffer[(size_t) (i + FftSize)] = 0.0f;

    fft.performRealOnlyForwardTransform (fftBuffer.data(), true);
}

void BarkAnalyzer::calculateBarkLevels() noexcept
{
    // fftBuffer is now interleaved complex: [r0 i0 r1 i1 ...]
    for (int band = 0; band < NumBands; ++band)
    {
        float sumPower = 0.0f;
        int count = 0;

        const int start = bandStartBin[(size_t) band];
        const int end   = bandEndBin[(size_t) band];

        for (int bin = start; bin <= end; ++bin)
        {
            const float real = fftBuffer[(size_t) (bin * 2)];
            const float imag = fftBuffer[(size_t) (bin * 2 + 1)];
            const float mag2 = (real * real) + (imag * imag);
            sumPower += mag2;
            ++count;
        }

        float avgPower = (count > 0) ? (sumPower / (float) count) : 0.0f;
        avgPower /= (float) (FftSize * FftSize);

        const float levelDb = (avgPower > 1.0e-12f) ? (10.0f * std::log10 (avgPower)) : -100.0f;
        barkLevelsDb[(size_t) band] = levelDb;

        const float prev = smoothedBarkLevelsDb[(size_t) band];
        smoothedBarkLevelsDb[(size_t) band] = (smoothingCoeff * prev) + ((1.0f - smoothingCoeff) * levelDb);
    }
}

int BarkAnalyzer::hzToBarkBand (float hz) noexcept
{
    for (int i = 0; i < NumBands; ++i)
        if (hz < BarkEdgesHz[(size_t) (i + 1)])
            return i;
    return NumBands - 1;
}

float BarkAnalyzer::hzToBark (float hz) noexcept
{
    return 26.81f * hz / (1960.0f + hz) - 0.53f;
}

float BarkAnalyzer::barkToHz (float bark) noexcept
{
    return 1960.0f * (bark + 0.53f) / (26.28f - bark);
}

float BarkAnalyzer::getBandCenterHz (int band) noexcept
{
    if (band >= 0 && band < NumBands)
        return BarkCentersHz[(size_t) band];
    return 0.0f;
}

std::pair<float, float> BarkAnalyzer::getBandEdgesHz (int band) noexcept
{
    if (band >= 0 && band < NumBands)
        return { BarkEdgesHz[(size_t) band], BarkEdgesHz[(size_t) (band + 1)] };
    return { 0.0f, 0.0f };
}
}

