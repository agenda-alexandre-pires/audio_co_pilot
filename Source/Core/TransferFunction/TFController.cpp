#include "TFController.h"
#include "../../Localization/LocalizedStrings.h"

TFController::TFController(DeviceManager& dm)
    : deviceManager(dm)
{
    // Listen to AudioDeviceManager changes (not DeviceManager directly)
    deviceManager.getAudioDeviceManager().addChangeListener(this);
    
    // Listen to localization changes
    LocalizedStrings::getInstance().addChangeListener(this);
    
    // Initialize analysis buffers
    referenceBuffer.resize(analysisBufferSize, 0.0f);
    measurementBuffer.resize(analysisBufferSize, 0.0f);
}

TFController::~TFController()
{
    deactivate();
    deviceManager.getAudioDeviceManager().removeChangeListener(this);
    LocalizedStrings::getInstance().removeChangeListener(this);
}

void TFController::activate()
{
    if (isActive.load())
        return;
    
    // Verify device is available before activating
    auto* device = deviceManager.getAudioDeviceManager().getCurrentAudioDevice();
    if (device == nullptr || !device->isOpen())
    {
        juce::Logger::writeToLog("TFController::activate() - No device available, cannot activate");
        return;
    }
    
    // Register as audio callback with Phase 1 DeviceManager's AudioDeviceManager
    deviceManager.getAudioDeviceManager().addAudioCallback(this);
    
    // Initialize processor with current device settings
    updateProcessorSettings();
    
    // Set default channels if not already set
    int numInputs = device->getInputChannelNames().size();
    if (referenceChannel.load() < 0 || referenceChannel.load() >= numInputs)
        referenceChannel.store(0);
    if (measurementChannel.load() < 0 || measurementChannel.load() >= numInputs)
        measurementChannel.store((numInputs > 1) ? 1 : 0);
    
    isActive.store(true);
    juce::Logger::writeToLog("TFController activated with device: " + device->getName() + 
                             ", refCh: " + juce::String(referenceChannel.load()) + 
                             ", measCh: " + juce::String(measurementChannel.load()));
}

void TFController::deactivate()
{
    if (!isActive.load())
        return;
    
    deviceManager.getAudioDeviceManager().removeAudioCallback(this);
    processor.reset();
    autoAnalyzer.reset();
    isActive.store(false);
}

void TFController::setReferenceChannel(int channelIndex)
{
    referenceChannel.store(channelIndex);
    processor.reset();
}

void TFController::setMeasurementChannel(int channelIndex)
{
    measurementChannel.store(channelIndex);
    processor.reset();
}

int TFController::getAvailableInputChannels() const
{
    auto* device = deviceManager.getAudioDeviceManager().getCurrentAudioDevice();
    if (device == nullptr)
        return 0;
    
    return device->getInputChannelNames().size();
}

juce::AudioIODevice* TFController::getCurrentAudioDevice() const
{
    return deviceManager.getAudioDeviceManager().getCurrentAudioDevice();
}

void TFController::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                    int numInputChannels,
                                                    float* const* outputChannelData,
                                                    int numOutputChannels,
                                                    int numSamples,
                                                    const juce::AudioIODeviceCallbackContext& context)
{
    // Check if device is active and module is active
    if (!isActive.load() || numSamples == 0 || numInputChannels == 0)
        return;
    
    // Verify device is actually running
    auto* device = deviceManager.getAudioDeviceManager().getCurrentAudioDevice();
    if (device == nullptr || !device->isOpen() || !deviceManager.isDeviceActive())
        return;
    
    int refCh = referenceChannel.load();
    int measCh = measurementChannel.load();
    
    // Validate channels - ensure they're within bounds
    if (refCh < 0 || refCh >= numInputChannels || measCh < 0 || measCh >= numInputChannels)
    {
        // Auto-correct to valid channels if invalid
        if (refCh < 0 || refCh >= numInputChannels)
            refCh = 0;
        if (measCh < 0 || measCh >= numInputChannels)
            measCh = (numInputChannels > 1) ? 1 : 0;
        
        referenceChannel.store(refCh);
        measurementChannel.store(measCh);
    }
    
    // Store audio buffers for analysis (circular buffer)
    // Use atomic operations for thread-safe access
    if (inputChannelData[refCh] != nullptr && inputChannelData[measCh] != nullptr)
    {
        int currentIdx = bufferWriteIndex.load(std::memory_order_relaxed);
        for (int i = 0; i < numSamples; ++i)
        {
            referenceBuffer[currentIdx] = inputChannelData[refCh][i];
            measurementBuffer[currentIdx] = inputChannelData[measCh][i];
            currentIdx = (currentIdx + 1) % analysisBufferSize;
        }
        bufferWriteIndex.store(currentIdx, std::memory_order_release);
    }
    
    // Process reference channel
    if (inputChannelData[refCh] != nullptr)
    {
        processor.processReference(inputChannelData[refCh], numSamples);
    }
    
    // Process measurement channel
    if (inputChannelData[measCh] != nullptr)
    {
        processor.processMeasurement(inputChannelData[measCh], numSamples);
    }
    
    // Perform auto-analysis periodically (not every frame for performance)
    // Increased interval significantly to reduce CPU usage and prevent freezing
    // NOTE: We schedule analysis on message thread to avoid doing heavy work on audio thread
    analysisCounter++;
    static constexpr int optimizedAnalysisInterval = 128;  // Analyze every 128 frames (~2.9s @ 44.1kHz)
    if (analysisCounter >= optimizedAnalysisInterval)
    {
        analysisCounter = 0;
        // Only analyze if we have enough data
        if (bufferWriteIndex > analysisBufferSize / 2)
        {
            // Schedule analysis on message thread to avoid blocking audio thread
            // Store a flag to check if we're still valid when callback executes
            bool wasActive = isActive.load();
            juce::MessageManager::callAsync([this, wasActive]() {
                // Double-check we're still active (thread-safe check)
                if (wasActive && isActive.load())
                {
                    performAutoAnalysis();
                }
            });
        }
    }
}

void TFController::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device == nullptr)
        return;
    
    updateProcessorSettings();
}

void TFController::audioDeviceStopped()
{
    processor.reset();
    autoAnalyzer.reset();
    std::fill(referenceBuffer.begin(), referenceBuffer.end(), 0.0f);
    std::fill(measurementBuffer.begin(), measurementBuffer.end(), 0.0f);
    bufferWriteIndex.store(0, std::memory_order_release);
}

void TFController::audioDeviceError(const juce::String& errorMessage)
{
    juce::Logger::writeToLog("TFController audio error: " + errorMessage);
    processor.reset();
}

void TFController::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &deviceManager.getAudioDeviceManager())
    {
        // Device changed in Phase 1 - update settings
        updateProcessorSettings();
        
        // Reset channel selections if invalid
        int availableChannels = getAvailableInputChannels();
        if (referenceChannel.load() >= availableChannels)
            referenceChannel.store(0);
        if (measurementChannel.load() >= availableChannels)
            measurementChannel.store(juce::jmin(1, availableChannels - 1));
        
        processor.reset();
        autoAnalyzer.reset();
        
        // Notify UI
        sendChangeMessage();
    }
    else if (source == &LocalizedStrings::getInstance())
    {
        // Language changed - reinitialize knowledge base
        // This callback is already on the message thread (from ChangeBroadcaster)
        // So we can safely reinitialize directly
        knowledgeBase.reinitializeForLanguage();
    }
}

void TFController::performAutoAnalysis()
{
    // This is now called from message thread, so it's safe to access LocalizedStrings
    
    // Get current TF data
    std::vector<float> magnitudeDb;
    std::vector<float> phaseDegrees;
    std::vector<float> frequencies;
    
    processor.getMagnitudeResponse(magnitudeDb);
    processor.getPhaseResponse(phaseDegrees);
    processor.getFrequencyBins(frequencies);
    
    if (magnitudeDb.empty() || phaseDegrees.empty() || frequencies.empty())
        return;
    
    // Prepare buffers for cross-correlation (circular to linear)
    // Copy buffer data safely (quick copy)
    std::vector<float> refLinear(analysisBufferSize);
    std::vector<float> measLinear(analysisBufferSize);
    
    // Quick copy of buffer data (atomic read ensures we get consistent snapshot)
    int currentWriteIdx = bufferWriteIndex.load(std::memory_order_acquire);
    for (int i = 0; i < analysisBufferSize; ++i)
    {
        int idx = (currentWriteIdx + i) % analysisBufferSize;
        refLinear[i] = referenceBuffer[idx];
        measLinear[i] = measurementBuffer[idx];
    }
    
    // Perform analysis
    auto result = autoAnalyzer.analyze(refLinear.data(), analysisBufferSize,
                                       measLinear.data(), analysisBufferSize,
                                       magnitudeDb, phaseDegrees, frequencies);
    
    // Generate suggestions from knowledge base
    std::vector<TFKnowledgeBase::Suggestion> allSuggestions;
    
    // Add suggestions from magnitude issues
    for (size_t i = 0; i < result.magnitudeIssues.size() && i < magnitudeDb.size(); ++i)
    {
        float freq = result.magnitudeIssues[i];
        int freqIdx = static_cast<int>(freq);
        if (freqIdx >= 0 && freqIdx < static_cast<int>(magnitudeDb.size()))
        {
            float magDev = magnitudeDb[freqIdx];
            float phaseDev = (freqIdx < static_cast<int>(phaseDegrees.size())) ? phaseDegrees[freqIdx] : 0.0f;
            
            auto suggestions = knowledgeBase.getSuggestions(freq, magDev, phaseDev);
            allSuggestions.insert(allSuggestions.end(), suggestions.begin(), suggestions.end());
        }
    }
    
    // Add suggestions from phase issues
    for (size_t i = 0; i < result.phaseIssues.size(); ++i)
    {
        float freq = result.phaseIssues[i];
        int freqIdx = static_cast<int>(freq);
        if (freqIdx >= 0 && freqIdx < static_cast<int>(phaseDegrees.size()))
        {
            float phaseDev = phaseDegrees[freqIdx];
            float magDev = (freqIdx < static_cast<int>(magnitudeDb.size())) ? magnitudeDb[freqIdx] : 0.0f;
            
            auto suggestions = knowledgeBase.getSuggestions(freq, magDev, phaseDev);
            allSuggestions.insert(allSuggestions.end(), suggestions.begin(), suggestions.end());
        }
    }
    
    // Store results (thread-safe)
    {
        juce::ScopedLock lock(analysisLock);
        lastAnalysisResult = result;
        lastSuggestions = allSuggestions;
    }
}

TFAutoAnalyzer::AnalysisResult TFController::getAnalysisResults()
{
    juce::ScopedLock lock(analysisLock);
    return lastAnalysisResult;
}

std::vector<TFKnowledgeBase::Suggestion> TFController::getSuggestions()
{
    juce::ScopedLock lock(analysisLock);
    return lastSuggestions;
}

void TFController::updateProcessorSettings()
{
    auto* device = deviceManager.getAudioDeviceManager().getCurrentAudioDevice();
    if (device == nullptr)
        return;
    
    currentSampleRate = device->getCurrentSampleRate();
    currentFFTSize = 16384;  // Smaart-style: 16384 for good resolution
    
    processor.prepare(currentFFTSize, currentSampleRate);
    autoAnalyzer.prepare(currentFFTSize, currentSampleRate);
    
    // Reset buffers
    std::fill(referenceBuffer.begin(), referenceBuffer.end(), 0.0f);
    std::fill(measurementBuffer.begin(), measurementBuffer.end(), 0.0f);
    bufferWriteIndex.store(0, std::memory_order_release);
}
