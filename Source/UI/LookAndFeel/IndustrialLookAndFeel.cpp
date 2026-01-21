#include "IndustrialLookAndFeel.h"

namespace AudioCoPilot
{
    using namespace DesignSystem;
    
    IndustrialLookAndFeel::IndustrialLookAndFeel()
    {
        // Cache cores principais
        _bgColour = Colours::getColour(Colours::Surface::Background);
        _textColour = Colours::getColour(Colours::Text::Primary);
        _accentColour = Colours::getColour(Colours::Status::Safe);
        _borderColour = Colours::getColour(Colours::Border::Default);
        
        // ============================================
        // CORES GLOBAIS
        // ============================================
        
        // Window
        setColour(juce::ResizableWindow::backgroundColourId, _bgColour);
        setColour(juce::DocumentWindow::backgroundColourId, _bgColour);
        
        // Labels
        setColour(juce::Label::textColourId, _textColour);
        setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
        
        // Text Button
        setColour(juce::TextButton::buttonColourId, Colours::getColour(Colours::Surface::Panel));
        setColour(juce::TextButton::buttonOnColourId, _accentColour);
        setColour(juce::TextButton::textColourOffId, _textColour);
        setColour(juce::TextButton::textColourOnId, Colours::getColour(Colours::Text::Inverse));
        
        // ComboBox
        setColour(juce::ComboBox::backgroundColourId, Colours::getColour(Colours::Surface::Panel));
        setColour(juce::ComboBox::textColourId, _textColour);
        setColour(juce::ComboBox::outlineColourId, _borderColour);
        setColour(juce::ComboBox::arrowColourId, Colours::getColour(Colours::Text::Secondary));
        setColour(juce::ComboBox::focusedOutlineColourId, Colours::getColour(Colours::Border::Focus));
        
        // PopupMenu
        setColour(juce::PopupMenu::backgroundColourId, Colours::getColour(Colours::Surface::Elevated));
        setColour(juce::PopupMenu::textColourId, _textColour);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, _accentColour);
        setColour(juce::PopupMenu::highlightedTextColourId, Colours::getColour(Colours::Text::Inverse));
        
        // TextEditor
        setColour(juce::TextEditor::backgroundColourId, Colours::getColour(Colours::Surface::Inset));
        setColour(juce::TextEditor::textColourId, _textColour);
        setColour(juce::TextEditor::highlightColourId, Colours::getColour(Colours::Status::Selection));
        setColour(juce::TextEditor::highlightedTextColourId, _textColour);
        setColour(juce::TextEditor::outlineColourId, _borderColour);
        setColour(juce::TextEditor::focusedOutlineColourId, Colours::getColour(Colours::Border::Focus));
        
        // ScrollBar
        setColour(juce::ScrollBar::backgroundColourId, Colours::getColour(Colours::Surface::Inset));
        setColour(juce::ScrollBar::thumbColourId, Colours::getColour(Colours::Surface::Panel));
        setColour(juce::ScrollBar::trackColourId, Colours::getColour(Colours::Surface::Inset));
    }
    
    // ============================================
    // FONTS
    // ============================================
    
    juce::Font IndustrialLookAndFeel::getLabelFont(juce::Label& label)
    {
        juce::ignoreUnused(label);
        return Typography::bodyMedium();
    }
    
    juce::Font IndustrialLookAndFeel::getComboBoxFont(juce::ComboBox& box)
    {
        juce::ignoreUnused(box);
        return Typography::bodyMedium();
    }
    
    juce::Font IndustrialLookAndFeel::getTextButtonFont(juce::TextButton& button, int buttonHeight)
    {
        juce::ignoreUnused(button, buttonHeight);
        return Typography::labelLarge();
    }
    
    juce::Font IndustrialLookAndFeel::getPopupMenuFont()
    {
        return Typography::bodyMedium();
    }
    
    juce::Font IndustrialLookAndFeel::getAlertWindowTitleFont()
    {
        return Typography::headlineMedium();
    }
    
    juce::Font IndustrialLookAndFeel::getAlertWindowMessageFont()
    {
        return Typography::bodyMedium();
    }
    
    juce::Font IndustrialLookAndFeel::getAlertWindowFont()
    {
        return Typography::bodyMedium();
    }
    
    // ============================================
    // BUTTONS
    // ============================================
    
    void IndustrialLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                      juce::Button& button,
                                                      const juce::Colour& backgroundColour,
                                                      bool shouldDrawButtonAsHighlighted,
                                                      bool shouldDrawButtonAsDown)
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        auto cornerRadius = Spacing::RadiusMedium;
        
        // Determina cor base
        juce::Colour baseColour = backgroundColour;
        
        if (shouldDrawButtonAsDown)
        {
            baseColour = baseColour.darker(0.2f);
        }
        else if (shouldDrawButtonAsHighlighted)
        {
            baseColour = baseColour.brighter(0.1f);
        }
        
        // Gradiente sutil de cima para baixo
        juce::ColourGradient gradient(
            baseColour.brighter(0.05f), bounds.getTopLeft(),
            baseColour.darker(0.05f), bounds.getBottomLeft(),
            false);
        
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, cornerRadius);
        
        // Borda interna highlight (topo)
        if (!shouldDrawButtonAsDown)
        {
            g.setColour(Colours::getColour(Colours::Effects::Highlight));
            g.drawHorizontalLine(static_cast<int>(bounds.getY() + 1), 
                                 bounds.getX() + cornerRadius, 
                                 bounds.getRight() - cornerRadius);
        }
        
        // Borda externa
        g.setColour(shouldDrawButtonAsHighlighted ? 
                    Colours::getColour(Colours::Border::Light) : _borderColour);
        g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);
        
        // Glow se é o botão "on"
        if (button.getToggleState())
        {
            drawGlowEffect(g, bounds, _accentColour, 8.0f);
        }
    }
    
    void IndustrialLookAndFeel::drawButtonText(juce::Graphics& g,
                                                juce::TextButton& button,
                                                bool shouldDrawButtonAsHighlighted,
                                                bool shouldDrawButtonAsDown)
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted);
        
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(font);
        
        juce::Colour textColour = button.findColour(button.getToggleState() ?
            juce::TextButton::textColourOnId : juce::TextButton::textColourOffId);
        
        if (shouldDrawButtonAsDown)
            textColour = textColour.darker(0.1f);
        
        g.setColour(textColour);
        
        auto bounds = button.getLocalBounds();
        g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
    }
    
    void IndustrialLookAndFeel::drawToggleButton(juce::Graphics& g,
                                                  juce::ToggleButton& button,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto toggleSize = 20.0f;
        auto toggleBounds = bounds.removeFromLeft(toggleSize).withSizeKeepingCentre(toggleSize, toggleSize);
        
        // Fundo do toggle
        g.setColour(Colours::getColour(Colours::Surface::Inset));
        g.fillRoundedRectangle(toggleBounds, 4.0f);
        
        // Borda
        g.setColour(_borderColour);
        g.drawRoundedRectangle(toggleBounds, 4.0f, 1.0f);
        
        // Check mark se ativo
        if (button.getToggleState())
        {
            auto checkBounds = toggleBounds.reduced(4.0f);
            
            // Fundo verde
            g.setColour(_accentColour);
            g.fillRoundedRectangle(checkBounds, 2.0f);
            
            // Glow
            drawGlowEffect(g, toggleBounds, _accentColour, 6.0f);
        }
        
        // Texto
        g.setColour(shouldDrawButtonAsHighlighted ? _textColour : Colours::getColour(Colours::Text::Secondary));
        g.setFont(Typography::bodyMedium());
        
        auto textBounds = button.getLocalBounds().toFloat();
        textBounds.removeFromLeft(toggleSize + 8.0f);
        
        g.drawText(button.getButtonText(), textBounds.toNearestInt(), juce::Justification::centredLeft);
        
        juce::ignoreUnused(shouldDrawButtonAsDown);
    }
    
    // ============================================
    // COMBOBOX
    // ============================================
    
    void IndustrialLookAndFeel::drawComboBox(juce::Graphics& g,
                                              int width, int height,
                                              bool isButtonDown,
                                              int buttonX, int buttonY,
                                              int buttonW, int buttonH,
                                              juce::ComboBox& box)
    {
        juce::ignoreUnused(buttonX, buttonY, buttonW, buttonH);
        
        auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height));
        auto cornerRadius = Spacing::RadiusMedium;
        
        // Gradiente de fundo
        juce::ColourGradient gradient(
            Colours::getColour(Colours::Surface::Panel).brighter(0.05f), bounds.getTopLeft(),
            Colours::getColour(Colours::Surface::Panel).darker(0.05f), bounds.getBottomLeft(),
            false);
        
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds.reduced(0.5f), cornerRadius);
        
        // Borda
        g.setColour(isButtonDown ? Colours::getColour(Colours::Border::Focus) : _borderColour);
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
        
        // Seta
        auto arrowZone = juce::Rectangle<float>(
            static_cast<float>(width) - 24.0f, 0.0f, 20.0f, static_cast<float>(height));
        
        juce::Path arrow;
        auto arrowCenter = arrowZone.getCentre();
        auto arrowSize = 5.0f;
        
        arrow.addTriangle(
            arrowCenter.x - arrowSize, arrowCenter.y - arrowSize / 2,
            arrowCenter.x + arrowSize, arrowCenter.y - arrowSize / 2,
            arrowCenter.x, arrowCenter.y + arrowSize / 2
        );
        
        g.setColour(box.findColour(juce::ComboBox::arrowColourId));
        g.fillPath(arrow);
    }
    
    void IndustrialLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
    {
        auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height));
        
        // Sombra
        g.setColour(Colours::getColour(Colours::Effects::ShadowDark));
        g.fillRoundedRectangle(bounds.translated(2, 2), Spacing::RadiusMedium);
        
        // Fundo
        g.setColour(Colours::getColour(Colours::Surface::Elevated));
        g.fillRoundedRectangle(bounds, Spacing::RadiusMedium);
        
        // Borda
        g.setColour(_borderColour);
        g.drawRoundedRectangle(bounds.reduced(0.5f), Spacing::RadiusMedium, 1.0f);
    }
    
    // ============================================
    // TEXT EDITOR
    // ============================================
    
    void IndustrialLookAndFeel::fillTextEditorBackground(juce::Graphics& g,
                                                          int width, int height,
                                                          juce::TextEditor& textEditor)
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
        g.setColour(textEditor.findColour(juce::TextEditor::backgroundColourId));
        g.fillRoundedRectangle(bounds, Spacing::RadiusSmall);
    }
    
    void IndustrialLookAndFeel::drawTextEditorOutline(juce::Graphics& g,
                                                       int width, int height,
                                                       juce::TextEditor& textEditor)
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
        
        if (textEditor.hasKeyboardFocus(true))
            g.setColour(textEditor.findColour(juce::TextEditor::focusedOutlineColourId));
        else
            g.setColour(textEditor.findColour(juce::TextEditor::outlineColourId));
        
        g.drawRoundedRectangle(bounds.reduced(0.5f), Spacing::RadiusSmall, 1.0f);
    }
    
    // ============================================
    // SCROLL BAR
    // ============================================
    
    void IndustrialLookAndFeel::drawScrollbar(juce::Graphics& g,
                                               juce::ScrollBar& scrollbar,
                                               int x, int y, int width, int height,
                                               bool isScrollbarVertical,
                                               int thumbStartPosition,
                                               int thumbSize,
                                               bool isMouseOver,
                                               bool isMouseDown)
    {
        juce::ignoreUnused(scrollbar);
        
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        
        // Track
        g.setColour(Colours::getColour(Colours::Surface::Inset));
        g.fillRoundedRectangle(bounds, 3.0f);
        
        // Thumb
        juce::Rectangle<float> thumbBounds;
        if (isScrollbarVertical)
        {
            thumbBounds = bounds.withY(static_cast<float>(thumbStartPosition))
                                 .withHeight(static_cast<float>(thumbSize))
                                 .reduced(2.0f, 0.0f);
        }
        else
        {
            thumbBounds = bounds.withX(static_cast<float>(thumbStartPosition))
                                 .withWidth(static_cast<float>(thumbSize))
                                 .reduced(0.0f, 2.0f);
        }
        
        juce::Colour thumbColour = Colours::getColour(Colours::Surface::Panel);
        if (isMouseDown)
            thumbColour = thumbColour.brighter(0.2f);
        else if (isMouseOver)
            thumbColour = thumbColour.brighter(0.1f);
        
        g.setColour(thumbColour);
        g.fillRoundedRectangle(thumbBounds, 3.0f);
    }
    
    // ============================================
    // HELPERS
    // ============================================
    
    void IndustrialLookAndFeel::drawGlowEffect(juce::Graphics& g,
                                                juce::Rectangle<float> bounds,
                                                juce::Colour glowColour,
                                                float radius)
    {
        // Desenha glow em várias camadas com alpha decrescente
        for (float i = radius; i >= 1.0f; i -= 2.0f)
        {
            float alpha = (1.0f - i / radius) * 0.15f;
            g.setColour(glowColour.withAlpha(alpha));
            g.drawRoundedRectangle(bounds.expanded(i), Spacing::RadiusMedium + i / 2, 2.0f);
        }
    }
}
