#pragma once

#include "../../JuceHeader.h"
#include "../DesignSystem/DesignSystem.h"

namespace AudioCoPilot
{
    /**
     * @brief Look and Feel profissional Industrial Brutalist
     * 
     * Implementa visual de qualidade mundial inspirado em:
     * - SSL consoles
     * - Smaart v9
     * - Waves plugins
     * - FabFilter
     */
    class IndustrialLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        IndustrialLookAndFeel();
        ~IndustrialLookAndFeel() override = default;
        
        // ============================================
        // FONTS
        // ============================================
        
        juce::Font getLabelFont(juce::Label& label) override;
        juce::Font getComboBoxFont(juce::ComboBox& box) override;
        juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;
        juce::Font getPopupMenuFont() override;
        juce::Font getAlertWindowTitleFont() override;
        juce::Font getAlertWindowMessageFont() override;
        juce::Font getAlertWindowFont() override;
        
        // ============================================
        // BUTTONS
        // ============================================
        
        void drawButtonBackground(juce::Graphics& g,
                                  juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override;
        
        void drawButtonText(juce::Graphics& g,
                            juce::TextButton& button,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;
        
        void drawToggleButton(juce::Graphics& g,
                              juce::ToggleButton& button,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
        
        // ============================================
        // COMBOBOX
        // ============================================
        
        void drawComboBox(juce::Graphics& g,
                          int width, int height,
                          bool isButtonDown,
                          int buttonX, int buttonY,
                          int buttonW, int buttonH,
                          juce::ComboBox& box) override;
        
        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
        
        // ============================================
        // TEXT EDITOR
        // ============================================
        
        void fillTextEditorBackground(juce::Graphics& g,
                                      int width, int height,
                                      juce::TextEditor& textEditor) override;
        
        void drawTextEditorOutline(juce::Graphics& g,
                                   int width, int height,
                                   juce::TextEditor& textEditor) override;
        
        // ============================================
        // SCROLL BAR
        // ============================================
        
        void drawScrollbar(juce::Graphics& g,
                           juce::ScrollBar& scrollbar,
                           int x, int y, int width, int height,
                           bool isScrollbarVertical,
                           int thumbStartPosition,
                           int thumbSize,
                           bool isMouseOver,
                           bool isMouseDown) override;
        
    private:
        // Cache de cores para performance
        juce::Colour _bgColour;
        juce::Colour _textColour;
        juce::Colour _accentColour;
        juce::Colour _borderColour;
        
        // Helpers
        void drawGlowEffect(juce::Graphics& g,
                            juce::Rectangle<float> bounds,
                            juce::Colour glowColour,
                            float radius);
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IndustrialLookAndFeel)
    };
}
