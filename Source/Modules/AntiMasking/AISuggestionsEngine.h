#pragma once

#include "../../JuceHeader.h"
#include "MaskingCalculator.h"
#include "BarkAnalyzer.h"
#include "../../Localization/LocalizedStrings.h"
#include <vector>

namespace AudioCoPilot
{
struct AISuggestion
{
    juce::String title;
    juce::String details;
    float severity01 { 0.0f };
};

class AISuggestionsEngine
{
public:
    // Returns a small list of actionable suggestions.
    static std::vector<AISuggestion> generate (const MaskingAnalysisResult& r, const LocalizedStrings& strings)
    {
        std::vector<AISuggestion> out;
        out.reserve (6);

        const bool pt = (strings.getCurrentLanguage() == LocalizedStrings::Language::Portuguese_BR);

        // Find worst bands by SMR
        struct BandScore { int band; float smr; };
        std::array<BandScore, 24> scores {};
        for (int b = 0; b < 24; ++b)
            scores[(size_t) b] = { b, r.bands[(size_t) b].smrDb };

        std::sort (scores.begin(), scores.end(), [] (auto a, auto b) { return a.smr < b.smr; });

        const int topN = 6;
        for (int i = 0; i < topN; ++i)
        {
            const int band = scores[(size_t) i].band;
            const auto edges = BarkAnalyzer::getBandEdgesHz (band);
            const float smr = scores[(size_t) i].smr;
            const float maskedness = juce::jlimit (0.0f, 1.0f, juce::jmap (smr, 10.0f, -10.0f, 0.0f, 1.0f));

            const int dom = r.bands[(size_t) band].dominantMasker;

            // Suggest cut on dominant masker around that band
            if (dom >= 0 && dom < 3 && maskedness > 0.35f)
            {
                const float cutDb = juce::jlimit (1.0f, 6.0f, 2.0f + 4.0f * maskedness);
                AISuggestion s;
                s.severity01 = maskedness;
                if (pt)
                {
                    s.title = juce::String::fromUTF8("Corte no Mascarador ") + juce::String (dom + 1)
                              + " em " + juce::String ((int) edges.first) + "-" + juce::String ((int) edges.second) + " Hz";
                    s.details = "SMR " + juce::String (smr, 1) + " dB (" + juce::String::fromUTF8("mascaramento alto") + "). "
                                + juce::String::fromUTF8("Sugestão") + ": EQ bell Q~1.0 com -" + juce::String (cutDb, 1) + " dB.";
                }
                else
                {
                    s.title = "Cut Masker " + juce::String (dom + 1)
                              + " at " + juce::String ((int) edges.first) + "-" + juce::String ((int) edges.second) + " Hz";
                    s.details = "SMR " + juce::String (smr, 1) + " dB (high masking). "
                                "Suggestion: bell EQ Q~1.0 with -" + juce::String (cutDb, 1) + " dB.";
                }
                out.push_back (std::move (s));
            }

            // Suggest small boost on target if not clipping (heuristic)
            if (maskedness > 0.55f)
            {
                const float boostDb = juce::jlimit (0.5f, 3.0f, 0.8f + 2.2f * maskedness);
                AISuggestion s;
                s.severity01 = maskedness * 0.8f;
                if (pt)
                {
                    s.title = juce::String::fromUTF8("Realce do Target em ") + juce::String ((int) edges.first)
                              + "-" + juce::String ((int) edges.second) + " Hz";
                    s.details = "SMR " + juce::String (smr, 1) + " dB. "
                                + juce::String::fromUTF8("Sugestão") + ": +" + juce::String (boostDb, 1)
                                + " dB (EQ bell Q~0.8) " + juce::String::fromUTF8("se houver headroom") + ".";
                }
                else
                {
                    s.title = "Boost Target at " + juce::String ((int) edges.first) + "-" + juce::String ((int) edges.second) + " Hz";
                    s.details = "SMR " + juce::String (smr, 1) + " dB. "
                                "Suggestion: +" + juce::String (boostDb, 1) + " dB (bell EQ Q~0.8) if headroom allows.";
                }
                out.push_back (std::move (s));
            }
        }

        // Global summary
        {
            AISuggestion s;
            s.severity01 = juce::jlimit (0.0f, 1.0f, 1.0f - r.overallAudibility01);
            s.title = strings.getAntiMaskingSummaryTitle();
            s.details = strings.getAntiMaskingSummaryLine (r.overallAudibility01, r.criticalBandCount);
            out.insert (out.begin(), std::move (s));
        }

        return out;
    }
};
}

