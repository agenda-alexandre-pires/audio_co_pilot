#include "TFKnowledgeBase.h"
#include "../../Localization/LocalizedStrings.h"

TFKnowledgeBase::TFKnowledgeBase()
{
    // Constructor is called from main thread, safe to initialize without lock
    // But we'll use lock anyway for consistency
    juce::ScopedLock sl(lock);
    initializeKnowledgeBase();
}

void TFKnowledgeBase::reinitializeForLanguage()
{
    juce::ScopedLock sl(lock);
    knowledgeMap.clear();
    initializeKnowledgeBase();
}

TFKnowledgeBase::~TFKnowledgeBase()
{
}

void TFKnowledgeBase::initializeKnowledgeBase()
{
    // Note: This is called with lock already held from reinitializeForLanguage() or constructor
    auto& strings = LocalizedStrings::getInstance();
    bool isPT = strings.getCurrentLanguage() == LocalizedStrings::Language::Portuguese_BR;
    
    // Sugestões por faixa de frequência
    // Sub-bass (20-100Hz)
    knowledgeMap["subbass_boost"] = {
        {"Rumble", "20-100Hz", 
         isPT ? juce::String::fromUTF8("Excesso de energia em sub-bass") : "Excess energy in sub-bass",
         isPT ? juce::String::fromUTF8("Pode causar boominess e mascarar frequências médias. Verifique posicionamento de subwoofers.") 
              : "May cause boominess and mask mid frequencies. Check subwoofer positioning.",
         isPT ? juce::String::fromUTF8("Aplicar high-pass filter em 20-30Hz. Reduzir ganho em 40-80Hz com EQ paramétrico.")
              : "Apply high-pass filter at 20-30Hz. Reduce gain at 40-80Hz with parametric EQ.",
         4}
    };
    
    knowledgeMap["subbass_cut"] = {
        {"Rumble", "20-100Hz",
         isPT ? juce::String::fromUTF8("Falta de energia em sub-bass") : "Lack of energy in sub-bass",
         isPT ? juce::String::fromUTF8("Sistema pode soar fino ou sem peso. Verifique resposta de subwoofers.")
              : "System may sound thin or lack weight. Check subwoofer response.",
         isPT ? juce::String::fromUTF8("Aumentar ganho em 40-60Hz. Verificar fase de subwoofers. Considerar adicionar subwoofers.")
              : "Increase gain at 40-60Hz. Check subwoofer phase. Consider adding subwoofers.",
         3}
    };
    
    // Bass (100-250Hz)
    knowledgeMap["bass_boost"] = {
        {"Boominess", "100-250Hz",
         isPT ? juce::String::fromUTF8("Excesso de energia em bass") : "Excess energy in bass",
         isPT ? juce::String::fromUTF8("Causa som 'boomy' e reduz clareza. Pode indicar problemas de acústica ou equalização.")
              : "Causes 'boomy' sound and reduces clarity. May indicate acoustic or equalization problems.",
         isPT ? juce::String::fromUTF8("Aplicar notch filter ou EQ paramétrico em 100-200Hz. Verificar acústica da sala (modos normais).")
              : "Apply notch filter or parametric EQ at 100-200Hz. Check room acoustics (room modes).",
         5}
    };
    
    knowledgeMap["bass_cut"] = {
        {"Boominess", "100-250Hz",
         isPT ? juce::String::fromUTF8("Falta de energia em bass") : "Lack of energy in bass",
         isPT ? juce::String::fromUTF8("Sistema pode soar fino. Verifique resposta de woofers.")
              : "System may sound thin. Check woofer response.",
         isPT ? juce::String::fromUTF8("Aumentar ganho em 150-200Hz. Verificar fase entre drivers.")
              : "Increase gain at 150-200Hz. Check phase between drivers.",
         3}
    };
    
    // Low-mid (250-500Hz)
    knowledgeMap["lowmid_boost"] = {
        {"Mud", "250-500Hz",
         isPT ? juce::String::fromUTF8("Excesso em low-mid") : "Excess in low-mid",
         isPT ? juce::String::fromUTF8("Causa som 'muddy' e reduz clareza. Pode mascarar frequências médias importantes.")
              : "Causes 'muddy' sound and reduces clarity. May mask important mid frequencies.",
         isPT ? juce::String::fromUTF8("Aplicar EQ paramétrico com Q médio (2-4) em 300-400Hz. Reduzir ganho em 3-6dB.")
              : "Apply parametric EQ with medium Q (2-4) at 300-400Hz. Reduce gain by 3-6dB.",
         4}
    };
    
    knowledgeMap["lowmid_cut"] = {
        {"Mud", "250-500Hz",
         isPT ? juce::String::fromUTF8("Falta em low-mid") : "Lack in low-mid",
         isPT ? juce::String::fromUTF8("Sistema pode soar fino ou sem corpo.")
              : "System may sound thin or lack body.",
         isPT ? juce::String::fromUTF8("Aumentar ganho em 300-400Hz. Verificar resposta de mid-woofers.")
              : "Increase gain at 300-400Hz. Check mid-woofer response.",
         3}
    };
    
    // Mid (500-2kHz)
    knowledgeMap["mid_boost"] = {
        {"Harshness", "500-2kHz",
         isPT ? juce::String::fromUTF8("Excesso em mid-range") : "Excess in mid-range",
         isPT ? juce::String::fromUTF8("Pode causar fadiga auditiva e som 'nasal'. Afeta clareza vocal.")
              : "May cause auditory fatigue and 'nasal' sound. Affects vocal clarity.",
         isPT ? juce::String::fromUTF8("Aplicar EQ paramétrico com Q médio-alto (3-6) em 800-1500Hz. Reduzir ganho progressivamente.")
              : "Apply parametric EQ with medium-high Q (3-6) at 800-1500Hz. Reduce gain progressively.",
         4}
    };
    
    knowledgeMap["mid_cut"] = {
        {"Harshness", "500-2kHz",
         isPT ? juce::String::fromUTF8("Falta em mid-range") : "Lack in mid-range",
         isPT ? juce::String::fromUTF8("Sistema pode soar distante ou sem presença. Afeta clareza vocal.")
              : "System may sound distant or lack presence. Affects vocal clarity.",
         isPT ? juce::String::fromUTF8("Aumentar ganho em 1-2kHz. Verificar resposta de mid-drivers.")
              : "Increase gain at 1-2kHz. Check mid-driver response.",
         4}
    };
    
    // High-mid (2-5kHz)
    knowledgeMap["highmid_boost"] = {
        {"Harshness", "2-5kHz",
         isPT ? juce::String::fromUTF8("Excesso em high-mid") : "Excess in high-mid",
         isPT ? juce::String::fromUTF8("Causa som 'brilhante' demais e pode indicar feedback potencial. Área crítica para feedback.")
              : "Causes overly 'bright' sound and may indicate potential feedback. Critical area for feedback.",
         isPT ? juce::String::fromUTF8("Aplicar notch filter ou EQ paramétrico com Q alto (4-8) em 2.5-4kHz. Reduzir ganho em 3-6dB.")
              : "Apply notch filter or parametric EQ with high Q (4-8) at 2.5-4kHz. Reduce gain by 3-6dB.",
         5}
    };
    
    knowledgeMap["highmid_cut"] = {
        {"Harshness", "2-5kHz",
         isPT ? juce::String::fromUTF8("Falta em high-mid") : "Lack in high-mid",
         isPT ? juce::String::fromUTF8("Sistema pode soar abafado. Afeta definição e presença.")
              : "System may sound muffled. Affects definition and presence.",
         isPT ? juce::String::fromUTF8("Aumentar ganho em 3-4kHz. Verificar resposta de tweeters.")
              : "Increase gain at 3-4kHz. Check tweeter response.",
         4}
    };
    
    // High (5-10kHz)
    knowledgeMap["high_boost"] = {
        {"Sibilance", "5-10kHz",
         isPT ? juce::String::fromUTF8("Excesso em high frequencies") : "Excess in high frequencies",
         isPT ? juce::String::fromUTF8("Pode causar sibilância excessiva e fadiga auditiva.")
              : "May cause excessive sibilance and auditory fatigue.",
         isPT ? juce::String::fromUTF8("Aplicar shelving filter ou EQ paramétrico em 6-8kHz. Reduzir ganho suavemente.")
              : "Apply shelving filter or parametric EQ at 6-8kHz. Reduce gain gently.",
         3}
    };
    
    knowledgeMap["high_cut"] = {
        {"Sibilance", "5-10kHz",
         isPT ? juce::String::fromUTF8("Falta em high frequencies") : "Lack in high frequencies",
         isPT ? juce::String::fromUTF8("Sistema pode soar abafado ou sem brilho.")
              : "System may sound muffled or lack brightness.",
         isPT ? juce::String::fromUTF8("Aumentar ganho em 8-10kHz. Verificar resposta de tweeters.")
              : "Increase gain at 8-10kHz. Check tweeter response.",
         3}
    };
    
    // Very High (10-20kHz)
    knowledgeMap["veryhigh_boost"] = {
        {"Air", "10-20kHz",
         isPT ? juce::String::fromUTF8("Excesso em very high frequencies") : "Excess in very high frequencies",
         isPT ? juce::String::fromUTF8("Pode causar fadiga auditiva. Muitas vezes não perceptível mas pode afetar harmônicos.")
              : "May cause auditory fatigue. Often not noticeable but may affect harmonics.",
         isPT ? juce::String::fromUTF8("Aplicar high-shelf filter em 12kHz. Reduzir ganho suavemente.")
              : "Apply high-shelf filter at 12kHz. Reduce gain gently.",
         2}
    };
    
    knowledgeMap["veryhigh_cut"] = {
        {"Air", "10-20kHz",
         isPT ? juce::String::fromUTF8("Falta em very high frequencies") : "Lack in very high frequencies",
         isPT ? juce::String::fromUTF8("Sistema pode soar sem 'ar' ou brilho natural.")
              : "System may sound without 'air' or natural brightness.",
         isPT ? juce::String::fromUTF8("Aumentar ganho em 12-15kHz. Verificar resposta de super-tweeters se disponíveis.")
              : "Increase gain at 12-15kHz. Check super-tweeter response if available.",
         2}
    };
    
    // Sugestões de fase
    knowledgeMap["phase_linearity"] = {
        {"Phase", "Full Range",
         isPT ? juce::String::fromUTF8("Variação excessiva de fase") : "Excessive phase variation",
         isPT ? juce::String::fromUTF8("Fase não-linear pode causar cancelamento e afetar resposta temporal.")
              : "Non-linear phase may cause cancellation and affect temporal response.",
         isPT ? juce::String::fromUTF8("Verificar alinhamento de drivers. Considerar delay compensation. Verificar acústica da sala.")
              : "Check driver alignment. Consider delay compensation. Check room acoustics.",
         4}
    };
    
    knowledgeMap["phase_wraps"] = {
        {"Phase", "Full Range",
         isPT ? juce::String::fromUTF8("Múltiplos wraps de fase") : "Multiple phase wraps",
         isPT ? juce::String::fromUTF8("Indica problemas de delay ou alinhamento entre referência e medição.")
              : "Indicates delay or alignment problems between reference and measurement.",
         isPT ? juce::String::fromUTF8("Aplicar delay compensation. Verificar sincronização entre canais. Usar phase unwrap.")
              : "Apply delay compensation. Check synchronization between channels. Use phase unwrap.",
         5}
    };
}

std::vector<TFKnowledgeBase::Suggestion> TFKnowledgeBase::getSuggestions(float frequency,
                                                                         float magnitudeDeviation,
                                                                         float phaseDeviation,
                                                                         const juce::String& context)
{
    std::vector<Suggestion> suggestions;
    
    juce::ScopedLock sl(lock);
    
    // Determinar banda de frequência
    juce::String bandKey;
    if (frequency >= 20.0f && frequency < 100.0f)
    {
        bandKey = "subbass";
    }
    else if (frequency >= 100.0f && frequency < 250.0f)
    {
        bandKey = "bass";
    }
    else if (frequency >= 250.0f && frequency < 500.0f)
    {
        bandKey = "lowmid";
    }
    else if (frequency >= 500.0f && frequency < 2000.0f)
    {
        bandKey = "mid";
    }
    else if (frequency >= 2000.0f && frequency < 5000.0f)
    {
        bandKey = "highmid";
    }
    else if (frequency >= 5000.0f && frequency < 10000.0f)
    {
        bandKey = "high";
    }
    else if (frequency >= 10000.0f)
    {
        bandKey = "veryhigh";
    }
    
    // Sugestões de magnitude
    if (std::abs(magnitudeDeviation) > 3.0f)
    {
        juce::String key = bandKey + (magnitudeDeviation > 0 ? "_boost" : "_cut");
        auto it = knowledgeMap.find(key);
        if (it != knowledgeMap.end())
        {
            for (const auto& sug : it->second)
            {
                Suggestion s = sug;
                s.frequencyRange = juce::String(frequency, 0) + "Hz";
                suggestions.push_back(s);
            }
        }
    }
    
    // Sugestões de fase
    if (std::abs(phaseDeviation) > 45.0f)
    {
        auto it = knowledgeMap.find("phase_linearity");
        if (it != knowledgeMap.end())
        {
            for (const auto& sug : it->second)
            {
                suggestions.push_back(sug);
            }
        }
    }
    
    return suggestions;
}

std::vector<TFKnowledgeBase::Suggestion> TFKnowledgeBase::getBandSuggestions(const juce::String& bandName,
                                                                             float avgDeviation,
                                                                             float range)
{
    std::vector<Suggestion> suggestions;
    auto& strings = LocalizedStrings::getInstance();
    
    juce::ScopedLock sl(lock);
    
    // Normalizar nome da banda
    juce::String normalizedBand = bandName.toLowerCase();
    normalizedBand = normalizedBand.removeCharacters(" ()-");
    
    juce::String key;
    if (normalizedBand.contains("sub") || normalizedBand.contains("20-100"))
    {
        key = "subbass";
    }
    else if (normalizedBand.contains("bass") || normalizedBand.contains("100-250"))
    {
        key = "bass";
    }
    else if (normalizedBand.contains("low-mid") || normalizedBand.contains("250-500"))
    {
        key = "lowmid";
    }
    else if (normalizedBand.contains("mid") && !normalizedBand.contains("high"))
    {
        key = "mid";
    }
    else if (normalizedBand.contains("high-mid") || normalizedBand.contains("2-5"))
    {
        key = "highmid";
    }
    else if (normalizedBand.contains("high") && !normalizedBand.contains("very"))
    {
        key = "high";
    }
    else if (normalizedBand.contains("very"))
    {
        key = "veryhigh";
    }
    
    if (!key.isEmpty())
    {
        juce::String boostCutKey = key + (avgDeviation > 0 ? "_boost" : "_cut");
        auto it = knowledgeMap.find(boostCutKey);
        if (it != knowledgeMap.end())
        {
            // Localize suggestions on the fly
            for (const auto& sug : it->second)
            {
                Suggestion localizedSug = sug;
                // Re-localize strings based on current language
                // (The knowledge base will be rebuilt with localized strings)
                suggestions.push_back(localizedSug);
            }
        }
        
        // Se range for muito grande, adicionar sugestão de variação
        if (range > 12.0f)
        {
            Suggestion varSug;
            varSug.category = strings.getCurrentLanguage() == LocalizedStrings::Language::Portuguese_BR 
                ? juce::String::fromUTF8("Variação") : "Variation";
            varSug.frequencyRange = bandName;
            varSug.issue = strings.getCurrentLanguage() == LocalizedStrings::Language::Portuguese_BR
                ? juce::String::fromUTF8("Variação excessiva dentro da banda")
                : "Excessive variation within band";
            varSug.recommendation = strings.getCurrentLanguage() == LocalizedStrings::Language::Portuguese_BR
                ? juce::String::fromUTF8("Resposta muito irregular nesta banda. Pode indicar problemas de acústica ou múltiplos drivers desalinhados.")
                : "Very irregular response in this band. May indicate acoustic problems or misaligned drivers.";
            varSug.action = strings.getCurrentLanguage() == LocalizedStrings::Language::Portuguese_BR
                ? juce::String::fromUTF8("Aplicar equalização paramétrica com múltiplos filtros. Verificar acústica da sala. Considerar alinhamento de drivers.")
                : "Apply parametric equalization with multiple filters. Check room acoustics. Consider driver alignment.";
            varSug.priority = 4;
            suggestions.push_back(varSug);
        }
    }
    
    return suggestions;
}
