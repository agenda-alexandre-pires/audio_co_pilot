#pragma once

#include "../../JuceHeader.h"

namespace AudioCoPilot
{

/**
 * Lock-free FIFO de blocos de áudio para análise em thread de fundo.
 * Usa AbstractFifo para evitar alocações no callback de áudio.
 */
class AIStageHandFifo
{
public:
    explicit AIStageHandFifo(juce::WaitableEvent& dataEvent);

    void prepare(int channels, int maxBlockSize);
    bool push(const float* const* input, int numChannels, int numSamples);
    bool pop(juce::AudioBuffer<float>& dest, int& numSamples);

    int getNumChannels() const noexcept { return numChannels; }
    int getMaxBlockSize() const noexcept { return maxBlockSize; }

private:
    static constexpr int queueDepth = 32;

    juce::WaitableEvent& dataAvailable;
    juce::AbstractFifo fifo { queueDepth };
    std::array<juce::AudioBuffer<float>, queueDepth> buffers;
    std::array<int, queueDepth> bufferLengths { {} };

    int numChannels { 0 };
    int maxBlockSize { 0 };
    std::atomic<bool> prepared { false };
};

} // namespace AudioCoPilot
