#pragma once

#include "../../JuceHeader.h"

namespace AudioCoPilot
{
    namespace DesignSystem
    {
        /**
         * @brief Sistema de tipografia profissional
         * 
         * Hierarquia clara, legibilidade em todas as situações.
         * Fonte principal: SF Pro (sistema macOS) ou Helvetica Neue
         * Fonte mono: SF Mono ou Menlo (para números/valores)
         */
        namespace Typography
        {
            // ============================================
            // FONTES BASE
            // ============================================
            
            inline juce::String getPrimaryFontName()
            {
                // Tenta SF Pro primeiro (melhor para macOS moderno)
                if (juce::Font::findAllTypefaceNames().contains("SF Pro Display"))
                    return "SF Pro Display";
                return "Helvetica Neue";
            }
            
            inline juce::String getMonoFontName()
            {
                if (juce::Font::findAllTypefaceNames().contains("SF Mono"))
                    return "SF Mono";
                return "Menlo";
            }
            
            // ============================================
            // TAMANHOS E PESOS
            // ============================================
            
            // Display - Títulos grandes
            inline juce::Font displayLarge()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 32.0f, juce::Font::bold));
            }
            
            inline juce::Font displayMedium()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 24.0f, juce::Font::bold));
            }
            
            inline juce::Font displaySmall()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 20.0f, juce::Font::bold));
            }
            
            // Headlines - Seções
            inline juce::Font headlineLarge()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 18.0f, juce::Font::bold));
            }
            
            inline juce::Font headlineMedium()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 16.0f, juce::Font::bold));
            }
            
            inline juce::Font headlineSmall()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 14.0f, juce::Font::bold));
            }
            
            // Body - Texto corrido
            inline juce::Font bodyLarge()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 14.0f, juce::Font::plain));
            }
            
            inline juce::Font bodyMedium()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 13.0f, juce::Font::plain));
            }
            
            inline juce::Font bodySmall()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 12.0f, juce::Font::plain));
            }
            
            // Labels - UI elements
            inline juce::Font labelLarge()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 12.0f, juce::Font::bold));
            }
            
            inline juce::Font labelMedium()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 11.0f, juce::Font::plain));
            }
            
            inline juce::Font labelSmall()
            {
                return juce::Font(juce::FontOptions(getPrimaryFontName(), 10.0f, juce::Font::plain));
            }
            
            // Mono - Valores numéricos
            inline juce::Font monoLarge()
            {
                return juce::Font(juce::FontOptions(getMonoFontName(), 16.0f, juce::Font::plain));
            }
            
            inline juce::Font monoMedium()
            {
                return juce::Font(juce::FontOptions(getMonoFontName(), 13.0f, juce::Font::plain));
            }
            
            inline juce::Font monoSmall()
            {
                return juce::Font(juce::FontOptions(getMonoFontName(), 11.0f, juce::Font::plain));
            }
            
            // Meter values
            inline juce::Font meterValue()
            {
                return juce::Font(juce::FontOptions(getMonoFontName(), 10.0f, juce::Font::bold));
            }
            
            inline juce::Font meterLabel()
            {
                return juce::Font(juce::FontOptions(getMonoFontName(), 9.0f, juce::Font::plain));
            }
        }
    }
}
