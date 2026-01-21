#include "UI/MainWindow.h"
#include <iostream>

namespace AudioCoPilot { void setMacDockIconFromPngInBundle() noexcept; }

class AudioCoPilotApplication : public juce::JUCEApplication
{
public:
    AudioCoPilotApplication() = default;
    
    const juce::String getApplicationName() override { return "Audio Co-Pilot"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return false; }
    
    void initialise(const juce::String& commandLine) override
    {
        // No file logging - logs only to console if needed
        // Force Dock icon at runtime (macOS cache can show generic icon otherwise)
        AudioCoPilot::setMacDockIconFromPngInBundle();
        
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }
    
    void shutdown() override
    {
        mainWindow = nullptr;
    }
    
    void systemRequestedQuit() override
    {
        quit();
    }
    
    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        // Ignore
    }
    
private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(AudioCoPilotApplication)
