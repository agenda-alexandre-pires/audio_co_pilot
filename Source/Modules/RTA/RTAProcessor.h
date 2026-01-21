#pragma once

#include "../../JuceHeader.h"
#include <vector>
#include <array>
#include <atomic>

namespace AudioCoPilot
{

enum class RTAResolution
{
    ThirdOctave,
    SixthOctave,
    TwelfthOctave,
    TwentyFourthOctave,
    FortyEighthOctave
};

class RTAProcessor
{
public:
    static constexpr int MaxChannels = 2; // Stereo RTA usually, or Dual Mono
    static constexpr int FFTOrder = 12; // 4096 points
    static constexpr int FFTSize = 1 << FFTOrder;

    RTAProcessor();
    ~RTAProcessor() = default;

    void prepare(double sampleRate);
    void reset();

    // Process audio samples for a specific channel index
    void processBlock(const float* const* channelData, int numChannels, int numSamples);
    
    // Set the desired resolution for the output data
    void setResolution(RTAResolution resolution);
    RTAResolution getResolution() const { return currentResolution.load(); }

    // Get the calculated levels for a channel in dB
    // Returns a reference to the vector containing the levels
    const std::vector<float>& getLevels(int channelIndex) const;

    // Get the center frequencies for the current resolution
    const std::vector<float>& getFrequencies() const;

private:
    void updateFrequencies();
    void pushNextSampleIntoFifo(int channel, float sample);
    void performFFT(int channel);
    void mapFFTToBands(int channel);

    double sampleRate { 48000.0 };
    std::atomic<RTAResolution> currentResolution { RTAResolution::ThirdOctave };
    
    // FFT
    std::unique_ptr<juce::dsp::FFT> fft;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    // Buffers per channel
    struct ChannelData {
        std::array<float, FFTSize * 2> fftData;
        std::array<float, FFTSize> fifo;
        int fifoIndex { 0 };
        std::vector<float> outputLevels; // decibels
    };
    
    std::array<ChannelData, MaxChannels> channels;
    
    // Calculated frequencies and levels
    std::vector<float> centerFrequencies;
    
    // Smoothing
    // float releaseSpeed { 0.2f }; // Fixed release for now
};

}
