#pragma once

#include "../JuceHeader.h"
#include "../Core/DeviceManager.h"
#include "../Localization/LocalizedStrings.h"

/**
 * MenuBarModel
 * 
 * macOS-style menu bar implementation.
 * Provides App, Settings, and Device menus.
 */
class MenuBarModel : public juce::MenuBarModel,
                     public juce::ChangeListener
{
public:
    MenuBarModel(DeviceManager& deviceManager);
    ~MenuBarModel() override;
    
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
    
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // Module activation callback
    std::function<void(int)> moduleActivationCallback;
    void setModuleActivationCallback(std::function<void(int)> callback) { moduleActivationCallback = callback; }
    
private:
    DeviceManager& deviceManager;
    
    enum MenuIDs
    {
        About = 1,
        Quit = 2,
        LanguageEnglish = 10,
        LanguagePortuguese = 11,
        DeviceSelector = 20,
        TransferFunction = 30,
        AntiMasking = 31,
        RTA = 32,
        AIStageHand = 33
    };
    
    void populateDeviceMenu(juce::PopupMenu& menu);
};
