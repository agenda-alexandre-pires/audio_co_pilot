#pragma once

#include "../../JuceHeader.h"
#include "../../Core/TransferFunction/TFProcessor.h"

/**
 * PhasePlotComponent
 * 
 * Displays phase response of transfer function.
 * Red graph with logarithmic frequency axis.
 */
class PhasePlotComponent : public juce::Component,
                           public juce::Timer
{
public:
    PhasePlotComponent(TFProcessor& processor);
    ~PhasePlotComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;
    
    void timerCallback() override;
    
private:
    void drawPhaseGraph(juce::Graphics& g, juce::Rectangle<int> bounds);
    float frequencyToX(float frequency, float width, float minFreq, float maxFreq);
    float phaseToY(float phaseDegrees, float height);
    
    TFProcessor& processor;
    
    std::vector<float> frequencies;
    std::vector<float> phaseData;
    
    static constexpr float minFrequency = 20.0f;
    static constexpr float maxFrequency = 20000.0f;
    static constexpr float minPhase = -180.0f;
    static constexpr float maxPhase = 180.0f;
    
    juce::Colour graphColour = juce::Colours::red;
};
