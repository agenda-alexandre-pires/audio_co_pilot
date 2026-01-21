#include "TFAutoAnalyzer.h"
#include "../../Localization/LocalizedStrings.h"
#include <cmath>
#include <algorithm>

TFAutoAnalyzer::TFAutoAnalyzer()
{
}

TFAutoAnalyzer::~TFAutoAnalyzer()
{
}

void TFAutoAnalyzer::prepare(int newFFTSize, double newSampleRate)
{
    fftSize = newFFTSize;
    sampleRate = newSampleRate;
    correlationBuffer.resize(maxDelaySamples * 2);
    reset();
}

void TFAutoAnalyzer::reset()
{
    delayCompensation.store(0);
}

TFAutoAnalyzer::AnalysisResult TFAutoAnalyzer::analyze(const float* reference, int refSamples,
                                                       const float* measurement, int measSamples,
                                                       const std::vector<float>& magnitudeDb,
                                                       const std::vector<float>& phaseDegrees,
                                                       const std::vector<float>& frequencies)
{
    AnalysisResult result;
    
    // 1. Detectar delay via cross-correlation
    if (refSamples > 0 && measSamples > 0 && reference != nullptr && measurement != nullptr)
    {
        int detectedDelay = detectDelayCrossCorrelation(reference, refSamples, measurement, measSamples);
        result.detectedDelaySamples = detectedDelay;
        result.detectedDelayMs = static_cast<float>(detectedDelay * 1000.0 / sampleRate);
        delayCompensation.store(detectedDelay);
        result.delayCompensated = (detectedDelay != 0);
    }
    
    // 2. Análise de magnitude
    if (!magnitudeDb.empty() && !frequencies.empty())
    {
        analyzeMagnitude(magnitudeDb, frequencies, result);
    }
    
    // 3. Análise de fase
    if (!phaseDegrees.empty() && !frequencies.empty())
    {
        analyzePhase(phaseDegrees, frequencies, result);
    }
    
    // 4. Calcular flatness score
    if (!magnitudeDb.empty())
    {
        result.overallFlatness = calculateFlatness(magnitudeDb);
    }
    
    // 5. Gerar resumo
    auto& strings = LocalizedStrings::getInstance();
    
    if (result.overallFlatness > 85.0f)
    {
        result.summary = strings.getTFSummaryExcellent();
    }
    else if (result.overallFlatness > 70.0f)
    {
        result.summary = strings.getTFSummaryAcceptable();
    }
    else if (result.overallFlatness > 50.0f)
    {
        result.summary = strings.getTFSummarySignificant();
    }
    else
    {
        result.summary = strings.getTFSummaryIrregular();
    }
    
    if (result.delayCompensated)
    {
        result.summary += strings.getTFDelayInfo(result.detectedDelayMs);
    }
    
    return result;
}

int TFAutoAnalyzer::detectDelayCrossCorrelation(const float* ref, int refSize,
                                                const float* meas, int measSize)
{
    if (refSize == 0 || measSize == 0 || ref == nullptr || meas == nullptr)
        return 0;
    
    // Limitar tamanho para performance
    int searchRange = juce::jmin(maxDelaySamples, refSize, measSize);
    if (searchRange < 64)  // Mínimo para análise confiável
        return 0;
    
    // Normalizar sinais
    float refRms = 0.0f;
    float measRms = 0.0f;
    for (int i = 0; i < searchRange; ++i)
    {
        refRms += ref[i] * ref[i];
        measRms += meas[i] * meas[i];
    }
    refRms = std::sqrt(refRms / searchRange);
    measRms = std::sqrt(measRms / searchRange);
    
    if (refRms < 1e-6f || measRms < 1e-6f)
        return 0;
    
    // Cross-correlation
    float maxCorrelation = -1.0f;
    int bestDelay = 0;
    
    // Procurar delay de -searchRange/2 a +searchRange/2
    int delayStart = -searchRange / 2;
    int delayEnd = searchRange / 2;
    
    for (int delay = delayStart; delay <= delayEnd; ++delay)
    {
        float correlation = 0.0f;
        int validSamples = 0;
        
        for (int i = 0; i < searchRange; ++i)
        {
            int refIdx = i;
            int measIdx = i + delay;
            
            if (measIdx >= 0 && measIdx < searchRange)
            {
                correlation += (ref[refIdx] / refRms) * (meas[measIdx] / measRms);
                validSamples++;
            }
        }
        
        if (validSamples > 0)
        {
            correlation /= validSamples;
            
            if (correlation > maxCorrelation)
            {
                maxCorrelation = correlation;
                bestDelay = delay;
            }
        }
    }
    
    // Só retornar delay se correlação for significativa (>0.3)
    if (maxCorrelation > 0.3f)
    {
        return bestDelay;
    }
    
    return 0;
}

void TFAutoAnalyzer::analyzeMagnitude(const std::vector<float>& magnitudeDb,
                                     const std::vector<float>& frequencies,
                                     AnalysisResult& result)
{
    if (magnitudeDb.size() != frequencies.size() || magnitudeDb.size() < 10)
        return;
    
    // Target: resposta plana (0 dB)
    const float tolerance = 3.0f;  // ±3dB considerado aceitável
    const float warningThreshold = 6.0f;  // ±6dB requer atenção
    
    // Análise por bandas de frequência
    struct FrequencyBand
    {
        float minFreq, maxFreq;
        juce::String name;
    };
    
    auto& strings = LocalizedStrings::getInstance();
    
    const FrequencyBand bands[] = {
        {20.0f, 100.0f, strings.getTFBandSubBass()},
        {100.0f, 250.0f, strings.getTFBandBass()},
        {250.0f, 500.0f, strings.getTFBandLowMid()},
        {500.0f, 2000.0f, strings.getTFBandMid()},
        {2000.0f, 5000.0f, strings.getTFBandHighMid()},
        {5000.0f, 10000.0f, strings.getTFBandHigh()},
        {10000.0f, 20000.0f, strings.getTFBandVeryHigh()}
    };
    
    for (const auto& band : bands)
    {
        float bandAvg = 0.0f;
        float bandMax = -100.0f;
        float bandMin = 100.0f;
        int count = 0;
        
        for (size_t i = 0; i < frequencies.size(); ++i)
        {
            if (frequencies[i] >= band.minFreq && frequencies[i] <= band.maxFreq)
            {
                float mag = magnitudeDb[i];
                bandAvg += mag;
                bandMax = juce::jmax(bandMax, mag);
                bandMin = juce::jmin(bandMin, mag);
                count++;
            }
        }
        
        if (count > 0)
        {
            bandAvg /= count;
            float deviation = std::abs(bandAvg);
            float range = bandMax - bandMin;
            
            if (deviation > warningThreshold || range > warningThreshold * 2)
            {
                result.magnitudeIssues.push_back((band.minFreq + band.maxFreq) / 2.0f);
                
                if (bandAvg > warningThreshold)
                {
                    result.magnitudeSuggestions.push_back(strings.getTFIssueBoost(band.name, bandAvg));
                }
                else if (bandAvg < -warningThreshold)
                {
                    result.magnitudeSuggestions.push_back(strings.getTFIssueCut(band.name, bandAvg));
                }
                
                if (range > warningThreshold * 2)
                {
                    result.magnitudeSuggestions.push_back(strings.getTFIssueVariation(band.name, range));
                }
            }
        }
    }
}

void TFAutoAnalyzer::analyzePhase(const std::vector<float>& phaseDegrees,
                                  const std::vector<float>& frequencies,
                                  AnalysisResult& result)
{
    if (phaseDegrees.size() != frequencies.size() || phaseDegrees.size() < 10)
        return;
    
    // Análise de linearidade de fase
    // Fase linear é ideal, mas pequenas variações são normais
    
    // Calcular desvio de linearidade
    float phaseRange = 0.0f;
    float phaseAvg = 0.0f;
    
    for (float phase : phaseDegrees)
    {
        phaseAvg += phase;
    }
    phaseAvg /= phaseDegrees.size();
    
    for (float phase : phaseDegrees)
    {
        float deviation = std::abs(phase - phaseAvg);
        phaseRange = juce::jmax(phaseRange, deviation);
    }
    
    auto& strings = LocalizedStrings::getInstance();
    
    // Detectar problemas específicos
    if (phaseRange > 90.0f)  // Variação de fase muito grande
    {
        result.phaseSuggestions.push_back(strings.getTFIssuePhaseVariation(phaseRange));
    }
    
    // Detectar wraps de fase problemáticos
    int wrapCount = 0;
    for (size_t i = 1; i < phaseDegrees.size(); ++i)
    {
        float diff = phaseDegrees[i] - phaseDegrees[i-1];
        if (std::abs(diff) > 180.0f)
        {
            wrapCount++;
        }
    }
    
    if (wrapCount > phaseDegrees.size() * 0.1f)  // Mais de 10% dos pontos têm wraps
    {
        result.phaseSuggestions.push_back(strings.getTFIssuePhaseWraps());
    }
    
    // Análise por bandas críticas
    struct CriticalBand
    {
        float minFreq, maxFreq;
        juce::String issue;
    };
    
    const CriticalBand criticalBands[] = {
        {200.0f, 500.0f, "Fase crítica em low-mid pode causar cancelamento"},
        {1000.0f, 3000.0f, "Fase em mid-range afeta clareza vocal"},
        {3000.0f, 8000.0f, "Fase em high-mid afeta definição e presença"}
    };
    
    for (const auto& band : criticalBands)
    {
        float bandPhaseRange = 0.0f;
        int count = 0;
        
        for (size_t i = 0; i < frequencies.size(); ++i)
        {
            if (frequencies[i] >= band.minFreq && frequencies[i] <= band.maxFreq)
            {
                if (count == 0)
                {
                    bandPhaseRange = phaseDegrees[i];
                }
                else
                {
                    float minPhase = juce::jmin(bandPhaseRange, phaseDegrees[i]);
                    float maxPhase = juce::jmax(bandPhaseRange, phaseDegrees[i]);
                    bandPhaseRange = maxPhase - minPhase;
                }
                count++;
            }
        }
        
        if (count > 0 && bandPhaseRange > 60.0f)
        {
            result.phaseIssues.push_back((band.minFreq + band.maxFreq) / 2.0f);
            juce::String freqRange = juce::String(band.minFreq, 0) + "-" + juce::String(band.maxFreq, 0) + "Hz";
            result.phaseSuggestions.push_back(strings.getTFIssuePhaseCritical(freqRange));
        }
    }
}

float TFAutoAnalyzer::calculateFlatness(const std::vector<float>& magnitudeDb)
{
    if (magnitudeDb.empty())
        return 0.0f;
    
    // Calcular desvio padrão da magnitude
    float mean = 0.0f;
    for (float mag : magnitudeDb)
    {
        mean += mag;
    }
    mean /= magnitudeDb.size();
    
    float variance = 0.0f;
    for (float mag : magnitudeDb)
    {
        float diff = mag - mean;
        variance += diff * diff;
    }
    variance /= magnitudeDb.size();
    float stdDev = std::sqrt(variance);
    
    // Score de 0-100 baseado em quão próximo de 0dB e quão plano
    // Ideal: média próxima de 0dB e stdDev baixo
    float meanScore = 100.0f - std::min(100.0f, std::abs(mean) * 10.0f);  // Penaliza desvio da média
    float flatnessScore = 100.0f - std::min(100.0f, stdDev * 5.0f);  // Penaliza variação
    
    return (meanScore + flatnessScore) / 2.0f;
}
