#pragma once

#include "../../JuceHeader.h"
#include "../../Core/TransferFunction/TFController.h"

/**
 * TransferFunctionView
 * 
 * Main view for Transfer Function module.
 * Contains phase and magnitude plots, channel selectors.
 */
class TransferFunctionView : public juce::Component,
                             public juce::ComboBox::Listener,
                             public juce::ChangeListener,
                             public juce::Timer
{
public:
    TransferFunctionView(TFController& controller);
    ~TransferFunctionView() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void timerCallback() override;
    
private:
    void updateChannelSelectors();
    void updateSuggestions();
    
    TFController& controller;
    
    std::unique_ptr<juce::Label> referenceLabel;
    std::unique_ptr<juce::ComboBox> referenceChannelSelector;
    std::unique_ptr<juce::Label> measurementLabel;
    std::unique_ptr<juce::ComboBox> measurementChannelSelector;
    
    std::unique_ptr<juce::Label> delayLabel;
    std::unique_ptr<juce::Label> delayValueLabel;
    
    std::unique_ptr<class PhasePlotComponent> phasePlot;
    std::unique_ptr<class MagnitudePlotComponent> magnitudePlot;
    std::unique_ptr<class TFAutoSuggestionsComponent> suggestionsComponent;
};
