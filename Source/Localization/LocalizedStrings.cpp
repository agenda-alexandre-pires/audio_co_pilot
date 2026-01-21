#include "LocalizedStrings.h"

LocalizedStrings& LocalizedStrings::getInstance()
{
    static LocalizedStrings instance;
    return instance;
}

void LocalizedStrings::setLanguage(Language lang)
{
    if (currentLanguage != lang)
    {
        currentLanguage = lang;
        sendChangeMessage();
    }
}

juce::String LocalizedStrings::getMenuApp() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Aplicativo") : "App";
}

juce::String LocalizedStrings::getMenuAbout() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Sobre") : "About";
}

juce::String LocalizedStrings::getMenuQuit() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Sair") : "Quit";
}

juce::String LocalizedStrings::getMenuSettings() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Configurações") : "Settings";
}

juce::String LocalizedStrings::getMenuLanguage() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Idioma") : "Language";
}

juce::String LocalizedStrings::getMenuDevice() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Dispositivo") : "Device";
}

juce::String LocalizedStrings::getMenuDeviceSelector() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Seletor de Dispositivo") : "Device Selector";
}

juce::String LocalizedStrings::getMenuModules() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Módulos") : "Modules";
}

juce::String LocalizedStrings::getMenuModuleTransferFunction() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Função de Transferência") : "Transfer Function";
}

juce::String LocalizedStrings::getMenuModuleAntiMasking() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Anti-Mascaramento") : "Anti-Masking";
}

juce::String LocalizedStrings::getMenuModuleRTA() const
{
    return "RTA"; // Same for both currently, or "Analisador em Tempo Real"
}

juce::String LocalizedStrings::getMenuModuleAIStageHand() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("AI Stage Hand (Feedback)") : "AI Stage Hand (Feedback)";
}

juce::String LocalizedStrings::getDeviceSelectorLabel() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Dispositivo de Áudio:") : "Audio Device:";
}

juce::String LocalizedStrings::getNoDeviceSelected() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Nenhum dispositivo selecionado") : "No device selected";
}

juce::String LocalizedStrings::getInputChannels() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Entrada") : "Input";
}

juce::String LocalizedStrings::getOutputChannels() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Saída") : "Output";
}

juce::String LocalizedStrings::getInputSelector() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Seletor de Entrada") : "Input Selector";
}

juce::String LocalizedStrings::getOutputSelector() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Seletor de Saída") : "Output Selector";
}

juce::String LocalizedStrings::getChannelLabel(int channelNumber) const
{
    return currentLanguage == Language::Portuguese_BR 
        ? (juce::String::fromUTF8("Canal ") + juce::String(channelNumber + 1))
        : ("Channel " + juce::String(channelNumber + 1));
}

juce::String LocalizedStrings::getAntiMaskingTitle() const
{
    return currentLanguage == Language::Portuguese_BR
        ? juce::String::fromUTF8("Módulo Anti-Mascaramento (Bark)")
        : "Anti-Masking Module (Bark)";
}

juce::String LocalizedStrings::getAntiMaskingTargetLabel() const
{
    return currentLanguage == Language::Portuguese_BR ? "Target:" : "Target:";
}

juce::String LocalizedStrings::getAntiMaskingMaskerLabel(int maskerNumber) const
{
    if (currentLanguage == Language::Portuguese_BR)
        return juce::String::fromUTF8("Mascarador ") + juce::String(maskerNumber) + ":";
    return "Masker " + juce::String(maskerNumber) + ":";
}

juce::String LocalizedStrings::getAntiMaskingEnable() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Ativar") : "Enable";
}

juce::String LocalizedStrings::getAntiMaskingTargetSpectrumTitle() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Target (Bark)") : "Target (Bark)";
}

juce::String LocalizedStrings::getAntiMaskingMaskerSpectrumTitle(int maskerNumber) const
{
    if (currentLanguage == Language::Portuguese_BR)
        return juce::String::fromUTF8("Mascarador ") + juce::String(maskerNumber) + " (Bark)";
    return "Masker " + juce::String(maskerNumber) + " (Bark)";
}

juce::String LocalizedStrings::getAntiMaskingSuggestionsTitle() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Sugestões (IA)") : "AI Suggestions";
}

juce::String LocalizedStrings::getAntiMaskingSummaryTitle() const
{
    return currentLanguage == Language::Portuguese_BR ? juce::String::fromUTF8("Resumo") : "Summary";
}

juce::String LocalizedStrings::getAntiMaskingSummaryLine(float audibility01, int criticalBands) const
{
    if (currentLanguage == Language::Portuguese_BR)
    {
        return juce::String::fromUTF8("Audibilidade geral: ") + juce::String(audibility01, 2)
             + juce::String::fromUTF8(" | Bandas críticas: ") + juce::String(criticalBands);
    }

    return "Overall audibility: " + juce::String(audibility01, 2)
         + " | Critical bands: " + juce::String(criticalBands);
}

juce::String LocalizedStrings::getBrandLine() const
{
    juce::ignoreUnused (currentLanguage);
    return "EOSTech " + juce::String::fromUTF8("©") + " 2026";
}

juce::String LocalizedStrings::getLanguageName(Language lang) const
{
    switch (lang)
    {
        case Language::English:
            return "English";
        case Language::Portuguese_BR:
            return juce::String::fromUTF8("Português (Brasil)");
        default:
            return "English";
    }
}

juce::String LocalizedStrings::getLanguageNameEnglish() const
{
    return "English";
}

juce::String LocalizedStrings::getLanguageNamePortuguese() const
{
    return juce::String::fromUTF8("Português (Brasil)");
}

juce::String LocalizedStrings::getString(const juce::String& key) const
{
    // Placeholder for future string table implementation
    return key;
}
