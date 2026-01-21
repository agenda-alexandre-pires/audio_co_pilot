#pragma once

namespace AudioCoPilot
{
    namespace DesignSystem
    {
        /**
         * @brief Sistema de espaçamento consistente
         * 
         * Baseado em grid de 4px para consistência visual.
         */
        namespace Spacing
        {
            // Base unit
            constexpr int Unit = 4;
            
            // Espaçamentos padrão
            constexpr int XXS   = Unit * 1;   // 4px
            constexpr int XS    = Unit * 2;   // 8px
            constexpr int S     = Unit * 3;   // 12px
            constexpr int M     = Unit * 4;   // 16px
            constexpr int L     = Unit * 5;   // 20px
            constexpr int XL    = Unit * 6;   // 24px
            constexpr int XXL   = Unit * 8;   // 32px
            constexpr int XXXL  = Unit * 12;  // 48px
            
            // Margens de componentes
            constexpr int ComponentPadding = M;
            constexpr int ComponentMargin = S;
            constexpr int SectionGap = XL;
            
            // Border radius
            constexpr float RadiusSmall = 4.0f;
            constexpr float RadiusMedium = 6.0f;
            constexpr float RadiusLarge = 8.0f;
            constexpr float RadiusXLarge = 12.0f;
            
            // Alturas padrão de componentes
            constexpr int ButtonHeight = 32;
            constexpr int InputHeight = 36;
            constexpr int HeaderHeight = 52;
            constexpr int FooterHeight = 32;
            constexpr int ToolbarHeight = 44;
            constexpr int TabHeight = 40;
        }
        
        /**
         * @brief Sistema de sombras e elevação
         */
        namespace Elevation
        {
            // Níveis de elevação (para sombras e z-index conceitual)
            constexpr int Level0 = 0;   // Superfície base
            constexpr int Level1 = 1;   // Cards, painéis
            constexpr int Level2 = 2;   // Dropdowns
            constexpr int Level3 = 3;   // Modais
            constexpr int Level4 = 4;   // Tooltips
            
            // Blur radius para sombras
            constexpr float ShadowBlur1 = 4.0f;
            constexpr float ShadowBlur2 = 8.0f;
            constexpr float ShadowBlur3 = 16.0f;
            constexpr float ShadowBlur4 = 24.0f;
        }
    }
}
