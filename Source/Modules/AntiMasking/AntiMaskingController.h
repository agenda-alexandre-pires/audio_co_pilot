#pragma once

#include "../../JuceHeader.h"
#include "../../Core/DeviceManager.h"
#include "AntiMaskingProcessor.h"
#include <atomic>
#include <thread>

namespace AudioCoPilot
{
// SPSC ring buffer for float audio samples (single producer: audio callback, single consumer: worker thread)
class AudioRingBuffer
{
public:
    void prepare (int capacitySamples)
    {
        // capacitySamples must be power of 2 for fast wrap
        capacity = juce::nextPowerOfTwo (juce::jmax (2, capacitySamples));
        mask = (size_t) (capacity - 1);
        buffer.reset (new float[(size_t) capacity]);
        std::fill (buffer.get(), buffer.get() + capacity, 0.0f);
        writeIndex.store (0, std::memory_order_release);
        readIndex.store (0, std::memory_order_release);
    }

    int getCapacity() const noexcept { return capacity; }

    // Producer: write up to num samples, returns written count
    int push (const float* src, int num) noexcept
    {
        if (buffer == nullptr || src == nullptr || num <= 0) return 0;
        const size_t w = writeIndex.load (std::memory_order_relaxed);
        const size_t r = readIndex.load (std::memory_order_acquire);
        const size_t free = (size_t) capacity - (w - r);
        const size_t toWrite = (size_t) juce::jmin ((int) free, num);
        for (size_t i = 0; i < toWrite; ++i)
            buffer[(w + i) & mask] = src[i];
        writeIndex.store (w + toWrite, std::memory_order_release);
        return (int) toWrite;
    }

    // Consumer: read up to num samples, returns read count
    int pop (float* dst, int num) noexcept
    {
        if (buffer == nullptr || dst == nullptr || num <= 0) return 0;
        const size_t r = readIndex.load (std::memory_order_relaxed);
        const size_t w = writeIndex.load (std::memory_order_acquire);
        const size_t avail = (w - r);
        const size_t toRead = (size_t) juce::jmin ((int) avail, num);
        for (size_t i = 0; i < toRead; ++i)
            dst[i] = buffer[(r + i) & mask];
        readIndex.store (r + toRead, std::memory_order_release);
        return (int) toRead;
    }

    int available() const noexcept
    {
        const size_t r = readIndex.load (std::memory_order_acquire);
        const size_t w = writeIndex.load (std::memory_order_acquire);
        return (int) (w - r);
    }

private:
    int capacity { 0 };
    size_t mask { 0 };
    std::unique_ptr<float[]> buffer;
    std::atomic<size_t> writeIndex { 0 };
    std::atomic<size_t> readIndex { 0 };
};

class AntiMaskingController : public juce::AudioIODeviceCallback,
                              public juce::ChangeListener,
                              public juce::ChangeBroadcaster
{
public:
    AntiMaskingController (DeviceManager& dm);
    ~AntiMaskingController() override;

    void activate();
    void deactivate();

    int getAvailableInputChannels() const;

    // Channel indices are device input indices (0-based)
    void setTargetChannel (int channelIndex);
    void setMaskerChannel (int maskerSlot, int channelIndex, bool enabled);

    int getTargetChannel() const noexcept { return targetChannel.load(); }
    int getMaskerChannel (int slot) const noexcept { return maskerChannels[(size_t) slot].load(); }
    bool isMaskerEnabled (int slot) const noexcept { return maskerEnabled[(size_t) slot].load(); }

    const MaskingAnalysisResult& getAveragedResult() const noexcept { return processor.getAveragedResult(); }
    std::array<std::array<float, 24>, 4> getLatestSpectraDb() const noexcept { return latestSpectraDb; }

    // Audio callbacks
    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError (const juce::String& errorMessage) override;

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

private:
    void updateSettingsFromDevice();
    void workerLoop();

    DeviceManager& deviceManager;
    AntiMaskingProcessor processor;

    std::atomic<bool> active { false };
    std::atomic<bool> workerShouldRun { false };
    std::thread worker;
    juce::WaitableEvent dataReady;

    std::atomic<int> targetChannel { 0 };
    std::array<std::atomic<int>, 3> maskerChannels { 1, 2, 3 };
    std::array<std::atomic<bool>, 3> maskerEnabled { true, false, false };

    // ring buffers per selected stream (target + 3 maskers)
    static constexpr int MaxStreams = 4;
    std::array<AudioRingBuffer, MaxStreams> ring;
    std::array<std::array<float, 1024>, MaxStreams> workBlock {}; // fixed work buffer

    int currentBlockSize { 512 };
    double currentSampleRate { 48000.0 };

    // Updated from worker thread; UI reads (best-effort snapshot). Small, acceptable without locks.
    std::array<std::array<float, 24>, 4> latestSpectraDb { []{
        std::array<std::array<float, 24>, 4> x{};
        for (auto& a : x) a.fill(-100.0f);
        return x;
    }() };
};
}

