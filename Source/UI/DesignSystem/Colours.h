#pragma once

#include "../../JuceHeader.h"

namespace AudioCoPilot
{
    namespace DesignSystem
    {
        /**
         * @brief Design System - Paleta de cores Industrial Brutalist
         * 
         * Inspirado em consoles SSL, interfaces Smaart, e equipamentos profissionais.
         * Regras fundamentais:
         * - NUNCA use preto puro (#000000) - causa fadiga visual
         * - NUNCA use branco puro (#FFFFFF) - muito contraste
         * - Cores de destaque devem ser DESSATURADAS em 20-30%
         * - Gradientes sutis criam profundidade sem distrair
         */
        namespace Colours
        {
            // ============================================
            // SUPERFÍCIES E BACKGROUNDS
            // ============================================
            
            namespace Surface
            {
                // Background principal - cinza escuro com leve tom azulado
                constexpr uint32_t Background      = 0xFF0D0E10;  // Quase preto, tom frio
                constexpr uint32_t BackgroundAlt   = 0xFF121416;  // Painéis elevados
                constexpr uint32_t Panel           = 0xFF1A1C20;  // Painéis e cards
                constexpr uint32_t PanelHover      = 0xFF22252A;  // Hover state
                constexpr uint32_t PanelPressed    = 0xFF0A0B0D;  // Pressed state
                constexpr uint32_t Elevated        = 0xFF252830;  // Elementos elevados (dropdowns)
                constexpr uint32_t Overlay         = 0xE6121416;  // Overlays com transparência
                
                // Slots e áreas rebaixadas
                constexpr uint32_t Inset           = 0xFF080909;  // Áreas "afundadas"
                constexpr uint32_t InsetBorder     = 0xFF1A1C1E;  // Borda de áreas afundadas
                
                // Meter backgrounds
                constexpr uint32_t MeterBackground = 0xFF050606;  // Fundo dos meters
                constexpr uint32_t MeterInset      = 0xFF0A0B0C;  // Borda interna meters
            }
            
            // ============================================
            // TEXTO E TIPOGRAFIA
            // ============================================
            
            namespace Text
            {
                constexpr uint32_t Primary         = 0xFFE8EAED;  // Texto principal (não branco puro)
                constexpr uint32_t Secondary       = 0xFF9AA0A6;  // Texto secundário
                constexpr uint32_t Tertiary        = 0xFF5F6368;  // Labels discretos
                constexpr uint32_t Disabled        = 0xFF3C4043;  // Texto desabilitado
                constexpr uint32_t Inverse         = 0xFF121416;  // Texto em fundos claros
                
                // Texto em destaque
                constexpr uint32_t Accent          = 0xFF00E676;  // Texto de destaque verde
                constexpr uint32_t Link            = 0xFF82B1FF;  // Links
                constexpr uint32_t LinkHover       = 0xFFB388FF;  // Links hover
            }
            
            // ============================================
            // CORES DE STATUS E FEEDBACK
            // ============================================
            
            namespace Status
            {
                // Verde - OK / Seguro / Ativo
                constexpr uint32_t Safe            = 0xFF00E676;  // Verde vibrante
                constexpr uint32_t SafeDim         = 0xFF00A853;  // Verde escuro
                constexpr uint32_t SafeGlow        = 0x3300E676;  // Glow verde (20% alpha)
                
                // Âmbar - Atenção / Warning
                constexpr uint32_t Warning         = 0xFFFFAB00;  // Âmbar
                constexpr uint32_t WarningDim      = 0xFFFF8F00;  // Âmbar escuro
                constexpr uint32_t WarningGlow     = 0x33FFAB00;  // Glow âmbar
                
                // Vermelho - Perigo / Erro / Clip
                constexpr uint32_t Alert           = 0xFFFF1744;  // Vermelho
                constexpr uint32_t AlertDim        = 0xFFD50000;  // Vermelho escuro
                constexpr uint32_t AlertGlow       = 0x33FF1744;  // Glow vermelho
                
                // Azul - Info / Seleção
                constexpr uint32_t Info            = 0xFF448AFF;  // Azul info
                constexpr uint32_t InfoDim         = 0xFF2962FF;  // Azul escuro
                constexpr uint32_t Selection       = 0xFF82B1FF;  // Seleção
            }
            
            // ============================================
            // CORES DE METERING (Estilo SSL/Neve)
            // ============================================
            
            namespace Meter
            {
                // Gradiente do meter: Verde -> Amarelo -> Âmbar -> Vermelho
                constexpr uint32_t Green           = 0xFF00C853;  // -∞ to -18 dB
                constexpr uint32_t GreenBright     = 0xFF00E676;  // LED aceso
                constexpr uint32_t Yellow          = 0xFFFFEA00;  // -18 to -12 dB
                constexpr uint32_t YellowBright    = 0xFFFFFF00;  // LED aceso
                constexpr uint32_t Amber           = 0xFFFF9100;  // -12 to -6 dB
                constexpr uint32_t AmberBright     = 0xFFFFAB00;  // LED aceso
                constexpr uint32_t Orange          = 0xFFFF6D00;  // -6 to -3 dB
                constexpr uint32_t OrangeBright    = 0xFFFF9100;  // LED aceso
                constexpr uint32_t Red             = 0xFFFF1744;  // -3 to 0 dB
                constexpr uint32_t RedBright       = 0xFFFF5252;  // LED aceso
                constexpr uint32_t Clip            = 0xFFFF0000;  // Clip indicator
                constexpr uint32_t ClipGlow        = 0xFFFF4444;  // Glow do clip
                
                // Peak hold
                constexpr uint32_t PeakHold        = 0xFFFFFFFF;  // Linha de peak
                constexpr uint32_t PeakHoldDim     = 0xAAFFFFFF;  // Peak hold atenuado
                
                // LED off state
                constexpr uint32_t LEDOff          = 0xFF1A1A1A;  // LED apagado
                constexpr uint32_t LEDOffBorder    = 0xFF2A2A2A;  // Borda LED apagado
            }
            
            // ============================================
            // BORDAS E SEPARADORES
            // ============================================
            
            namespace Border
            {
                constexpr uint32_t Default         = 0xFF2D3036;  // Borda padrão
                constexpr uint32_t Light           = 0xFF3C4043;  // Borda clara
                constexpr uint32_t Dark            = 0xFF1A1C1E;  // Borda escura
                constexpr uint32_t Focus           = 0xFF448AFF;  // Borda de foco
                constexpr uint32_t Active          = 0xFF00E676;  // Borda ativa
                constexpr uint32_t Error           = 0xFFFF1744;  // Borda de erro
                
                // Separadores
                constexpr uint32_t Separator       = 0xFF1F2125;  // Linhas divisórias
                constexpr uint32_t SeparatorLight  = 0xFF2D3036;  // Linhas mais visíveis
            }
            
            // ============================================
            // CORES DE CANAIS (Para identificação visual)
            // ============================================
            
            namespace Channel
            {
                // Cores para identificar diferentes canais/fontes
                constexpr uint32_t Target          = 0xFF00E676;  // Verde - Target
                constexpr uint32_t Masker1         = 0xFF448AFF;  // Azul - Masker 1
                constexpr uint32_t Masker2         = 0xFFE040FB;  // Magenta - Masker 2
                constexpr uint32_t Masker3         = 0xFFFFAB00;  // Âmbar - Masker 3
                constexpr uint32_t Reference       = 0xFF00BCD4;  // Ciano - Reference
                constexpr uint32_t Measurement     = 0xFFFF5722;  // Laranja - Measurement
                
                // Versões dim (para fundos)
                constexpr uint32_t TargetDim       = 0x3300E676;
                constexpr uint32_t Masker1Dim      = 0x33448AFF;
                constexpr uint32_t Masker2Dim      = 0x33E040FB;
                constexpr uint32_t Masker3Dim      = 0x33FFAB00;
            }
            
            // ============================================
            // EFEITOS ESPECIAIS
            // ============================================
            
            namespace Effects
            {
                // Sombras (para usar com setColour e alpha)
                constexpr uint32_t ShadowDark      = 0x66000000;  // Sombra forte
                constexpr uint32_t ShadowMedium    = 0x33000000;  // Sombra média
                constexpr uint32_t ShadowLight     = 0x1A000000;  // Sombra leve
                
                // Glows (brilhos)
                constexpr uint32_t GlowGreen       = 0x4D00E676;  // Glow verde
                constexpr uint32_t GlowBlue        = 0x4D448AFF;  // Glow azul
                constexpr uint32_t GlowRed         = 0x4DFF1744;  // Glow vermelho
                
                // Highlights
                constexpr uint32_t Highlight       = 0x1AFFFFFF;  // Highlight sutil
                constexpr uint32_t HighlightStrong = 0x33FFFFFF;  // Highlight forte
            }
            
            // ============================================
            // HELPERS
            // ============================================
            
            inline juce::Colour getColour(uint32_t hex)
            {
                return juce::Colour(hex);
            }
            
            inline juce::Colour withAlpha(uint32_t hex, float alpha)
            {
                return juce::Colour(hex).withAlpha(alpha);
            }
        }
    }
}
