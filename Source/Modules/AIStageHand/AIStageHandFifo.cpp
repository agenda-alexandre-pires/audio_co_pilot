#include "AIStageHandFifo.h"

namespace AudioCoPilot
{

AIStageHandFifo::AIStageHandFifo(juce::WaitableEvent& dataEvent)
    : dataAvailable(dataEvent)
{
}

void AIStageHandFifo::prepare(int channels, int blockSize)
{
    numChannels = juce::jlimit(1, 64, channels);
    maxBlockSize = juce::jmax(64, blockSize); // mínimo para garantir espaço

    for (auto& buf : buffers)
    {
        buf.setSize(numChannels, maxBlockSize, false, false, true);
        buf.clear();
    }

    for (auto& len : bufferLengths)
        len = 0;

    fifo.reset();
    prepared.store(true);
}

bool AIStageHandFifo::push(const float* const* input, int numChannelsIn, int numSamples)
{
    if (!prepared.load() || input == nullptr)
        return false;

    const int channelsToCopy = juce::jmin(numChannels, numChannelsIn);
    const int samplesToCopy = juce::jmin(maxBlockSize, numSamples);

    int start1, size1, start2, size2;
    fifo.prepareToWrite(1, start1, size1, start2, size2);

    if (size1 > 0)
    {
        auto& target = buffers[(size_t) start1];
        for (int ch = 0; ch < channelsToCopy; ++ch)
            juce::FloatVectorOperations::copy(target.getWritePointer(ch), input[ch], samplesToCopy);

        if (channelsToCopy < numChannels)
        {
            for (int ch = channelsToCopy; ch < numChannels; ++ch)
                target.clear(ch, 0, samplesToCopy);
        }

        bufferLengths[(size_t) start1] = samplesToCopy;
        fifo.finishedWrite(1);
        dataAvailable.signal();
        return true;
    }

    return false;
}

bool AIStageHandFifo::pop(juce::AudioBuffer<float>& dest, int& numSamplesOut)
{
    if (!prepared.load())
        return false;

    int start1, size1, start2, size2;
    fifo.prepareToRead(1, start1, size1, start2, size2);

    if (size1 > 0)
    {
        auto& source = buffers[(size_t) start1];
        const int samples = bufferLengths[(size_t) start1];

        dest.setSize(numChannels, samples, false, false, true);
        for (int ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::copy(dest.getWritePointer(ch), source.getReadPointer(ch), samples);

        numSamplesOut = samples;
        fifo.finishedRead(1);
        return true;
    }

    return false;
}

} // namespace AudioCoPilot
