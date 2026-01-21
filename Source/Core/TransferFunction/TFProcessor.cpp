#include "TFProcessor.h"
#include <cmath>
#include <algorithm>

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
    
    // Calculate hop size (75% overlap)
    hopSize = static_cast<int>(fftSize * (1.0 - overlap));
    frameDt = static_cast<double>(hopSize) / sampleRate;
    
    // Update averaging alpha
    // Use very fast averaging for responsiveness (0.2s = 200ms)
    double Tavg = 0.2;  // Fast averaging to see changes immediately
    averagingAlpha = 1.0 - std::exp(-frameDt / Tavg);
    
    // Prepare FFT analyzers
    referenceFFT->prepare(fftSize, sampleRate);
    measurementFFT->prepare(fftSize, sampleRate);
    
    int spectrumSize = fftSize / 2 + 1;
    
    // Resize all vectors
    X.resize(spectrumSize);
    Y.resize(spectrumSize);
    Gxx.resize(spectrumSize, 0.0);
    Gyy.resize(spectrumSize, 0.0);
    Gxy.resize(spectrumSize, std::complex<double>(0.0, 0.0));
    H.resize(spectrumSize, std::complex<double>(0.0, 0.0));
    H_compensated.resize(spectrumSize, std::complex<double>(0.0, 0.0));
    H_smoothed.resize(spectrumSize, std::complex<double>(0.0, 0.0));
    gamma2.resize(spectrumSize, 0.0);
    
    magnitudeDb.resize(spectrumSize, -60.0f);
    phaseDegrees.resize(spectrumSize, 0.0f);
    coherence.resize(spectrumSize, 0.0f);
    frequencies.resize(spectrumSize);
    
    // Pre-compute frequency bins
    for (int i = 0; i < spectrumSize; ++i)
    {
        frequencies[i] = static_cast<float>(i * sampleRate / fftSize);
    }
    
    // Clear buffers
    referenceBuffer.clear();
    measurementBuffer.clear();
    
    reset();
    ready.store(true);
}

void TFProcessor::processReference(const float* input, int numSamples)
{
    if (!ready.load() || numSamples == 0)
        return;
    
    // Accumulate samples
    referenceBuffer.insert(referenceBuffer.end(), input, input + numSamples);
    
    // Try to process synchronized frames
    tryProcessSynchronizedFrames();
}

void TFProcessor::processMeasurement(const float* input, int numSamples)
{
    if (!ready.load() || numSamples == 0)
        return;
    
    // Accumulate samples
    measurementBuffer.insert(measurementBuffer.end(), input, input + numSamples);
    
    // Try to process synchronized frames
    tryProcessSynchronizedFrames();
}

void TFProcessor::tryProcessSynchronizedFrames()
{
    // Process frames only when BOTH buffers have enough data
    // This ensures we process synchronized frames
    while (static_cast<int>(referenceBuffer.size()) >= fftSize && 
           static_cast<int>(measurementBuffer.size()) >= fftSize)
    {
        // Extract synchronized frames
        std::vector<float> refFrame(referenceBuffer.begin(), referenceBuffer.begin() + fftSize);
        std::vector<float> measFrame(measurementBuffer.begin(), measurementBuffer.begin() + fftSize);
        
        // Process FFTs
        std::vector<std::complex<float>> refSpectrum;
        std::vector<std::complex<float>> measSpectrum;
        
        referenceFFT->processBlock(refFrame.data(), fftSize, refSpectrum);
        measurementFFT->processBlock(measFrame.data(), fftSize, measSpectrum);
        
        // Convert to double complex and process frame
        {
            juce::ScopedLock lock(processLock);
            int spectrumSize = static_cast<int>(X.size());
            for (int i = 0; i < spectrumSize && i < static_cast<int>(refSpectrum.size()) && i < static_cast<int>(measSpectrum.size()); ++i)
            {
                X[i] = std::complex<double>(refSpectrum[i].real(), refSpectrum[i].imag());
                Y[i] = std::complex<double>(measSpectrum[i].real(), measSpectrum[i].imag());
            }
        }
        
        // Remove processed samples (keep overlap) from both buffers
        referenceBuffer.erase(referenceBuffer.begin(), referenceBuffer.begin() + hopSize);
        measurementBuffer.erase(measurementBuffer.begin(), measurementBuffer.begin() + hopSize);
        
        // Process the synchronized frame
        processFrame();
    }
}

void TFProcessor::processFrame()
{
    juce::ScopedLock lock(processLock);
    
    // Step 1: Update averages and compute H = Gxy / (Gxx + eps)
    // Following exact document formula:
    // Gxx = avg(X * conj(X))
    // Gxy = avg(Y * conj(X))
    // H = Gxy / (Gxx + eps)
    updateAverages();
    
    // Step 2: Estimate delay periodically
    delayUpdateCounter++;
    if (delayUpdateCounter >= delayUpdatePeriod)
    {
        delayUpdateCounter = 0;
        estimateDelay();
    }
    
    // Step 3: Compensate delay in complex domain (as per document)
    double avgCoherence = 0.0;
    int spectrumSize = static_cast<int>(gamma2.size());
    for (int k = 0; k < spectrumSize; ++k)
    {
        avgCoherence += gamma2[k];
    }
    avgCoherence /= spectrumSize;
    
    if (avgCoherence > 0.6 && std::abs(estimatedDelay) < 0.01)
    {
        applyDelayCompensation();
    }
    else
    {
        H_compensated = H;  // No delay compensation
    }
    
    // Step 4: Apply smoothing in complex domain (as per document)
    applySmoothing();
    
    // Step 5: Unwrap phase (still in complex domain)
    unwrapPhase();
    
    // Step 6: Extract magnitude and phase (ONLY AFTER all processing)
    extractMagnitudeAndPhase();
    
    newDataAvailable.store(true);
}

// computeCrossSpectrum is now integrated into updateAverages

void TFProcessor::updateAverages()
{
    int spectrumSize = static_cast<int>(X.size());
    double alpha = averagingAlpha;
    
    // Follow exact formula from document:
    // Gxx = avg(X * conj(X))
    // Gxy = avg(Y * conj(X))
    // H = Gxy / (Gxx + eps)
    
    // Exponential averaging (IIR)
    for (int k = 0; k < spectrumSize; ++k)
    {
        // Instantaneous values for this frame
        // Gxx_k = X * conj(X) = |X|^2
        double Gxx_k = std::norm(X[k]);
        
        // Gxy_k = Y * conj(X)  (MEAS * conj(REF)) - CORRECT FORMULA
        std::complex<double> Gxy_k = Y[k] * std::conj(X[k]);
        
        // Update averages: G = (1-a)*G + a*G_k
        Gxx[k] = (1.0 - alpha) * Gxx[k] + alpha * Gxx_k;
        Gxy[k] = (1.0 - alpha) * Gxy[k] + alpha * Gxy_k;
        
        // Compute H1 = Gxy / (Gxx + eps) - EXACTLY AS DOCUMENT
        double denom = Gxx[k] + eps;
        H[k] = Gxy[k] / denom;
        
        // Also compute Gyy for coherence calculation
        double Gyy_k = std::norm(Y[k]);
        Gyy[k] = (1.0 - alpha) * Gyy[k] + alpha * Gyy_k;
        
        // Compute coherence: gamma2 = |Gxy|^2 / (Gxx * Gyy)
        double num = std::norm(Gxy[k]);
        double denom_coh = Gxx[k] * Gyy[k] + eps;
        gamma2[k] = num / denom_coh;
    }
}

void TFProcessor::estimateDelay()
{
    int spectrumSize = static_cast<int>(Gxy.size());
    
    // GCC-PHAT: C_phat = Gxy / |Gxy|
    std::vector<std::complex<double>> C_phat(spectrumSize);
    for (int k = 0; k < spectrumSize; ++k)
    {
        double mag = std::abs(Gxy[k]);
        if (mag > eps)
        {
            C_phat[k] = Gxy[k] / mag;
        }
        else
        {
            C_phat[k] = std::complex<double>(0.0, 0.0);
        }
    }
    
    // IFFT to get cross-correlation
    // For simplicity, use JUCE FFT (need to create IFFT)
    // For now, use phase-based delay estimation (more stable)
    
    // Alternative: Linear fit of phase
    // Select bins with good coherence
    std::vector<double> freqs;
    std::vector<double> phases;
    
    for (int k = 1; k < spectrumSize; ++k)  // Skip DC
    {
        if (gamma2[k] > cohMinMath)
        {
            double f = frequencies[k];
            if (f >= 100.0 && f <= 10000.0)  // Use mid-range frequencies
            {
                freqs.push_back(f);
                double phase = std::arg(H[k]);
                phases.push_back(phase);
            }
        }
    }
    
    if (freqs.size() < 10)
        return;
    
    // Linear fit: phase = b + m*f
    // Delay: tau = -m / (2*pi)
    double sum_f = 0.0, sum_phase = 0.0, sum_f2 = 0.0, sum_f_phase = 0.0;
    int n = static_cast<int>(freqs.size());
    
    for (int i = 0; i < n; ++i)
    {
        double f = freqs[i];
        double p = phases[i];
        sum_f += f;
        sum_phase += p;
        sum_f2 += f * f;
        sum_f_phase += f * p;
    }
    
    double denom = n * sum_f2 - sum_f * sum_f;
    if (std::abs(denom) > eps)
    {
        double m = (n * sum_f_phase - sum_f * sum_phase) / denom;
        double tau_new = -m / (2.0 * juce::MathConstants<double>::pi);
        
        // Smooth delay estimate
        smoothedDelay = 0.9 * smoothedDelay + 0.1 * tau_new;
        estimatedDelay = smoothedDelay;
    }
}

void TFProcessor::applyDelayCompensation()
{
    int spectrumSize = static_cast<int>(H.size());
    double tau = estimatedDelay;
    
    // Only apply if delay is reasonable (not too large)
    if (std::abs(tau) > 0.1)  // More than 100ms is suspicious
    {
        // Reset delay if it seems wrong
        estimatedDelay = 0.0;
        smoothedDelay = 0.0;
        tau = 0.0;
    }
    
    for (int k = 0; k < spectrumSize; ++k)
    {
        double f = frequencies[k];
        // Apply delay compensation: H_comp = H * exp(+j*2*pi*f*tau)
        // This removes the linear phase component due to delay
        double phase_comp = 2.0 * juce::MathConstants<double>::pi * f * tau;
        std::complex<double> comp = std::exp(std::complex<double>(0.0, phase_comp));
        H_compensated[k] = H[k] * comp;
    }
}

void TFProcessor::applySmoothing()
{
    int spectrumSize = static_cast<int>(H_compensated.size());
    double oct = smoothingOctaves.load();
    
    // TEMPORARILY DISABLE smoothing for debugging - use raw data
    // This will show exact frequency response without any smoothing artifacts
    H_smoothed = H_compensated;
    return;
    
    // Very minimal fractional-octave smoothing (or skip if oct is too small)
    if (oct < 1.0/96.0)
    {
        // Skip smoothing - use raw data for maximum detail
        H_smoothed = H_compensated;
        return;
    }
    
    // Fractional-octave smoothing
    for (int k = 0; k < spectrumSize; ++k)
    {
        double f0 = frequencies[k];
        if (f0 < 20.0 || f0 > 20000.0)  // Skip out of range
        {
            H_smoothed[k] = H_compensated[k];
            continue;
        }
        
        // Band limits: f1 = f0 * 2^(-oct/2), f2 = f0 * 2^(+oct/2)
        double f1 = f0 * std::pow(2.0, -oct / 2.0);
        double f2 = f0 * std::pow(2.0, +oct / 2.0);
        
        // Collect bins in band with coherence weighting
        std::complex<double> sum_H(0.0, 0.0);
        double sum_w = 0.0;
        int count = 0;
        
        for (int i = 0; i < spectrumSize; ++i)
        {
            double f = frequencies[i];
            if (f >= f1 && f <= f2)
            {
                // Weight by coherence
                double w = std::max(0.0, std::min(1.0, gamma2[i]));
                sum_H += w * H_compensated[i];
                sum_w += w;
                count++;
            }
        }
        
        // Only smooth if we have enough bins in the band
        if (sum_w > eps && count >= 3)
        {
            H_smoothed[k] = sum_H / sum_w;
        }
        else
        {
            H_smoothed[k] = H_compensated[k];
        }
    }
}

void TFProcessor::unwrapPhase()
{
    int spectrumSize = static_cast<int>(H_smoothed.size());
    
    if (spectrumSize < 2)
        return;
    
    // Unwrap phase along frequency
    // Start from bin 1 (skip DC which is often noisy)
    int startBin = 1;
    double prev_phase = std::arg(H_smoothed[startBin]);
    
    // Store unwrapped phases
    std::vector<double> unwrapped_phases(spectrumSize);
    unwrapped_phases[startBin] = prev_phase;
    
    // Forward pass: unwrap from low to high frequency
    for (int k = startBin + 1; k < spectrumSize; ++k)
    {
        double curr_phase = std::arg(H_smoothed[k]);
        double d = curr_phase - prev_phase;
        
        // Unwrap: if jump > pi, subtract 2pi; if jump < -pi, add 2pi
        while (d > juce::MathConstants<double>::pi)
        {
            curr_phase -= 2.0 * juce::MathConstants<double>::pi;
            d -= 2.0 * juce::MathConstants<double>::pi;
        }
        while (d < -juce::MathConstants<double>::pi)
        {
            curr_phase += 2.0 * juce::MathConstants<double>::pi;
            d += 2.0 * juce::MathConstants<double>::pi;
        }
        
        // Only update if coherence is good
        if (gamma2[k] >= cohMinMath)
        {
            unwrapped_phases[k] = curr_phase;
            H_smoothed[k] = std::polar(std::abs(H_smoothed[k]), curr_phase);
            prev_phase = curr_phase;
        }
        else
        {
            // Keep previous phase for low coherence bins
            unwrapped_phases[k] = prev_phase;
            H_smoothed[k] = std::polar(std::abs(H_smoothed[k]), prev_phase);
        }
    }
    
    // Backward pass: fix any remaining issues from high to low frequency
    for (int k = spectrumSize - 2; k >= startBin; --k)
    {
        double curr_phase = unwrapped_phases[k];
        double next_phase = unwrapped_phases[k + 1];
        double d = next_phase - curr_phase;
        
        // If there's still a large jump, adjust
        if (std::abs(d) > juce::MathConstants<double>::pi)
        {
            if (d > juce::MathConstants<double>::pi)
            {
                curr_phase += 2.0 * juce::MathConstants<double>::pi;
            }
            else
            {
                curr_phase -= 2.0 * juce::MathConstants<double>::pi;
            }
            unwrapped_phases[k] = curr_phase;
            H_smoothed[k] = std::polar(std::abs(H_smoothed[k]), curr_phase);
        }
    }
}

void TFProcessor::extractMagnitudeAndPhase()
{
    int spectrumSize = static_cast<int>(H_smoothed.size());
    
    for (int k = 0; k < spectrumSize; ++k)
    {
        // Magnitude in dB
        double mag = std::abs(H_smoothed[k]);
        magnitudeDb[k] = static_cast<float>(20.0 * std::log10(std::max(mag, eps)));
        
        // Phase in degrees
        double phase_rad = std::arg(H_smoothed[k]);
        phaseDegrees[k] = static_cast<float>(phase_rad * 180.0 / juce::MathConstants<double>::pi);
        
        // Coherence (0..1)
        coherence[k] = static_cast<float>(std::max(0.0, std::min(1.0, gamma2[k])));
    }
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

void TFProcessor::getCoherence(std::vector<float>& coherenceOut)
{
    juce::ScopedLock lock(processLock);
    coherenceOut = coherence;
}

void TFProcessor::getFrequencyBins(std::vector<float>& frequenciesOut)
{
    juce::ScopedLock lock(processLock);
    frequenciesOut = frequencies;
}

void TFProcessor::reset()
{
    juce::ScopedLock lock(processLock);
    
    std::fill(Gxx.begin(), Gxx.end(), 0.0);
    std::fill(Gyy.begin(), Gyy.end(), 0.0);
    std::fill(Gxy.begin(), Gxy.end(), std::complex<double>(0.0, 0.0));
    std::fill(H.begin(), H.end(), std::complex<double>(0.0, 0.0));
    std::fill(H_compensated.begin(), H_compensated.end(), std::complex<double>(0.0, 0.0));
    std::fill(H_smoothed.begin(), H_smoothed.end(), std::complex<double>(0.0, 0.0));
    std::fill(gamma2.begin(), gamma2.end(), 0.0);
    
    std::fill(magnitudeDb.begin(), magnitudeDb.end(), -60.0f);
    std::fill(phaseDegrees.begin(), phaseDegrees.end(), 0.0f);
    std::fill(coherence.begin(), coherence.end(), 0.0f);
    
    estimatedDelay = 0.0;
    smoothedDelay = 0.0;
    delayUpdateCounter = 0;
    
    referenceBuffer.clear();
    measurementBuffer.clear();
    
    newDataAvailable.store(false);
}
