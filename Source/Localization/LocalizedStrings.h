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
    Language getCurrentLanguage() const;
    
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
    
    // Transfer Function UI strings
    juce::String getTFReferenceChannel() const;
    juce::String getTFMeasurementChannel() const;
    juce::String getTFAutoAnalysisTitle() const;
    juce::String getTFFlatnessScore() const;
    juce::String getTFDelayDetected() const;
    juce::String getTFSuggestionsTitle() const;
    juce::String getTFNoSuggestions() const;
    juce::String getTFMoreSuggestions(int count) const;
    
    // Transfer Function Analysis strings
    juce::String getTFSummaryExcellent() const;
    juce::String getTFSummaryAcceptable() const;
    juce::String getTFSummarySignificant() const;
    juce::String getTFSummaryIrregular() const;
    juce::String getTFDelayInfo(float ms) const;
    
    // Transfer Function Band names
    juce::String getTFBandSubBass() const;
    juce::String getTFBandBass() const;
    juce::String getTFBandLowMid() const;
    juce::String getTFBandMid() const;
    juce::String getTFBandHighMid() const;
    juce::String getTFBandHigh() const;
    juce::String getTFBandVeryHigh() const;
    
    // Transfer Function Issue strings
    juce::String getTFIssueBoost(const juce::String& band, float db) const;
    juce::String getTFIssueCut(const juce::String& band, float db) const;
    juce::String getTFIssueVariation(const juce::String& band, float range) const;
    juce::String getTFIssuePhaseVariation(float range) const;
    juce::String getTFIssuePhaseWraps() const;
    juce::String getTFIssuePhaseCritical(const juce::String& freqRange) const;
    
    // Transfer Function Knowledge Base strings
    juce::String getTFKBSubBassBoostIssue() const;
    juce::String getTFKBSubBassBoostRec() const;
    juce::String getTFKBSubBassBoostAction() const;
    juce::String getTFKBSubBassCutIssue() const;
    juce::String getTFKBSubBassCutRec() const;
    juce::String getTFKBSubBassCutAction() const;
    juce::String getTFKBBassBoostIssue() const;
    juce::String getTFKBBassBoostRec() const;
    juce::String getTFKBBassBoostAction() const;
    juce::String getTFKBBassCutIssue() const;
    juce::String getTFKBBassCutRec() const;
    juce::String getTFKBBassCutAction() const;
    juce::String getTFKBLowMidBoostIssue() const;
    juce::String getTFKBLowMidBoostRec() const;
    juce::String getTFKBLowMidBoostAction() const;
    juce::String getTFKBLowMidCutIssue() const;
    juce::String getTFKBLowMidCutRec() const;
    juce::String getTFKBLowMidCutAction() const;
    juce::String getTFKBMidBoostIssue() const;
    juce::String getTFKBMidBoostRec() const;
    juce::String getTFKBMidBoostAction() const;
    juce::String getTFKBMidCutIssue() const;
    juce::String getTFKBMidCutRec() const;
    juce::String getTFKBMidCutAction() const;
    juce::String getTFKBHighMidBoostIssue() const;
    juce::String getTFKBHighMidBoostRec() const;
    juce::String getTFKBHighMidBoostAction() const;
    juce::String getTFKBHighMidCutIssue() const;
    juce::String getTFKBHighMidCutRec() const;
    juce::String getTFKBHighMidCutAction() const;
    juce::String getTFKBHighBoostIssue() const;
    juce::String getTFKBHighBoostRec() const;
    juce::String getTFKBHighBoostAction() const;
    juce::String getTFKBHighCutIssue() const;
    juce::String getTFKBHighCutRec() const;
    juce::String getTFKBHighCutAction() const;
    juce::String getTFKBVeryHighBoostIssue() const;
    juce::String getTFKBVeryHighBoostRec() const;
    juce::String getTFKBVeryHighBoostAction() const;
    juce::String getTFKBVeryHighCutIssue() const;
    juce::String getTFKBVeryHighCutRec() const;
    juce::String getTFKBVeryHighCutAction() const;
    juce::String getTFKBPhaseLinearityIssue() const;
    juce::String getTFKBPhaseLinearityRec() const;
    juce::String getTFKBPhaseLinearityAction() const;
    juce::String getTFKBPhaseWrapsIssue() const;
    juce::String getTFKBPhaseWrapsRec() const;
    juce::String getTFKBPhaseWrapsAction() const;
    juce::String getTFKBVariationIssue() const;
    juce::String getTFKBVariationRec() const;
    juce::String getTFKBVariationAction() const;

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
    
    mutable juce::CriticalSection lock;  // Thread-safety for concurrent access
    Language currentLanguage{Language::English};
    
    // Internal helper (not thread-safe, use with lock)
    Language getLanguageUnsafe() const;
    
    juce::String getString(const juce::String& key) const;
};
