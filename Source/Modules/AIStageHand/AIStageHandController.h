#pragma once

#include "../../JuceHeader.h"
#include "../../Core/DeviceManager.h"
#include "AIStageHandFifo.h"
#include "AIStageHandAnalyzer.h"

namespace AudioCoPilot
{

/**
 * Controlador do módulo AIStageHand.
 * - Recebe áudio via callback (read-only)
 * - Enfileira dados para thread de análise
 * - Exponde alertas e níveis para a UI
 */
class AIStageHandController : public juce::AudioIODeviceCallback
{
public:
    explicit AIStageHandController(DeviceManager& dm);
    ~AIStageHandController() override;

    void activate();
    void deactivate();

    // UI accessors (thread-safe via atomics / cópias)
    std::vector<float> getChannelRms() const;
    std::vector<double> getChannelAlertTimesMs() const;
    juce::StringArray getAlertLog() const;

    // Audio callbacks
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;

private:
    static constexpr int maxChannels = 64;

    DeviceManager& deviceManager;
    juce::WaitableEvent dataAvailable;
    AIStageHandFifo fifo;
    AIStageHandAnalyzer analyzer;

    std::atomic<bool> active { false };
    std::atomic<int> numInputChannels { 0 };
    std::atomic<double> sampleRate { 48000.0 };

    std::array<std::atomic<float>, maxChannels> rmsValues;
    std::array<std::atomic<double>, maxChannels> lastAlertMs;

    mutable juce::SpinLock alertLock;
    juce::StringArray alertLog;

    void handleAlert(const AIStageHandAlert& alert);
    void resetMeters(int channels);
};

} // namespace AudioCoPilot
