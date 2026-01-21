#pragma once

#include "../../JuceHeader.h"
#include "../../Core/TransferFunction/TFKnowledgeBase.h"
#include "../../Core/TransferFunction/TFAutoAnalyzer.h"

/**
 * TFAutoSuggestionsComponent
 * 
 * Componente para exibir sugestões automáticas de análise de Transfer Function.
 * Mostra análise de delay, magnitude, fase e sugestões de correção.
 */
class TFAutoSuggestionsComponent : public juce::Component,
                                   public juce::Timer
{
public:
    TFAutoSuggestionsComponent();
    ~TFAutoSuggestionsComponent() override;
    
    // Atualizar dados de análise
    void updateAnalysis(const TFAutoAnalyzer::AnalysisResult& result,
                       const std::vector<TFKnowledgeBase::Suggestion>& suggestions);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;
    
private:
    void paintSummary(juce::Graphics& g, juce::Rectangle<int> area);
    void paintSuggestions(juce::Graphics& g, juce::Rectangle<int> area);
    
    TFAutoAnalyzer::AnalysisResult currentResult;
    std::vector<TFKnowledgeBase::Suggestion> currentSuggestions;
    
    juce::CriticalSection dataLock;
    
    static constexpr int summaryHeight = 80;
    static constexpr int suggestionItemHeight = 60;
    static constexpr int maxVisibleSuggestions = 8;
};
