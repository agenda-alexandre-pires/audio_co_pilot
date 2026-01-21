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
    
    // Update averaging alpha using time constant
    // alpha = exp(-frameDt / Tavg) for exponential averaging
    // This ensures fast convergence (0.3-1.0s time constant recommended)
    double Tavg = averagingTime.load();
    averagingAlpha = std::exp(-frameDt / Tavg);
    
    // Prepare FFT analyzers
    // NOTE: FFTAnalyzer rounds fftSize to nearest power of 2
    referenceFFT->prepare(fftSize, sampleRate);
    measurementFFT->prepare(fftSize, sampleRate);
    
    // Get the ACTUAL fftSize used by FFTAnalyzer (may be rounded to power of 2)
    int actualFFTSize = referenceFFT->getFFTSize();
    if (actualFFTSize != fftSize)
    {
        juce::Logger::writeToLog("TFProcessor::prepare - fftSize rounded: " + 
                                 juce::String(fftSize) + " -> " + juce::String(actualFFTSize));
        fftSize = actualFFTSize;  // Use the actual FFT size
    }
    
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
    // CORRECT FORMULA: freqHz = k * sampleRate / fftSize
    // where k is the bin index (0..spectrumSize-1) and fftSize is the actual FFT size (e.g., 16384)
    for (int i = 0; i < spectrumSize; ++i)
    {
        frequencies[i] = static_cast<float>(i * sampleRate / static_cast<double>(fftSize));
    }
    
    // DIAGNOSTIC LOG: Processor settings (Parte 3)
    juce::Logger::writeToLog("TFProcessor::prepare - fftSize=" + juce::String(fftSize) + 
                             ", sampleRate=" + juce::String(sampleRate, 1) + 
                             ", spectrumSize=" + juce::String(spectrumSize) +
                             ", freq[0]=" + juce::String(frequencies[0], 2) +
                             ", freq[100]=" + juce::String(frequencies[juce::jmin(100, spectrumSize-1)], 2) +
                             ", freq[1000]=" + juce::String(frequencies[juce::jmin(1000, spectrumSize-1)], 2) +
                             ", freq[last]=" + juce::String(frequencies[spectrumSize-1], 2));
    
    // Clear buffers
    referenceBuffer.clear();
    measurementBuffer.clear();
    
    reset();
    ready.store(true);
}

// OLD methods removed - caused sync issues
// Now using processBlock() to ensure REF and MEAS are always synchronized

void TFProcessor::processBlock(const float* ref, const float* meas, int numSamples)
{
    if (!ready.load() || numSamples <= 0)
        return;
    
    // Accumulate samples from both channels together (guaranteed synchronized)
    referenceBuffer.insert(referenceBuffer.end(), ref, ref + numSamples);
    measurementBuffer.insert(measurementBuffer.end(), meas, meas + numSamples);
    
    // Process synchronized frames
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
    
    // Apply delay compensation if coherence is good (removed 10ms limit - too restrictive)
    if (avgCoherence > 0.6)
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
        
        // Update averages in complex domain (Smaart-like):
        // avgGxx = alpha*avgGxx + (1-alpha)*Gxx_k
        // avgGxy = alpha*avgGxy + (1-alpha)*Gxy_k
        // This ensures fast convergence and stability
        Gxx[k] = alpha * Gxx[k] + (1.0 - alpha) * Gxx_k;
        Gxy[k] = alpha * Gxy[k] + (1.0 - alpha) * Gxy_k;
        
        // Also compute Gyy for coherence calculation
        double Gyy_k = std::norm(Y[k]);
        Gyy[k] = alpha * Gyy[k] + (1.0 - alpha) * Gyy_k;
        
        // Compute H1 = avgGxy / (avgGxx + eps) - ONLY AFTER averaging
        // This ensures phase is stable and converges quickly
        double denom = Gxx[k] + eps;
        H[k] = Gxy[k] / denom;
        
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
    
    // Linear fit of phase (with unwrap first)
    // Select bins with good coherence
    std::vector<double> freqs;
    std::vector<double> phases;
    
    for (int k = 1; k < spectrumSize; ++k)  // Skip DC
    {
        if (gamma2[k] > cohMinMath)
        {
            double f = frequencies[k];
            if (f >= 200.0 && f <= 8000.0)  // Use mid-range frequencies (more stable)
            {
                freqs.push_back(f);
                double phase = std::arg(H[k]);
                phases.push_back(phase);
            }
        }
    }
    
    if (freqs.size() < 20)  // Need enough points for reliable fit
        return;
    
    // CRITICAL: Unwrap phases BEFORE linear fit
    for (size_t i = 1; i < phases.size(); ++i)
    {
        double d = phases[i] - phases[i - 1];
        while (d > juce::MathConstants<double>::pi)
        {
            phases[i] -= 2.0 * juce::MathConstants<double>::pi;
            d -= 2.0 * juce::MathConstants<double>::pi;
        }
        while (d < -juce::MathConstants<double>::pi)
        {
            phases[i] += 2.0 * juce::MathConstants<double>::pi;
            d += 2.0 * juce::MathConstants<double>::pi;
        }
    }
    
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
        
        // Clamp to reasonable range
        tau_new = std::max(-0.1, std::min(0.1, tau_new));
        
        // Smooth delay estimate (faster update for responsiveness)
        smoothedDelay = 0.8 * smoothedDelay + 0.2 * tau_new;
        estimatedDelay = smoothedDelay;
    }
}

void TFProcessor::applyDelayCompensation()
{
    int spectrumSize = static_cast<int>(H.size());
    double tau = estimatedDelay;
    
    // Protection: reset delay if it seems wrong (more than 100ms is suspicious)
    if (std::abs(tau) > 0.1)
    {
        estimatedDelay = 0.0;
        smoothedDelay = 0.0;
        tau = 0.0;
        juce::Logger::writeToLog("TFProcessor::applyDelayCompensation - Delay reset (too large: " + juce::String(tau, 4) + "s)");
    }
    
    // Apply delay compensation in complex domain
    // H_comp = H * exp(+j*2*pi*f*tau)
    // This removes the linear phase component due to delay between REF and MEAS channels
    for (int k = 0; k < spectrumSize; ++k)
    {
        double f = frequencies[k];
        double phase_comp = 2.0 * juce::MathConstants<double>::pi * f * tau;
        std::complex<double> comp = std::exp(std::complex<double>(0.0, phase_comp));
        H_compensated[k] = H[k] * comp;
    }
    
    // Log delay compensation (every 100 delay updates to avoid spam)
    static int delayLogCounter = 0;
    delayLogCounter++;
    if (delayLogCounter % 100 == 0 && std::abs(tau) > 1e-6)
    {
        juce::Logger::writeToLog("TFProcessor::applyDelayCompensation - tau=" + juce::String(tau * 1000.0, 2) + "ms");
    }
}

void TFProcessor::applySmoothing()
{
    int spectrumSize = static_cast<int>(H_compensated.size());
    double oct = smoothingOctaves.load();
    
    // Apply fractional-octave smoothing (1/12 octave default)
    if (oct < 1.0/96.0)
    {
        // Skip smoothing only if oct is extremely small
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
    
    // Extract magnitude and phase from averaged complex H
    // CRITICAL: No averaging in phaseDegrees/phaseRad - averaging is done in complex domain
    // This ensures fast convergence and stability (Smaart-like)
    for (int k = 0; k < spectrumSize; ++k)
    {
        // Magnitude in dB: magDb = 20*log10(|Havg|)
        double mag = std::abs(H_smoothed[k]);
        magnitudeDb[k] = static_cast<float>(20.0 * std::log10(std::max(mag, eps)));
        
        // Phase in radians: phaseRad = atan2(imag(Havg), real(Havg))
        // Then convert to degrees
        double phase_rad = std::atan2(H_smoothed[k].imag(), H_smoothed[k].real());
        phaseDegrees[k] = static_cast<float>(phase_rad * 180.0 / juce::MathConstants<double>::pi);
        
        // Coherence (0..1)
        coherence[k] = static_cast<float>(std::max(0.0, std::min(1.0, gamma2[k])));
    }
    
    // DIAGNOSTIC: Find peak within 20-20k Hz range and calculate expected bin for 1kHz
    int kPeak = 0;
    float vPeak = -9999.0f;
    for (int k = 1; k < spectrumSize; ++k)  // Start from k=1 to skip DC
    {
        // Only consider frequencies in 20-20000 Hz range
        float freq = (k < static_cast<int>(frequencies.size())) ? frequencies[k] : 0.0f;
        if (freq >= 20.0f && freq <= 20000.0f)
        {
            if (magnitudeDb[k] > vPeak)
            {
                vPeak = magnitudeDb[k];
                kPeak = k;
            }
        }
    }
    
    // Calculate expected bin for 1kHz: k1k = 1000 * fftSize / sampleRate
    int k1kExpected = static_cast<int>(1000.0 * fftSize / sampleRate);
    float freq1kExpected = (k1kExpected < spectrumSize) ? frequencies[k1kExpected] : 0.0f;
    
    // Log peak info (every 100 frames to avoid spam)
    static int frameCounter = 0;
    frameCounter++;
    if (kPeak > 0 && frameCounter % 100 == 0)
    {
        float peakFreq = (kPeak < static_cast<int>(frequencies.size())) ? frequencies[kPeak] : 0.0f;
        juce::Logger::writeToLog("TF Peak: k=" + juce::String(kPeak) + 
                                 " freq=" + juce::String(peakFreq, 2) + " Hz" +
                                 " mag=" + juce::String(vPeak, 2) + " dB" +
                                 " Fs=" + juce::String(sampleRate, 1) + 
                                 " fftSize=" + juce::String(fftSize) +
                                 " | k@1k expected=" + juce::String(k1kExpected) +
                                 " freq@k1k=" + juce::String(freq1kExpected, 2) + " Hz");
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

