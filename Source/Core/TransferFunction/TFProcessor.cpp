#include "TFProcessor.h"
#include <cmath>

TFProcessor::TFProcessor()
    : referenceFFT(std::make_unique<FFTAnalyzer>())
    , measurementFFT(std::make_unique<FFTAnalyzer>())
{
}

TFProcessor::~TFProcessor()
{
}

void TFProcessor::prepare(int newFFTSize, double newSampleRate)
{
    juce::ScopedLock lock(processLock);
    
    fftSize = newFFTSize;
    sampleRate = newSampleRate;
    
    referenceFFT->prepare(fftSize, sampleRate);
    measurementFFT->prepare(fftSize, sampleRate);
    
    int spectrumSize = fftSize / 2 + 1;
    referenceSpectrum.resize(spectrumSize);
    measurementSpectrum.resize(spectrumSize);
    transferFunction.resize(spectrumSize);
    averagedTF.resize(spectrumSize);
    
    magnitudeDb.resize(spectrumSize);
    phaseDegrees.resize(spectrumSize);
    frequencies.resize(spectrumSize);
    
    // Pre-compute frequency bins
    for (int i = 0; i < spectrumSize; ++i)
    {
        frequencies[i] = static_cast<float>(i * sampleRate / fftSize);
    }
    
    reset();
    ready.store(true);
}

void TFProcessor::processReference(const float* input, int numSamples)
{
    if (!ready.load() || numSamples == 0)
        return;
    
    referenceFFT->processBlock(input, numSamples, referenceSpectrum);
    computeTransferFunction();
}

void TFProcessor::processMeasurement(const float* input, int numSamples)
{
    if (!ready.load() || numSamples == 0)
        return;
    
    measurementFFT->processBlock(input, numSamples, measurementSpectrum);
    computeTransferFunction();
}

void TFProcessor::computeTransferFunction()
{
    juce::ScopedLock lock(processLock);
    
    if (!validateFFTResults())
        return;
    
    int spectrumSize = static_cast<int>(transferFunction.size());
    
    // Compute H(f) = Y(f) / X(f)
    for (int i = 0; i < spectrumSize; ++i)
    {
        std::complex<float> ref = referenceSpectrum[i];
        std::complex<float> meas = measurementSpectrum[i];
        
        // Avoid division by zero
        float refMagnitude = std::abs(ref);
        if (refMagnitude < 1e-10f)
        {
            transferFunction[i] = std::complex<float>(0.0f, 0.0f);
            continue;
        }
        
        // Complex division
        transferFunction[i] = meas / ref;
    }
    
    // Simple moving average
    if (frameCount == 0)
    {
        averagedTF = transferFunction;
    }
    else
    {
        float alpha = 1.0f / averagingFrames;
        for (int i = 0; i < spectrumSize; ++i)
        {
            averagedTF[i] = (1.0f - alpha) * averagedTF[i] + alpha * transferFunction[i];
        }
    }
    
    frameCount = (frameCount + 1) % averagingFrames;
    
    // Extract magnitude and phase
    for (int i = 0; i < spectrumSize; ++i)
    {
        float magnitude = std::abs(averagedTF[i]);
        magnitudeDb[i] = 20.0f * std::log10(std::max(magnitude, 1e-10f));
        
        float phase = std::arg(averagedTF[i]);
        phaseDegrees[i] = phase * 180.0f / juce::MathConstants<float>::pi;
    }
    
    newDataAvailable.store(true);
}

bool TFProcessor::validateFFTResults()
{
    // Check for invalid data
    for (size_t i = 0; i < referenceSpectrum.size(); ++i)
    {
        auto ref = referenceSpectrum[i];
        if (std::isnan(std::abs(ref)) || std::isinf(std::abs(ref)))
            return false;
    }
    
    for (size_t i = 0; i < measurementSpectrum.size(); ++i)
    {
        auto meas = measurementSpectrum[i];
        if (std::isnan(std::abs(meas)) || std::isinf(std::abs(meas)))
            return false;
    }
    
    return true;
}

void TFProcessor::getMagnitudeResponse(std::vector<float>& magnitudeDbOut)
{
    juce::ScopedLock lock(processLock);
    magnitudeDbOut = magnitudeDb;
}

void TFProcessor::getPhaseResponse(std::vector<float>& phaseDegreesOut)
{
    juce::ScopedLock lock(processLock);
    phaseDegreesOut = phaseDegrees;
}

void TFProcessor::getFrequencyBins(std::vector<float>& frequenciesOut)
{
    juce::ScopedLock lock(processLock);
    frequenciesOut = frequencies;
}

void TFProcessor::reset()
{
    juce::ScopedLock lock(processLock);
    
    std::fill(referenceSpectrum.begin(), referenceSpectrum.end(), std::complex<float>(0.0f, 0.0f));
    std::fill(measurementSpectrum.begin(), measurementSpectrum.end(), std::complex<float>(0.0f, 0.0f));
    std::fill(transferFunction.begin(), transferFunction.end(), std::complex<float>(0.0f, 0.0f));
    std::fill(averagedTF.begin(), averagedTF.end(), std::complex<float>(0.0f, 0.0f));
    std::fill(magnitudeDb.begin(), magnitudeDb.end(), -60.0f);
    std::fill(phaseDegrees.begin(), phaseDegrees.end(), 0.0f);
    
    frameCount = 0;
    newDataAvailable.store(false);
}
