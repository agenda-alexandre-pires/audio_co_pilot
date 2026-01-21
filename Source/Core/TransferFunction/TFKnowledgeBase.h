#pragma once

#include "../../JuceHeader.h"
#include <map>
#include <vector>

/**
 * TFKnowledgeBase
 * 
 * Base de conhecimento profissional para sugestões de correção de Transfer Function.
 * Similar ao AIStageHand, mas focado em análise de resposta de sistema.
 */
class TFKnowledgeBase
{
public:
    struct Suggestion
    {
        juce::String category;
        juce::String frequencyRange;
        juce::String issue;
        juce::String recommendation;
        juce::String action;
        int priority;  // 1-5, 5 = crítico
    };
    
    TFKnowledgeBase();
    ~TFKnowledgeBase();
    
    // Reinitialize knowledge base when language changes
    void reinitializeForLanguage();
    
    // Obter sugestões baseadas em análise
    std::vector<Suggestion> getSuggestions(float frequency,
                                          float magnitudeDeviation,
                                          float phaseDeviation,
                                          const juce::String& context = "");
    
    // Obter sugestões genéricas por banda
    std::vector<Suggestion> getBandSuggestions(const juce::String& bandName,
                                              float avgDeviation,
                                              float range);
    
private:
    void initializeKnowledgeBase();
    Suggestion createLocalizedSuggestion(const juce::String& category,
                                        const juce::String& freqRange,
                                        const juce::String& issueKey,
                                        const juce::String& recKey,
                                        const juce::String& actionKey,
                                        int priority);
    
    mutable juce::CriticalSection lock;  // Thread-safety for knowledge map
    std::map<juce::String, std::vector<Suggestion>> knowledgeMap;
};
