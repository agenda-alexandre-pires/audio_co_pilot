#pragma once

#include "../../JuceHeader.h"
#include "FFTAnalyzer.h"
#include <complex>
#include <vector>
#include <atomic>

/**
 * TFProcessor - Smaart-style Transfer Function
 * 
 * Implements H1 estimator with:
 * - Cross-spectrum averaging (Gxx, Gyy, Gxy)
 * - Exponential averaging (IIR)
 * - Delay compensation
 * - Phase unwrap
 * - Fractional-octave smoothing
 * - Coherence (gamma2)
 */
class TFProcessor
{
public:
    TFProcessor();
    ~TFProcessor();
    
    // Setup processor
    void prepare(int fftSize, double sampleRate);
    
    // Process audio buffers (called from audio thread)
    // OLD: Separate calls (removed - causes sync issues)
    // void processReference(const float* input, int numSamples);
    // void processMeasurement(const float* input, int numSamples);
    
    // NEW: Synchronized processing - both channels together
    void processBlock(const float* ref, const float* meas, int numSamples);
    
    // Get transfer function results (called from UI thread)
    void getMagnitudeResponse(std::vector<float>& magnitudeDb);
    void getPhaseResponse(std::vector<float>& phaseDegrees);
    void getCoherence(std::vector<float>& coherence);
    
    // Get frequency bins (for axis)
    void getFrequencyBins(std::vector<float>& frequencies);
    
    // Get estimated delay between channels (in seconds)
    double getEstimatedDelay() const { return estimatedDelay; }
    
    // Reset processing
    void reset();
    
    // Check if processor is ready
    bool isReady() const { return ready.load(); }
    
    // Settings
    void setAveragingTime(double seconds) { averagingTime.store(seconds); }
    double getAveragingTime() const { return averagingTime.load(); }
    
    void setSmoothingOctaves(double octaves) { smoothingOctaves.store(octaves); }
    double getSmoothingOctaves() const { return smoothingOctaves.load(); }
    
private:
    void tryProcessSynchronizedFrames();  // Process frames only when both buffers are ready
    void processFrame();
    void computeCrossSpectrum();
    void updateAverages();
    void estimateDelay();
    void applyDelayCompensation();
    void applySmoothing();
    void unwrapPhase();
    void extractMagnitudeAndPhase();
    
    // FFT analyzers
    std::unique_ptr<FFTAnalyzer> referenceFFT;
    std::unique_ptr<FFTAnalyzer> measurementFFT;
    
    // Buffers for overlap processing
    std::vector<float> referenceBuffer;
    std::vector<float> measurementBuffer;
    
    // FFT results (complex spectra)
    std::vector<std::complex<double>> X;  // Reference spectrum
    std::vector<std::complex<double>> Y;  // Measurement spectrum
    
    // Cross-spectra (averaged)
    std::vector<double> Gxx;  // Auto-spectrum of reference
    std::vector<double> Gyy;  // Auto-spectrum of measurement
    std::vector<std::complex<double>> Gxy;  // Cross-spectrum
    
    // Transfer function
    std::vector<std::complex<double>> H;  // H1 = Gxy / Gxx
    std::vector<std::complex<double>> H_compensated;  // With delay compensation
    std::vector<std::complex<double>> H_smoothed;  // After smoothing
    
    // Coherence
    std::vector<double> gamma2;  // Magnitude-squared coherence
    
    // Results for UI
    std::vector<float> magnitudeDb;
    std::vector<float> phaseDegrees;
    std::vector<float> coherence;
    std::vector<float> frequencies;
    
    // Averaging state (exponential averaging with time constant)
    double averagingAlpha{0.0};  // Computed from averagingTime: alpha = exp(-frameDt / Tavg)
    std::atomic<double> averagingTime{0.7};  // seconds - time constant (0.3-1.0s for fast convergence)
    double frameDt{0.0};  // Hop time in seconds
    
    // Delay compensation
    double estimatedDelay{0.0};  // in seconds
    double smoothedDelay{0.0};  // smoothed version
    int delayUpdateCounter{0};
    static constexpr int delayUpdatePeriod = 100;  // frames
    
    // Smoothing - 1/12 octave default (Smaart-like)
    std::atomic<double> smoothingOctaves{1.0/12.0};  // 1/12 octave default
    
    // Processing parameters
    int fftSize{16384};
    double sampleRate{48000.0};
    int hopSize{0};  // NFFT * (1 - overlap)
    static constexpr double overlap = 0.75;  // 75% overlap
    
    // State
    std::atomic<bool> ready{false};
    std::atomic<bool> newDataAvailable{false};
    
    // Thread safety
    juce::CriticalSection processLock;
    
    // Constants
    static constexpr double eps = 1e-12;
    static constexpr double cohMinDraw = 0.6;
    static constexpr double cohMinMath = 0.4;
};
