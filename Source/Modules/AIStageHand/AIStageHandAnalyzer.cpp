#include "AIStageHandAnalyzer.h"

namespace AudioCoPilot
{

AIStageHandAnalyzer::AIStageHandAnalyzer(AIStageHandFifo& f, juce::WaitableEvent& dataEvent, AlertCallback onAlert)
    : juce::Thread("AIStageHandAnalyzer"),
      fifo(f),
      dataAvailable(dataEvent),
      alertCallback(std::move(onAlert))
{
}

AIStageHandAnalyzer::~AIStageHandAnalyzer()
{
    stopThread(1000);
}

void AIStageHandAnalyzer::resetState(int channels)
{
    channelState.clear();
    channelState.resize((size_t) juce::jlimit(1, 64, channels));
    for (auto& st : channelState)
    {
        st.previousMagDb.assign((size_t) (fftSize / 2), -120.0f);
        st.lastFreq = 0.0f;
        st.lastDb = -120.0f;
        st.sustainCount = 0;
        st.lastAlertMs = 0.0;
    }
}

void AIStageHandAnalyzer::run()
{
    while (! threadShouldExit())
    {
        // Espera por dados ou timeout para permitir parada graciosa
        dataAvailable.wait(50);

        int numSamples = 0;
        while (fifo.pop(block, numSamples))
        {
            const int channels = fifo.getNumChannels();
            if ((int) channelState.size() != channels)
                resetState(channels);

            for (int ch = 0; ch < channels; ++ch)
            {
                auto* ptr = block.getReadPointer(ch);
                analyzeChannel(ch, ptr, numSamples, channelState[(size_t) ch]);
            }

            if (threadShouldExit())
                break;
        }
    }
}

void AIStageHandAnalyzer::analyzeChannel(int channelIndex, const float* data, int numSamples, PeakState& state)
{
    if (data == nullptr || numSamples <= 0)
        return;

    const int samplesToUse = juce::jmin(numSamples, fftSize);

    scratch.setSize(1, fftSize, false, false, true);
    auto* scratchPtr = scratch.getWritePointer(0);
    std::fill(scratchPtr, scratchPtr + fftSize, 0.0f);
    std::copy(data, data + samplesToUse, scratchPtr);

    window.multiplyWithWindowingTable(scratchPtr, fftSize);
    fft.performFrequencyOnlyForwardTransform(scratchPtr);

    const double sr = sampleRate.load();
    const float binWidth = (float) (sr / (double) fftSize);

    float bestProminence = 0.0f;
    int bestBin = -1;
    float bestDb = -120.0f;

    // magnitude em dB + proeminência
    for (int bin = 1; bin < fftSize / 2 - 1; ++bin)
    {
        const float mag = scratchPtr[bin];
        const float magDb = 20.0f * std::log10(mag + 1e-6f);
        const float neighbor = 0.5f * (scratchPtr[bin - 1] + scratchPtr[bin + 1]);
        const float neighborDb = 20.0f * std::log10(neighbor + 1e-6f);
        const float prominence = magDb - neighborDb;

        const float growth = magDb - state.previousMagDb[(size_t) bin];
        state.previousMagDb[(size_t) bin] = magDb;

        // Critério: pico estreito, crescendo e acima de -45dB
        if (prominence > 6.0f && magDb > -45.0f && growth > 3.0f)
        {
            if (prominence > bestProminence)
            {
                bestProminence = prominence;
                bestBin = bin;
                bestDb = magDb;
            }
        }
    }

    if (bestBin >= 0)
    {
        const float freq = binWidth * (float) bestBin;

        if (std::abs(freq - state.lastFreq) < 20.0f && bestDb >= state.lastDb - 1.0f)
        {
            state.sustainCount++;
        }
        else
        {
            state.sustainCount = 1;
        }

        state.lastFreq = freq;
        state.lastDb = bestDb;

        if (state.sustainCount >= 3)
        {
            const double nowMs = juce::Time::getMillisecondCounterHiRes();
            if (nowMs - state.lastAlertMs > 800.0) // debounce
            {
                state.lastAlertMs = nowMs;
                if (alertCallback)
                    alertCallback(buildAlert(channelIndex, freq));
            }
        }
    }
}

AIStageHandAlert AIStageHandAnalyzer::buildAlert(int channel, float freqHz)
{
    AIStageHandAlert alert;
    alert.channel = channel;
    alert.frequencyHz = freqHz;
    alert.timestampMs = juce::Time::getMillisecondCounterHiRes();
    alert.suggestion = suggestionForFreq(freqHz);
    return alert;
}

juce::String AIStageHandAnalyzer::suggestionForFreq(float freqHz) const
{
    if (freqHz >= 2500.0f && freqHz <= 7000.0f)
        return "High-pitched whistle detected. Likely floor monitor feedback. Reduce gain on this channel or apply a narrow Notch Filter.";
    if (freqHz >= 100.0f && freqHz <= 250.0f)
        return "Low-end boom detected. Check acoustic guitar positioning or engage High-Pass Filter (HPF).";
    if (freqHz >= 20.0f && freqHz < 100.0f)
        return "Sub-low rumble. Accumulation of stage energy. Reduce Sub-send or check for mic stand vibration.";
    if (freqHz >= 800.0f && freqHz <= 2500.0f)
        return "Harsher mid-frequency feedback. Common in vocal mics. Check monitor EQ or mic distance.";

    // Default fallback
    if (freqHz > 7000.0f)
        return "High-frequency sibilance/whistle. Soften EQ above 7 kHz or adjust mic angle.";

    return "Potential feedback detected. Reduce gain or apply a narrow notch at the detected frequency.";
}

} // namespace AudioCoPilot
