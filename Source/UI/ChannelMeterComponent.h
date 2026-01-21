#pragma once

#include "../JuceHeader.h"
#include "../Core/DeviceStateModel.h"
#include "../Localization/LocalizedStrings.h"
#include "DesignSystem/DesignSystem.h"

/**
 * ChannelMeterComponent
 * 
 * Real-time audio level meter displaying RMS and Peak levels.
 * Compact design for header area.
 */
class ChannelMeterComponent : public juce::Component,
                              public juce::Timer,
                              public juce::ChangeListener
{
public:
    ChannelMeterComponent(DeviceStateModel& stateModel, bool isInput);
    ~ChannelMeterComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // Calculate required dimensions based on number of channels
    int getRequiredWidth() const;
    int getRequiredHeight() const;
    
private:
    void updateMeterLevels();
    void updateTimerState(); // Start/stop timer based on device availability
    float levelToDb(float level) const;
    float dbToYPosition(float db, float height) const;  // For vertical meters (legacy)
    float dbToXPosition(float db, float width) const;   // For horizontal meters
    
    DeviceStateModel& stateModel;
    bool isInputChannel;
    
    static constexpr float minDb = -60.0f;
    static constexpr float maxDb = 0.0f;
    static constexpr float meterHoldTime = 0.3f; // seconds
    
    struct MeterState
    {
        float rmsDb{-60.0f};
        float peakDb{-60.0f};
        float peakHoldDb{-60.0f};
        double peakHoldTime{0.0};
    };
    
    std::vector<MeterState> meterStates;
    int numChannels{0};
    
    // Layout constants for vertical meters in grid layout
    static constexpr int maxMetersPerRow = 16;  // Maximum meters per row
    static constexpr int meterBarWidth = 20;  // Width of vertical meter bar
    static constexpr int meterBarHeight = 60;  // Height of vertical meter bar
    static constexpr int labelHeight = 15;  // Height for channel number label (below meter)
    static constexpr int horizontalSpacing = 8;  // Horizontal spacing between meters
    static constexpr int verticalSpacing = 6;  // Vertical spacing between rows (meter + label)
    
    juce::Font labelFont;
};
