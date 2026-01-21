#pragma once

#include "../../JuceHeader.h"
#include "AntiMaskingController.h"
#include "MaskingMatrixDisplay.h"
#include "BarkSpectrumDisplay.h"
#include "AISuggestionsEngine.h"

namespace AudioCoPilot
{
class AntiMaskingView : public juce::Component,
                        public juce::ComboBox::Listener,
                        public juce::Button::Listener,
                        public juce::ChangeListener,
                        public juce::Timer
{
public:
    AntiMaskingView (AntiMaskingController& c);
    ~AntiMaskingView() override;

    void resized() override;
    void paint (juce::Graphics& g) override;
    void visibilityChanged() override;

    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked (juce::Button* button) override;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    void timerCallback() override;

private:
    void rebuildChannelLists();

    AntiMaskingController& controller;

    juce::Label title;

    juce::Label targetLabel;
    juce::ComboBox targetCombo;

    std::array<juce::ToggleButton, 3> maskerEnable;
    std::array<juce::Label, 3> maskerLabel;
    std::array<juce::ComboBox, 3> maskerCombo;

    MaskingMatrixDisplay matrix;

    BarkSpectrumDisplay targetSpectrum;
    std::array<BarkSpectrumDisplay, 3> maskerSpectra;

    juce::Label suggestionsLabel;
    juce::TextEditor suggestionsBox;
};
}

