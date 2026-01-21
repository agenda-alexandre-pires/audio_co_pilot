#pragma once

#include "../../JuceHeader.h"
#include "RTAController.h"
#include "../../UI/DeviceSelectorComponent.h"

namespace AudioCoPilot
{

class RTAView : public juce::Component,
                public juce::Timer
{
public:
    RTAView(RTAController& controller, DeviceManager& deviceManager);
    ~RTAView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;
    
    void timerCallback() override;

private:
    RTAController& controller;
    
    // Reusing the existing Selector Device
    std::unique_ptr<DeviceSelectorComponent> deviceSelector;
    
    // Resolution Selector
    juce::ComboBox resolutionSelector;
    juce::Label resolutionLabel;
    
    // Colors for columns
    juce::Colour leftColor { juce::Colours::cyan };
    juce::Colour rightColor { juce::Colours::magenta };
};

}
