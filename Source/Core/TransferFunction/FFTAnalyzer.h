#pragma once

#include "../../JuceHeader.h"

/**
 * FFTAnalyzer
 * 
 * Performs FFT analysis on audio buffers.
 * Thread-safe for use in audio callback and processing thread.
 */
class FFTAnalyzer
{
public:
    FFTAnalyzer();
    ~FFTAnalyzer();
    
    // Setup FFT with desired size (must be power of 2)
    void prepare(int fftSize, double sampleRate);
    
    // Process audio buffer and return FFT result
    // Returns complex spectrum (size = fftSize/2 + 1)
    void processBlock(const float* input, int numSamples, std::vector<std::complex<float>>& output);
    
    // Get current FFT size
    int getFFTSize() const { return fftSize; }
    
    // Get current sample rate
    double getSampleRate() const { return sampleRate; }
    
private:
    void applyWindow(float* data, int size);
    
    int fftSize{2048};
    double sampleRate{44100.0};
    
    std::unique_ptr<juce::dsp::FFT> fft;
    std::vector<float> window;
    std::vector<float> fftData;
    std::vector<float> windowedData;
    
    juce::CriticalSection processLock;
};
