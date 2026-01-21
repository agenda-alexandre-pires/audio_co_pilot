#pragma once

#include "../../JuceHeader.h"
#include "MaskingCalculator.h"
#include "../../Localization/LocalizedStrings.h"

namespace AudioCoPilot
{
class MaskingMatrixDisplay : public juce::Component
{
public:
    void setResult (const MaskingAnalysisResult& r)
    {
        result = r;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto& strings = LocalizedStrings::getInstance();
        auto bounds = getLocalBounds().toFloat();
        g.fillAll (juce::Colour (0xff1a1a1a));

        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (13.0f, juce::Font::bold));
        g.drawText (strings.getAntiMaskingTitle(), getLocalBounds().removeFromTop (22), juce::Justification::centred);

        auto graph = bounds.reduced (10.0f);
        graph.removeFromTop (26.0f);
        graph.removeFromBottom (18.0f);

        const float bandW = graph.getWidth() / 24.0f;

        for (int b = 0; b < 24; ++b)
        {
            const float aud = result.bands[(size_t) b].audibility01;
            const int dom = result.bands[(size_t) b].dominantMasker;

            // Green (clear) -> Red (masked)
            auto base = juce::Colour::fromHSV (juce::jmap (aud, 0.0f, 1.0f, 0.0f, 0.33f), 0.85f, 0.95f, 1.0f);

            // Tint by dominant masker (optional)
            if (dom == 0) base = base.interpolatedWith (juce::Colours::deepskyblue, 0.25f);
            if (dom == 1) base = base.interpolatedWith (juce::Colours::magenta, 0.25f);
            if (dom == 2) base = base.interpolatedWith (juce::Colours::greenyellow, 0.25f);

            auto r = juce::Rectangle<float> (graph.getX() + b * bandW, graph.getY(), bandW - 1.0f, graph.getHeight());

            // Height indicates maskedness
            const float maskedness = 1.0f - aud;
            auto bar = r.withTop (r.getBottom() - r.getHeight() * maskedness);
            g.setColour (base.withAlpha (0.85f));
            g.fillRect (bar);

            g.setColour (juce::Colours::black.withAlpha (0.35f));
            g.drawRect (r);
        }

        // Summary
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.setFont (11.0f);
        auto bottom = getLocalBounds().removeFromBottom (18);
        g.drawText (strings.getAntiMaskingSummaryLine (result.overallAudibility01, result.criticalBandCount),
                    bottom, juce::Justification::centredLeft);
    }

private:
    MaskingAnalysisResult result {};
};
}

