#pragma once

#include "../../JuceHeader.h"
#include <array>

namespace AudioCoPilot
{
class BarkSpectrumDisplay : public juce::Component
{
public:
    void setNameAndColour (juce::String n, juce::Colour c)
    {
        name = std::move (n);
        colour = c;
        repaint();
    }

    juce::Colour getColour() const noexcept { return colour; }

    void setSpectrumDb (const std::array<float, 24>& s)
    {
        spectrumDb = s;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff1a1a1a));
        g.fillRoundedRectangle (bounds, 4.0f);

        g.setColour (colour);
        g.setFont (juce::Font (11.0f));
        g.drawText (name, getLocalBounds().removeFromTop (16), juce::Justification::centredLeft);

        auto graph = bounds.reduced (6.0f);
        graph.removeFromTop (18.0f);

        const float minDb = -60.0f;
        const float maxDb = 0.0f;
        const float bandW = graph.getWidth() / 24.0f;

        // grid
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        for (float db = minDb; db <= maxDb; db += 12.0f)
        {
            const float y = juce::jmap (db, minDb, maxDb, graph.getBottom(), graph.getY());
            g.drawHorizontalLine ((int) y, graph.getX(), graph.getRight());
        }

        for (int b = 0; b < 24; ++b)
        {
            const float db = juce::jlimit (minDb, maxDb, spectrumDb[(size_t) b]);
            const float h = juce::jmap (db, minDb, maxDb, 0.0f, graph.getHeight());
            auto r = juce::Rectangle<float> (graph.getX() + b * bandW + 1.0f,
                                             graph.getBottom() - h,
                                             bandW - 2.0f,
                                             h);
            g.setColour (colour.withAlpha (0.85f));
            g.fillRect (r);
        }

        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }

private:
    juce::String name { "Spectrum" };
    juce::Colour colour { juce::Colours::deepskyblue };
    std::array<float, 24> spectrumDb { [] { std::array<float, 24> a {}; a.fill (-100.0f); return a; }() };
};
}

