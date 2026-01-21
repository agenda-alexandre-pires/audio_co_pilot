#pragma once

#include "../../JuceHeader.h"
#include "../../UI/DesignSystem/DesignSystem.h"
#include "AIStageHandController.h"

namespace AudioCoPilot
{

/**
 * UI do módulo AIStageHand.
 * - Grid compacto de VU meters por canal
 * - Log de alertas com sugestões
 */
class AIStageHandView : public juce::Component,
                        private juce::Timer
{
public:
    explicit AIStageHandView(AIStageHandController& controller);
    ~AIStageHandView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AIStageHandController& controller;

    juce::StringArray alertCache;
    std::vector<float> levels;
    std::vector<double> alertsMs;

    juce::TextEditor alertBox;

    static constexpr int meterWidth = 16;
    static constexpr int meterSpacing = 8;
    static constexpr int maxPerRow = 16;
    static constexpr int headerHeight = 28;

    void timerCallback() override;
    void rebuildAlertText();
};

} // namespace AudioCoPilot
