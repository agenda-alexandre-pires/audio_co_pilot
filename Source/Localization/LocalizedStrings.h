#pragma once

#include "../JuceHeader.h"

/**
 * LocalizedStrings
 * 
 * Runtime language switching system.
 * Supports English (default) and Portuguese (Brazil).
 */
class LocalizedStrings : public juce::ChangeBroadcaster
{
public:
    enum class Language
    {
        English,
        Portuguese_BR
    };
    
    static LocalizedStrings& getInstance();
    
    void setLanguage(Language lang);
    Language getCurrentLanguage() const { return currentLanguage; }
    
    // Menu Strings
    juce::String getMenuApp() const;
    juce::String getMenuAbout() const;
    juce::String getMenuQuit() const;
    juce::String getMenuSettings() const;
    juce::String getMenuLanguage() const;
    juce::String getMenuDevice() const;
    juce::String getMenuDeviceSelector() const;
    juce::String getMenuModules() const;
    juce::String getMenuModuleTransferFunction() const;
    juce::String getMenuModuleAntiMasking() const;
    juce::String getMenuModuleRTA() const;
    juce::String getMenuModuleAIStageHand() const;
    
    // UI Strings
    juce::String getDeviceSelectorLabel() const;
    juce::String getNoDeviceSelected() const;
    juce::String getInputChannels() const;
    juce::String getOutputChannels() const;
    juce::String getInputSelector() const;
    juce::String getOutputSelector() const;
    juce::String getChannelLabel(int channelNumber) const;

    // Anti-Masking UI strings
    juce::String getAntiMaskingTitle() const;
    juce::String getAntiMaskingTargetLabel() const;
    juce::String getAntiMaskingMaskerLabel(int maskerNumber) const;
    juce::String getAntiMaskingEnable() const;
    juce::String getAntiMaskingTargetSpectrumTitle() const;
    juce::String getAntiMaskingMaskerSpectrumTitle(int maskerNumber) const;
    juce::String getAntiMaskingSuggestionsTitle() const;
    juce::String getAntiMaskingSummaryTitle() const;
    juce::String getAntiMaskingSummaryLine(float audibility01, int criticalBands) const;

    // Branding
    juce::String getBrandLine() const; // "EOSTech © 2026"
    
    // Language Names
    juce::String getLanguageName(Language lang) const;
    juce::String getLanguageNameEnglish() const;
    juce::String getLanguageNamePortuguese() const;
    
private:
    LocalizedStrings() = default;
    ~LocalizedStrings() = default;
    LocalizedStrings(const LocalizedStrings&) = delete;
    LocalizedStrings& operator=(const LocalizedStrings&) = delete;
    
    Language currentLanguage{Language::English};
    
    juce::String getString(const juce::String& key) const;
};
