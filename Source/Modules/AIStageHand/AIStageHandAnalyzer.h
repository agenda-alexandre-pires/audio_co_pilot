#pragma once

#include "../../JuceHeader.h"
#include "AIStageHandFifo.h"

namespace AudioCoPilot
{

struct AIStageHandAlert
{
    int channel { 0 };
    float frequencyHz { 0.0f };
    juce::String suggestion;
    double timestampMs { 0.0 };
};

/**
 * Thread de análise em background. Puxa blocos do FIFO e detecta feedback.
 */
class AIStageHandAnalyzer : public juce::Thread
{
public:
    using AlertCallback = std::function<void(const AIStageHandAlert&)>;

    AIStageHandAnalyzer(AIStageHandFifo& fifo, juce::WaitableEvent& dataEvent, AlertCallback onAlert);
    ~AIStageHandAnalyzer() override;

    void setSampleRate(double sr) { sampleRate.store(sr); }
    void resetState(int channels);

    void run() override;

private:
    struct PeakState
    {
        std::vector<float> previousMagDb;
        float lastFreq { 0.0f };
        float lastDb { -120.0f };
        int sustainCount { 0 };
        double lastAlertMs { 0.0 };
    };

    AIStageHandFifo& fifo;
    juce::WaitableEvent& dataAvailable;
    AlertCallback alertCallback;

    std::atomic<double> sampleRate { 48000.0 };

    static constexpr int fftOrder = 11; // 2048
    static constexpr int fftSize = 1 << fftOrder;
    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann };

    juce::AudioBuffer<float> scratch;
    juce::AudioBuffer<float> block;

    std::vector<PeakState> channelState;

    AIStageHandAlert buildAlert(int channel, float freqHz);
    juce::String suggestionForFreq(float freqHz) const;
    void analyzeChannel(int channelIndex, const float* data, int numSamples, PeakState& state);
};

} // namespace AudioCoPilot
