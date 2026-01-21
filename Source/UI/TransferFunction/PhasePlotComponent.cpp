#include "PhasePlotComponent.h"
#include <cmath>

PhasePlotComponent::PhasePlotComponent(TFProcessor& proc)
    : processor(proc)
{
    // Start timer for real-time updates (60Hz = 16.67ms for smooth Smaart-like display)
    startTimer(17);
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
    processor.getCoherence(coherenceData);
    
    if (phaseData.empty() || frequencies.empty() || phaseData.size() != frequencies.size())
        return;
    
    // Ensure coherence data matches
    if (coherenceData.size() != phaseData.size())
        coherenceData.resize(phaseData.size(), 1.0f);
    
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
    
    // Frequency labels (Smaart-style: 31.5, 63, 125, 250, 500, 1k, 2k, 4k, 8k, 16k)
    // CRITICAL: Use the SAME frequencyToX function for ticks AND plot
    g.setFont(juce::Font(10.0f));
    g.setColour(juce::Colours::lightgrey);
    const float ticks[] = {31.5f, 63.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f};
    const juce::String tickLabels[] = {"31.5", "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"};
    
    for (int i = 0; i < 10; ++i)
    {
        float freq = ticks[i];
        if (freq < minFrequency || freq > maxFrequency)
            continue;
        
        // Use the SAME frequencyToX function that the plot uses
        float x = frequencyToX(freq, width, minFrequency, maxFrequency);
        
        // Draw vertical line at this position
        g.drawVerticalLine(static_cast<int>(padding + x), graphArea.getY(), graphArea.getBottom());
        
        // Draw label at the same position
        g.drawText(tickLabels[i], 
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
    
    // Draw phase curve with coherence-based alpha (Smaart-style)
    if (phaseData.size() < 2)
        return;
    
    // Draw in segments based on coherence
    juce::Path currentSegment;
    bool segmentStarted = false;
    
    for (size_t i = 0; i < phaseData.size(); ++i)
    {
        float freq = frequencies[i];
        if (freq < minFrequency || freq > maxFrequency)
            continue;
        
        float coh = (i < coherenceData.size()) ? coherenceData[i] : 1.0f;
        
        // Calculate alpha based on coherence (Smaart-style)
        // alpha = clamp((coh - 0.5)/0.5, 0, 1)
        float alpha = juce::jlimit(0.0f, 1.0f, (coh - 0.5f) / 0.5f);
        
        // Skip points with very low coherence (almost invisible)
        if (alpha < 0.1f)
        {
            // Break path segment
            if (segmentStarted)
            {
                g.setColour(graphColour.withAlpha(0.3f));
                g.strokePath(currentSegment, juce::PathStrokeType(1.5f));
                currentSegment.clear();
                segmentStarted = false;
            }
            continue;
        }
        
        float x = frequencyToX(freq, width, minFrequency, maxFrequency);
        float y = phaseToY(phaseData[i], height);
        
        float graphX = padding + x;
        float graphY = graphArea.getY() + y;
        
        if (!segmentStarted)
        {
            currentSegment.startNewSubPath(graphX, graphY);
            segmentStarted = true;
        }
        else
        {
            currentSegment.lineTo(graphX, graphY);
        }
        
        // Draw segment when coherence changes significantly or at end
        if (i == phaseData.size() - 1 || 
            (i > 0 && std::abs(alpha - juce::jlimit(0.0f, 1.0f, (coherenceData[i-1] - 0.5f) / 0.5f)) > 0.3f))
        {
            g.setColour(graphColour.withAlpha(0.3f + alpha * 0.7f));  // Range: 0.3 to 1.0
            g.strokePath(currentSegment, juce::PathStrokeType(1.5f));
            currentSegment.clear();
            segmentStarted = false;
        }
    }
    
    // Draw final segment if any
    if (segmentStarted)
    {
        g.setColour(graphColour);
        g.strokePath(currentSegment, juce::PathStrokeType(1.5f));
    }
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
    // Only repaint if visible (optimized for real-time performance)
    if (!isVisible())
    {
        stopTimer();
        return;
    }
    
    // Repaint for real-time updates (60Hz = smooth Smaart-like display)
    repaint();
}

void PhasePlotComponent::visibilityChanged()
{
    if (isVisible())
    {
        if (!isTimerRunning())
            startTimer(17);  // 60Hz for smooth real-time display (Smaart-like)
    }
    else
    {
        if (isTimerRunning())
            stopTimer();
    }
}
