#pragma once

#include "../../JuceHeader.h"
#include "FFTAnalyzer.h"
#include <complex>
#include <vector>
#include <atomic>

/**
 * TFProcessor
 * 
 * Computes Transfer Function from reference and measurement signals.
 * Thread-safe processing with atomic results for UI thread.
 */
class TFProcessor
{
public:
    TFProcessor();
    ~TFProcessor();
    
    // Setup processor
    void prepare(int fftSize, double sampleRate);
    
    // Process audio buffers (called from audio thread)
    void processReference(const float* input, int numSamples);
    void processMeasurement(const float* input, int numSamples);
    
    // Get transfer function results (called from UI thread)
    void getMagnitudeResponse(std::vector<float>& magnitudeDb);
    void getPhaseResponse(std::vector<float>& phaseDegrees);
    
    // Get frequency bins (for axis)
    void getFrequencyBins(std::vector<float>& frequencies);
    
    // Reset processing
    void reset();
    
    // Check if processor is ready
    bool isReady() const { return ready.load(); }
    
private:
    void computeTransferFunction();
    bool validateFFTResults();
    
    std::unique_ptr<FFTAnalyzer> referenceFFT;
    std::unique_ptr<FFTAnalyzer> measurementFFT;
    
    std::vector<std::complex<float>> referenceSpectrum;
    std::vector<std::complex<float>> measurementSpectrum;
    std::vector<std::complex<float>> transferFunction;
    
    std::vector<float> magnitudeDb;
    std::vector<float> phaseDegrees;
    std::vector<float> frequencies;
    
    std::atomic<bool> ready{false};
    std::atomic<bool> newDataAvailable{false};
    
    int fftSize{2048};
    double sampleRate{44100.0};
    
    // Averaging
    static constexpr int averagingFrames = 8;
    std::vector<std::complex<float>> averagedTF;
    int frameCount{0};
    
    juce::CriticalSection processLock;
};
