#include "FFTAnalyzer.h"
#include <cmath>

FFTAnalyzer::FFTAnalyzer()
{
}

FFTAnalyzer::~FFTAnalyzer()
{
}

void FFTAnalyzer::prepare(int newFFTSize, double newSampleRate)
{
    juce::ScopedLock lock(processLock);
    
    // Ensure FFT size is power of 2
    int log2Size = static_cast<int>(std::log2(newFFTSize));
    fftSize = 1 << log2Size;  // Nearest power of 2
    int fftOrder = log2Size;
    
    sampleRate = newSampleRate;
    
    // Create FFT
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);
    
    // Prepare buffers
    fftData.resize(fftSize * 2);  // Real + Imaginary
    windowedData.resize(fftSize);
    
    // Create Hann window
    window.resize(fftSize);
    for (int i = 0; i < fftSize; ++i)
    {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (fftSize - 1)));
    }
}

void FFTAnalyzer::applyWindow(float* data, int size)
{
    for (int i = 0; i < size; ++i)
    {
        data[i] *= window[i];
    }
}

void FFTAnalyzer::processBlock(const float* input, int numSamples, std::vector<std::complex<float>>& output)
{
    juce::ScopedLock lock(processLock);
    
    if (numSamples == 0 || fftSize == 0)
    {
        output.clear();
        return;
    }
    
    // Zero-pad if needed
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    
    // Copy input to windowed buffer
    int samplesToCopy = juce::jmin(numSamples, fftSize);
    std::copy(input, input + samplesToCopy, windowedData.begin());
    std::fill(windowedData.begin() + samplesToCopy, windowedData.end(), 0.0f);
    
    // Apply window
    applyWindow(windowedData.data(), fftSize);
    
    // Copy to FFT input (interleaved real/imaginary)
    for (int i = 0; i < fftSize; ++i)
    {
        fftData[i * 2] = windowedData[i];      // Real
        fftData[i * 2 + 1] = 0.0f;             // Imaginary
    }
    
    // Perform FFT using juce::dsp::FFT
    if (fft != nullptr)
    {
        fft->performRealOnlyForwardTransform(fftData.data(), true);
        
        // Convert to complex output (only positive frequencies)
        // After performRealOnlyForwardTransform, fftData contains interleaved complex pairs
        // Format: [r0, i0, r1, i1, ..., rN, iN] where N = fftSize/2
        int outputSize = fftSize / 2 + 1;
        output.resize(outputSize);
        
        // Interpret as Complex<float> array (JUCE's Complex type)
        using Complex = juce::dsp::Complex<float>;
        auto* complexData = reinterpret_cast<Complex*>(fftData.data());
        
        for (int i = 0; i < outputSize; ++i)
        {
            output[i] = std::complex<float>(complexData[i].real(), complexData[i].imag());
        }
    }
    else
    {
        output.clear();
    }
}
