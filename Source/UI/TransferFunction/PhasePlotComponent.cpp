#include "PhasePlotComponent.h"
#include <cmath>

PhasePlotComponent::PhasePlotComponent(TFProcessor& proc)
    : processor(proc)
{
    // Timer will start when component becomes visible
}

PhasePlotComponent::~PhasePlotComponent()
{
    stopTimer();
}

void PhasePlotComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Background
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRect(bounds);
    
    // Title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("Phase Response", bounds.removeFromTop(25), juce::Justification::centredLeft);
    
    // Draw graph
    drawPhaseGraph(g, bounds);
}

void PhasePlotComponent::drawPhaseGraph(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Get latest data
    processor.getPhaseResponse(phaseData);
    processor.getFrequencyBins(frequencies);
    
    if (phaseData.empty() || frequencies.empty() || phaseData.size() != frequencies.size())
        return;
    
    const float padding = 40.0f;
    auto graphArea = bounds.reduced(static_cast<int>(padding));
    float width = static_cast<float>(graphArea.getWidth());
    float height = static_cast<float>(graphArea.getHeight());
    
    // Draw axes
    g.setColour(juce::Colours::darkgrey);
    
    // Horizontal axis (frequency, log scale)
    g.drawLine(padding, graphArea.getBottom(), graphArea.getRight(), graphArea.getBottom(), 1.0f);
    
    // Vertical axis (phase)
    g.drawLine(padding, graphArea.getY(), padding, graphArea.getBottom(), 1.0f);
    
    // Frequency labels (log scale)
    g.setFont(juce::Font(10.0f));
    g.setColour(juce::Colours::lightgrey);
    for (float freq = 100.0f; freq <= maxFrequency; freq *= 10.0f)
    {
        float x = frequencyToX(freq, width, minFrequency, maxFrequency);
        g.drawVerticalLine(static_cast<int>(padding + x), graphArea.getY(), graphArea.getBottom());
        g.drawText(juce::String(static_cast<int>(freq)) + " Hz", 
                   static_cast<int>(padding + x - 20), graphArea.getBottom() + 2, 40, 15,
                   juce::Justification::centred);
    }
    
    // Phase labels
    for (float phase = -180.0f; phase <= 180.0f; phase += 90.0f)
    {
        float y = phaseToY(phase, height);
        g.drawHorizontalLine(static_cast<int>(graphArea.getY() + y), 
                            graphArea.getX(), graphArea.getRight());
        g.drawText(juce::String(static_cast<int>(phase)) + "°", 
                   5, static_cast<int>(graphArea.getY() + y - 7), 35, 14,
                   juce::Justification::centredRight);
    }
    
    // Draw phase curve
    if (phaseData.size() < 2)
        return;
    
    juce::Path phasePath;
    bool pathStarted = false;
    
    for (size_t i = 0; i < phaseData.size(); ++i)
    {
        float freq = frequencies[i];
        if (freq < minFrequency || freq > maxFrequency)
            continue;
        
        float x = frequencyToX(freq, width, minFrequency, maxFrequency);
        float y = phaseToY(phaseData[i], height);
        
        float graphX = padding + x;
        float graphY = graphArea.getY() + y;
        
        if (!pathStarted)
        {
            phasePath.startNewSubPath(graphX, graphY);
            pathStarted = true;
        }
        else
        {
            phasePath.lineTo(graphX, graphY);
        }
    }
    
    g.setColour(graphColour);
    g.strokePath(phasePath, juce::PathStrokeType(2.0f));
}

float PhasePlotComponent::frequencyToX(float frequency, float width, float minFreq, float maxFreq)
{
    // Logarithmic scale
    float logMin = std::log10(minFreq);
    float logMax = std::log10(maxFreq);
    float logFreq = std::log10(frequency);
    
    return width * (logFreq - logMin) / (logMax - logMin);
}

float PhasePlotComponent::phaseToY(float phaseDegrees, float height)
{
    // Linear scale, -180° at top, +180° at bottom
    float normalized = (phaseDegrees - minPhase) / (maxPhase - minPhase);
    return height * (1.0f - normalized);
}

void PhasePlotComponent::resized()
{
    repaint();
}

void PhasePlotComponent::timerCallback()
{
    if (! isVisible())
    {
        stopTimer();
        return;
    }
    repaint();
}

void PhasePlotComponent::visibilityChanged()
{
    if (isVisible())
    {
        if (! isTimerRunning())
            startTimer(30);
    }
    else
    {
        if (isTimerRunning())
            stopTimer();
    }
}
