#pragma once

#include "../../JuceHeader.h"
#include "../../Core/TransferFunction/TFProcessor.h"

/**
 * MagnitudePlotComponent
 * 
 * Displays magnitude response of transfer function.
 * Blue graph with logarithmic frequency axis.
 */
class MagnitudePlotComponent : public juce::Component,
                               public juce::Timer
{
public:
    MagnitudePlotComponent(TFProcessor& processor);
    ~MagnitudePlotComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;
    
    void timerCallback() override;
    
private:
    void drawMagnitudeGraph(juce::Graphics& g, juce::Rectangle<int> bounds);
    float frequencyToX(float frequency, float width, float minFreq, float maxFreq);
    float magnitudeToY(float magnitudeDb, float height);
    
    TFProcessor& processor;
    
    std::vector<float> frequencies;
    std::vector<float> magnitudeData;
    std::vector<float> coherenceData;  // For coherence-based rendering
    
    static constexpr float minFrequency = 20.0f;
    static constexpr float maxFrequency = 20000.0f;
    static constexpr float minMagnitude = -18.0f;  // Smaart-like range
    static constexpr float maxMagnitude = 18.0f;   // Smaart-like range
    
    juce::Colour graphColour = juce::Colours::blue;
};
