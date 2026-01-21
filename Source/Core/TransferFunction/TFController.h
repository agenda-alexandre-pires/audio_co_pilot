#pragma once

#include "../../JuceHeader.h"
#include "../DeviceManager.h"
#include "TFProcessor.h"
#include "TFAutoAnalyzer.h"
#include "TFKnowledgeBase.h"
#include <atomic>
#include <vector>

/**
 * TFController
 * 
 * Controller for Transfer Function module.
 * Integrates with Phase 1 DeviceManager without owning audio devices.
 * Now includes intelligent auto-analysis with delay compensation.
 */
class TFController : public juce::AudioIODeviceCallback,
                     public juce::ChangeListener,
                     public juce::ChangeBroadcaster,
                     public juce::Timer
{
public:
    TFController(DeviceManager& deviceManager);
    ~TFController() override;
    
    // Setup/teardown
    void activate();
    void deactivate();
    
    // Channel selection
    void setReferenceChannel(int channelIndex);
    void setMeasurementChannel(int channelIndex);
    int getReferenceChannel() const { return referenceChannel.load(); }
    int getMeasurementChannel() const { return measurementChannel.load(); }
    
    // Get available channels from Phase 1 device
    int getAvailableInputChannels() const;
    
    // Get current device (for UI)
    juce::AudioIODevice* getCurrentAudioDevice() const;
    
    // Audio callback (from DeviceManager)
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                         int numInputChannels,
                                         float* const* outputChannelData,
                                         int numOutputChannels,
                                         int numSamples,
                                         const juce::AudioIODeviceCallbackContext& context) override;
    
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;
    
    // Change listener (for device changes)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // Timer callback (runs on message thread for auto-analysis)
    void timerCallback() override;
    
    // Get processor for UI
    TFProcessor& getProcessor() { return processor; }
    
    // Get auto-analysis results (called from UI thread)
    TFAutoAnalyzer::AnalysisResult getAnalysisResults();
    
    // Get knowledge base suggestions
    std::vector<TFKnowledgeBase::Suggestion> getSuggestions();
    
private:
    void updateProcessorSettings();
    void performAutoAnalysis();
    
    DeviceManager& deviceManager;
    TFProcessor processor;
    TFAutoAnalyzer autoAnalyzer;
    TFKnowledgeBase knowledgeBase;
    
    std::atomic<int> referenceChannel{0};
    std::atomic<int> measurementChannel{1};
    std::atomic<bool> isActive{false};
    
    int currentFFTSize{2048};
    double currentSampleRate{44100.0};
    
    // Buffers para análise (armazenam últimos N samples)
    static constexpr int analysisBufferSize = 4096;
    std::vector<float> referenceBuffer;
    std::vector<float> measurementBuffer;
    std::atomic<int> bufferWriteIndex{0};  // Made atomic for thread-safe access
    
    // Resultados de análise (thread-safe)
    juce::CriticalSection analysisLock;
    TFAutoAnalyzer::AnalysisResult lastAnalysisResult;
    std::vector<TFKnowledgeBase::Suggestion> lastSuggestions;
    
    // Contador para análise periódica (não a cada frame)
    int analysisCounter{0};
};
