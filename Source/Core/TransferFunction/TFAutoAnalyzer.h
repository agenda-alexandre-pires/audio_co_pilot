#pragma once

#include "../../JuceHeader.h"
#include <vector>
#include <complex>
#include <atomic>

/**
 * TFAutoAnalyzer
 * 
 * Sistema inteligente de análise automática de Transfer Function.
 * - Auto-detecta delay via cross-correlation
 * - Analisa magnitude e fase para detectar problemas
 * - Gera sugestões de correção baseadas em análise profissional
 */
class TFAutoAnalyzer
{
public:
    struct AnalysisResult
    {
        int detectedDelaySamples{0};
        float detectedDelayMs{0.0f};
        bool delayCompensated{false};
        
        // Análise de magnitude
        std::vector<float> magnitudeIssues;  // Frequências com problemas
        std::vector<juce::String> magnitudeSuggestions;
        
        // Análise de fase
        std::vector<float> phaseIssues;      // Frequências com problemas
        std::vector<juce::String> phaseSuggestions;
        
        // Resumo geral
        float overallFlatness{0.0f};  // 0-100, quanto mais próximo de 100, mais plano
        juce::String summary;
    };
    
    TFAutoAnalyzer();
    ~TFAutoAnalyzer();
    
    // Setup
    void prepare(int fftSize, double sampleRate);
    
    // Análise principal
    // reference e measurement são buffers de áudio no domínio do tempo
    AnalysisResult analyze(const float* reference, int refSamples,
                          const float* measurement, int measSamples,
                          const std::vector<float>& magnitudeDb,
                          const std::vector<float>& phaseDegrees,
                          const std::vector<float>& frequencies);
    
    // Get delay compensation offset (em samples)
    int getDelayCompensation() const { return delayCompensation.load(); }
    
    // Reset
    void reset();
    
private:
    // Cross-correlation para detectar delay
    int detectDelayCrossCorrelation(const float* ref, int refSize,
                                    const float* meas, int measSize);
    
    // Análise de magnitude
    void analyzeMagnitude(const std::vector<float>& magnitudeDb,
                         const std::vector<float>& frequencies,
                         AnalysisResult& result);
    
    // Análise de fase
    void analyzePhase(const std::vector<float>& phaseDegrees,
                     const std::vector<float>& frequencies,
                     AnalysisResult& result);
    
    // Calcula flatness score (0-100)
    float calculateFlatness(const std::vector<float>& magnitudeDb);
    
    int fftSize{2048};
    double sampleRate{44100.0};
    
    std::atomic<int> delayCompensation{0};
    
    // Buffers para cross-correlation
    std::vector<float> correlationBuffer;
    static constexpr int maxDelaySamples = 4096;  // ~93ms @ 44.1kHz
};
