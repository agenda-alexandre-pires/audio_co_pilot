#include "MagnitudePlotComponent.h"
#include <cmath>

MagnitudePlotComponent::MagnitudePlotComponent(TFProcessor& proc)
    : processor(proc)
{
    // Timer will start when component becomes visible
}

MagnitudePlotComponent::~MagnitudePlotComponent()
{
    stopTimer();
}

void MagnitudePlotComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Background
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRect(bounds);
    
    // Title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("Magnitude Response", bounds.removeFromTop(25), juce::Justification::centredLeft);
    
    // Draw graph
    drawMagnitudeGraph(g, bounds);
}

void MagnitudePlotComponent::drawMagnitudeGraph(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Get latest data
    processor.getMagnitudeResponse(magnitudeData);
    processor.getFrequencyBins(frequencies);
    
    if (magnitudeData.empty() || frequencies.empty() || magnitudeData.size() != frequencies.size())
        return;
    
    const float padding = 40.0f;
    auto graphArea = bounds.reduced(static_cast<int>(padding));
    float width = static_cast<float>(graphArea.getWidth());
    float height = static_cast<float>(graphArea.getHeight());
    
    // Draw axes
    g.setColour(juce::Colours::darkgrey);
    
    // Horizontal axis (frequency, log scale)
    g.drawLine(padding, graphArea.getBottom(), graphArea.getRight(), graphArea.getBottom(), 1.0f);
    
    // Vertical axis (magnitude)
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
    
    // Magnitude labels
    for (float mag = minMagnitude; mag <= maxMagnitude; mag += 20.0f)
    {
        float y = magnitudeToY(mag, height);
        g.drawHorizontalLine(static_cast<int>(graphArea.getY() + y), 
                            graphArea.getX(), graphArea.getRight());
        g.drawText(juce::String(static_cast<int>(mag)) + " dB", 
                   5, static_cast<int>(graphArea.getY() + y - 7), 35, 14,
                   juce::Justification::centredRight);
    }
    
    // Draw magnitude curve
    if (magnitudeData.size() < 2)
        return;
    
    juce::Path magnitudePath;
    bool pathStarted = false;
    
    for (size_t i = 0; i < magnitudeData.size(); ++i)
    {
        float freq = frequencies[i];
        if (freq < minFrequency || freq > maxFrequency)
            continue;
        
        float x = frequencyToX(freq, width, minFrequency, maxFrequency);
        float y = magnitudeToY(magnitudeData[i], height);
        
        float graphX = padding + x;
        float graphY = graphArea.getY() + y;
        
        if (!pathStarted)
        {
            magnitudePath.startNewSubPath(graphX, graphY);
            pathStarted = true;
        }
        else
        {
            magnitudePath.lineTo(graphX, graphY);
        }
    }
    
    g.setColour(graphColour);
    g.strokePath(magnitudePath, juce::PathStrokeType(2.0f));
}

float MagnitudePlotComponent::frequencyToX(float frequency, float width, float minFreq, float maxFreq)
{
    // Logarithmic scale
    float logMin = std::log10(minFreq);
    float logMax = std::log10(maxFreq);
    float logFreq = std::log10(frequency);
    
    return width * (logFreq - logMin) / (logMax - logMin);
}

float MagnitudePlotComponent::magnitudeToY(float magnitudeDb, float height)
{
    // Linear scale, max dB at top, min dB at bottom
    float normalized = (magnitudeDb - minMagnitude) / (maxMagnitude - minMagnitude);
    return height * (1.0f - normalized);
}

void MagnitudePlotComponent::resized()
{
    repaint();
}

void MagnitudePlotComponent::timerCallback()
{
    if (! isVisible())
    {
        stopTimer();
        return;
    }
    repaint();
}

void MagnitudePlotComponent::visibilityChanged()
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
