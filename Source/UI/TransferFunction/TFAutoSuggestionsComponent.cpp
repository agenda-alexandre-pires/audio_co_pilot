#include "TFAutoSuggestionsComponent.h"
#include "../DesignSystem/DesignSystem.h"
#include "../../Localization/LocalizedStrings.h"

TFAutoSuggestionsComponent::TFAutoSuggestionsComponent()
{
    startTimer(2000);  // Update every 2000ms (reduced for performance and to prevent flickering)
}

TFAutoSuggestionsComponent::~TFAutoSuggestionsComponent()
{
    stopTimer();
}

void TFAutoSuggestionsComponent::updateAnalysis(const TFAutoAnalyzer::AnalysisResult& result,
                                                const std::vector<TFKnowledgeBase::Suggestion>& suggestions)
{
    // Quick check without lock first (performance optimization)
    bool needsUpdate = false;
    {
        juce::ScopedLock lock(dataLock);
        
        // Only update if data actually changed significantly (prevent flickering)
        const float flatnessThreshold = 2.0f;
        const float delayThreshold = 0.5f;
        
        if (std::abs(currentResult.overallFlatness - result.overallFlatness) > flatnessThreshold ||
            currentResult.delayCompensated != result.delayCompensated ||
            std::abs(currentResult.detectedDelayMs - result.detectedDelayMs) > delayThreshold ||
            currentSuggestions.size() != suggestions.size() ||
            currentResult.summary != result.summary)
        {
            needsUpdate = true;
            currentResult = result;
            currentSuggestions = suggestions;
        }
    }
    
    // Only repaint if data changed significantly and component is visible
    if (needsUpdate && isVisible())
    {
        repaint();
    }
}

void TFAutoSuggestionsComponent::paint(juce::Graphics& g)
{
    using namespace AudioCoPilot::DesignSystem;
    
    g.fillAll(Colours::getColour(Colours::Surface::Background));
    
    auto bounds = getLocalBounds();
    
    // Summary area
    auto summaryArea = bounds.removeFromTop(summaryHeight);
    paintSummary(g, summaryArea);
    
    // Suggestions area
    paintSuggestions(g, bounds);
}

void TFAutoSuggestionsComponent::paintSummary(juce::Graphics& g, juce::Rectangle<int> area)
{
    using namespace AudioCoPilot::DesignSystem;
    
    juce::ScopedLock lock(dataLock);
    
    g.setColour(Colours::getColour(Colours::Surface::Elevated));
    g.fillRoundedRectangle(area.toFloat(), Spacing::RadiusSmall);
    
    g.setColour(Colours::getColour(Colours::Border::Default));
    g.drawRoundedRectangle(area.toFloat(), Spacing::RadiusSmall, 1.0f);
    
    auto textArea = area.reduced(Spacing::M);
    
    auto& strings = LocalizedStrings::getInstance();
    
    g.setColour(Colours::getColour(Colours::Text::Primary));
    g.setFont(Typography::headlineMedium());
    g.drawText(strings.getTFAutoAnalysisTitle(), textArea.removeFromTop(24), juce::Justification::left);
    
    textArea.removeFromTop(4);
    
    // Flatness score
    g.setFont(Typography::bodyMedium());
    juce::String flatnessText = strings.getTFFlatnessScore() + ": " + juce::String(currentResult.overallFlatness, 1) + "/100";
    g.drawText(flatnessText, textArea.removeFromTop(18), juce::Justification::left);
    
    textArea.removeFromTop(4);
    
    // Delay info
    if (currentResult.delayCompensated)
    {
        juce::String delayText = strings.getTFDelayDetected() + ": " + juce::String(currentResult.detectedDelayMs, 2) + "ms";
        g.setColour(Colours::getColour(Colours::Status::Warning));
        g.drawText(delayText, textArea.removeFromTop(18), juce::Justification::left);
    }
    
    // Summary
    if (!currentResult.summary.isEmpty())
    {
        textArea.removeFromTop(4);
        g.setColour(Colours::getColour(Colours::Text::Secondary));
        g.setFont(Typography::bodySmall());
        g.drawText(currentResult.summary, textArea, juce::Justification::left);
    }
}

void TFAutoSuggestionsComponent::paintSuggestions(juce::Graphics& g, juce::Rectangle<int> area)
{
    using namespace AudioCoPilot::DesignSystem;
    
    juce::ScopedLock lock(dataLock);
    
    auto& strings = LocalizedStrings::getInstance();
    
    if (currentSuggestions.empty())
    {
        g.setColour(Colours::getColour(Colours::Text::Secondary));
        g.setFont(Typography::bodyMedium());
        g.drawText(strings.getTFNoSuggestions(), area, juce::Justification::centred);
        return;
    }
    
    g.setColour(Colours::getColour(Colours::Text::Primary));
    g.setFont(Typography::headlineSmall());
    g.drawText(strings.getTFSuggestionsTitle(), area.removeFromTop(20), juce::Justification::left);
    area.removeFromTop(Spacing::XS);
    
    int visibleCount = juce::jmin(static_cast<int>(currentSuggestions.size()), maxVisibleSuggestions);
    int itemHeight = suggestionItemHeight;
    
    for (int i = 0; i < visibleCount; ++i)
    {
        auto itemArea = area.removeFromTop(itemHeight);
        
        // Background
        juce::Colour bgColour = Colours::getColour(Colours::Surface::Elevated);
        if (currentSuggestions[i].priority >= 4)
        {
            bgColour = Colours::getColour(Colours::Status::Warning).withAlpha(0.1f);
        }
        
        g.setColour(bgColour);
        g.fillRoundedRectangle(itemArea.toFloat(), Spacing::RadiusSmall);
        
        g.setColour(Colours::getColour(Colours::Border::Default));
        g.drawRoundedRectangle(itemArea.toFloat(), Spacing::RadiusSmall, 1.0f);
        
        auto textArea = itemArea.reduced(Spacing::XS);
        
        // Category and frequency
        g.setColour(Colours::getColour(Colours::Text::Accent));
        g.setFont(Typography::labelMedium());
        juce::String header = currentSuggestions[i].category + " - " + currentSuggestions[i].frequencyRange;
        g.drawText(header, textArea.removeFromTop(16), juce::Justification::left);
        
        textArea.removeFromTop(2);
        
        // Issue
        g.setColour(Colours::getColour(Colours::Text::Primary));
        g.setFont(Typography::bodySmall());
        g.drawText(currentSuggestions[i].issue, textArea.removeFromTop(14), juce::Justification::left);
        
        textArea.removeFromTop(2);
        
        // Action
        g.setColour(Colours::getColour(Colours::Text::Secondary));
        g.setFont(Typography::bodySmall());
        g.drawText(currentSuggestions[i].action, textArea, juce::Justification::left);
        
        area.removeFromTop(Spacing::XS);
    }
    
    if (static_cast<int>(currentSuggestions.size()) > maxVisibleSuggestions)
    {
        g.setColour(Colours::getColour(Colours::Text::Secondary));
        g.setFont(Typography::bodySmall());
        g.drawText(strings.getTFMoreSuggestions(static_cast<int>(currentSuggestions.size()) - maxVisibleSuggestions),
                  area.removeFromTop(16), juce::Justification::centred);
    }
}

void TFAutoSuggestionsComponent::resized()
{
    // Layout handled in paint()
}

void TFAutoSuggestionsComponent::timerCallback()
{
    // Only repaint if component is visible (performance optimization)
    if (isVisible())
    {
        repaint();
    }
}
