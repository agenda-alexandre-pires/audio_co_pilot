#include "LocalizedStrings.h"

// Helper macro to safely get language and release lock before string operations
#define GET_LANGUAGE_SAFE() \
    juce::ScopedLock sl(lock); \
    Language lang = currentLanguage;

LocalizedStrings& LocalizedStrings::getInstance()
{
    static LocalizedStrings instance;
    return instance;
}

LocalizedStrings::Language LocalizedStrings::getCurrentLanguage() const
{
    juce::ScopedLock sl(lock);
    return currentLanguage;
}

// Helper to get language safely (used internally)
LocalizedStrings::Language LocalizedStrings::getLanguageUnsafe() const
{
    return currentLanguage;
}

void LocalizedStrings::setLanguage(Language lang)
{
    juce::ScopedLock sl(lock);
    if (currentLanguage != lang)
    {
        currentLanguage = lang;
        sendChangeMessage();
    }
}

juce::String LocalizedStrings::getMenuApp() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Aplicativo") : "App";
}

juce::String LocalizedStrings::getMenuAbout() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Sobre") : "About";
}

juce::String LocalizedStrings::getMenuQuit() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Sair") : "Quit";
}

juce::String LocalizedStrings::getMenuSettings() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Configurações") : "Settings";
}

juce::String LocalizedStrings::getMenuLanguage() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Idioma") : "Language";
}

juce::String LocalizedStrings::getMenuDevice() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Dispositivo") : "Device";
}

juce::String LocalizedStrings::getMenuDeviceSelector() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Seletor de Dispositivo") : "Device Selector";
}

juce::String LocalizedStrings::getMenuModules() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Módulos") : "Modules";
}

juce::String LocalizedStrings::getMenuModuleTransferFunction() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Função de Transferência") : "Transfer Function";
}

juce::String LocalizedStrings::getMenuModuleAntiMasking() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Anti-Mascaramento") : "Anti-Masking";
}

juce::String LocalizedStrings::getMenuModuleRTA() const
{
    return "RTA"; // Same for both currently, or "Analisador em Tempo Real"
}

juce::String LocalizedStrings::getMenuModuleAIStageHand() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("AI Stage Hand (Feedback)") : "AI Stage Hand (Feedback)";
}

juce::String LocalizedStrings::getDeviceSelectorLabel() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Dispositivo de Áudio:") : "Audio Device:";
}

juce::String LocalizedStrings::getNoDeviceSelected() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Nenhum dispositivo selecionado") : "No device selected";
}

juce::String LocalizedStrings::getInputChannels() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Entrada") : "Input";
}

juce::String LocalizedStrings::getOutputChannels() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Saída") : "Output";
}

juce::String LocalizedStrings::getInputSelector() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Seletor de Entrada") : "Input Selector";
}

juce::String LocalizedStrings::getOutputSelector() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Seletor de Saída") : "Output Selector";
}

juce::String LocalizedStrings::getChannelLabel(int channelNumber) const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR 
        ? (juce::String::fromUTF8("Canal ") + juce::String(channelNumber + 1))
        : ("Channel " + juce::String(channelNumber + 1));
}

juce::String LocalizedStrings::getAntiMaskingTitle() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR
        ? juce::String::fromUTF8("Módulo Anti-Mascaramento (Bark)")
        : "Anti-Masking Module (Bark)";
}

juce::String LocalizedStrings::getAntiMaskingTargetLabel() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? "Target:" : "Target:";
}

juce::String LocalizedStrings::getAntiMaskingMaskerLabel(int maskerNumber) const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
        return juce::String::fromUTF8("Mascarador ") + juce::String(maskerNumber) + ":";
    return "Masker " + juce::String(maskerNumber) + ":";
}

juce::String LocalizedStrings::getAntiMaskingEnable() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Ativar") : "Enable";
}

juce::String LocalizedStrings::getAntiMaskingTargetSpectrumTitle() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Target (Bark)") : "Target (Bark)";
}

juce::String LocalizedStrings::getAntiMaskingMaskerSpectrumTitle(int maskerNumber) const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
        return juce::String::fromUTF8("Mascarador ") + juce::String(maskerNumber) + " (Bark)";
    return "Masker " + juce::String(maskerNumber) + " (Bark)";
}

juce::String LocalizedStrings::getAntiMaskingSuggestionsTitle() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Sugestões (IA)") : "AI Suggestions";
}

juce::String LocalizedStrings::getAntiMaskingSummaryTitle() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Resumo") : "Summary";
}

juce::String LocalizedStrings::getAntiMaskingSummaryLine(float audibility01, int criticalBands) const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
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

// Transfer Function UI strings
juce::String LocalizedStrings::getTFReferenceChannel() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Canal de Referência:") : "Reference Channel:";
}

juce::String LocalizedStrings::getTFMeasurementChannel() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Canal de Medição:") : "Measurement Channel:";
}

juce::String LocalizedStrings::getTFAutoAnalysisTitle() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Análise Automática") : "Auto Analysis";
}

juce::String LocalizedStrings::getTFFlatnessScore() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Pontuação de Planicidade") : "Flatness Score";
}

juce::String LocalizedStrings::getTFDelayDetected() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Delay detectado") : "Delay detected";
}

juce::String LocalizedStrings::getTFSuggestionsTitle() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Sugestões de Correção:") : "Correction Suggestions:";
}

juce::String LocalizedStrings::getTFNoSuggestions() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR 
        ? juce::String::fromUTF8("Nenhuma sugestão no momento. Sistema com resposta plana.")
        : "No suggestions at the moment. System with flat response.";
}

juce::String LocalizedStrings::getTFMoreSuggestions(int count) const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
        return juce::String::fromUTF8("... e mais ") + juce::String(count) + juce::String::fromUTF8(" sugestões");
    return "... and " + juce::String(count) + " more suggestions";
}

juce::String LocalizedStrings::getTFSummaryExcellent() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR 
        ? juce::String::fromUTF8("Resposta plana excelente. Sistema bem calibrado.")
        : "Excellent flat response. System well calibrated.";
}

juce::String LocalizedStrings::getTFSummaryAcceptable() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR 
        ? juce::String::fromUTF8("Resposta aceitável com pequenas variações. Considere ajustes finos.")
        : "Acceptable response with minor variations. Consider fine adjustments.";
}

juce::String LocalizedStrings::getTFSummarySignificant() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR 
        ? juce::String::fromUTF8("Resposta com desvios significativos. Correções recomendadas.")
        : "Response with significant deviations. Corrections recommended.";
}

juce::String LocalizedStrings::getTFSummaryIrregular() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR 
        ? juce::String::fromUTF8("Resposta muito irregular. Correções importantes necessárias.")
        : "Very irregular response. Important corrections necessary.";
}

juce::String LocalizedStrings::getTFDelayInfo(float ms) const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
        return juce::String::fromUTF8(" Delay detectado: ") + juce::String(ms, 2) + "ms.";
    return " Delay detected: " + juce::String(ms, 2) + "ms.";
}

juce::String LocalizedStrings::getTFBandSubBass() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Sub-bass (20-100Hz)") : "Sub-bass (20-100Hz)";
}

juce::String LocalizedStrings::getTFBandBass() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Bass (100-250Hz)") : "Bass (100-250Hz)";
}

juce::String LocalizedStrings::getTFBandLowMid() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Low-mid (250-500Hz)") : "Low-mid (250-500Hz)";
}

juce::String LocalizedStrings::getTFBandMid() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Mid (500-2kHz)") : "Mid (500-2kHz)";
}

juce::String LocalizedStrings::getTFBandHighMid() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("High-mid (2-5kHz)") : "High-mid (2-5kHz)";
}

juce::String LocalizedStrings::getTFBandHigh() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("High (5-10kHz)") : "High (5-10kHz)";
}

juce::String LocalizedStrings::getTFBandVeryHigh() const
{
    GET_LANGUAGE_SAFE()
    return lang == Language::Portuguese_BR ? juce::String::fromUTF8("Very High (10-20kHz)") : "Very High (10-20kHz)";
}

juce::String LocalizedStrings::getTFIssueBoost(const juce::String& band, float db) const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
        return juce::String::fromUTF8("BOOST em ") + band + ": " + juce::String(db, 1) 
             + juce::String::fromUTF8("dB acima do target. Considere reduzir ganho ou aplicar filtro notch.");
    return "BOOST in " + band + ": " + juce::String(db, 1) 
         + "dB above target. Consider reducing gain or applying notch filter.";
}

juce::String LocalizedStrings::getTFIssueCut(const juce::String& band, float db) const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
        return juce::String::fromUTF8("CUT em ") + band + ": " + juce::String(std::abs(db), 1) 
             + juce::String::fromUTF8("dB abaixo do target. Considere aumentar ganho ou aplicar filtro shelving.");
    return "CUT in " + band + ": " + juce::String(std::abs(db), 1) 
         + "dB below target. Consider increasing gain or applying shelving filter.";
}

juce::String LocalizedStrings::getTFIssueVariation(const juce::String& band, float range) const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
        return juce::String::fromUTF8("VARIAÇÃO EXCESSIVA em ") + band + ": " + juce::String(range, 1) 
             + juce::String::fromUTF8("dB de range. Considere equalização paramétrica para suavizar.");
    return "EXCESSIVE VARIATION in " + band + ": " + juce::String(range, 1) 
         + "dB range. Consider parametric equalization to smooth.";
}

juce::String LocalizedStrings::getTFIssuePhaseVariation(float range) const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
        return juce::String::fromUTF8("VARIAÇÃO DE FASE EXCESSIVA: ") + juce::String(range, 1) 
             + juce::String::fromUTF8("° de range. Pode indicar problemas de alinhamento ou acústica.");
    return "EXCESSIVE PHASE VARIATION: " + juce::String(range, 1) 
         + "° range. May indicate alignment or acoustic problems.";
}

juce::String LocalizedStrings::getTFIssuePhaseWraps() const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
        return juce::String::fromUTF8("MÚLTIPLOS WRAPS DE FASE detectados. Considere unwrap de fase ou verificar delay compensation.");
    return "MULTIPLE PHASE WRAPS detected. Consider phase unwrap or verify delay compensation.";
}

juce::String LocalizedStrings::getTFIssuePhaseCritical(const juce::String& freqRange) const
{
    GET_LANGUAGE_SAFE()
    if (lang == Language::Portuguese_BR)
        return juce::String::fromUTF8("Fase crítica em ") + freqRange 
             + juce::String::fromUTF8(". Verifique alinhamento de drivers e acústica.");
    return "Critical phase in " + freqRange + ". Verify driver alignment and acoustics.";
}
