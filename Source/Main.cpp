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
        // Configure file logger for Release builds (Parte 2)
        juce::File logDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                            .getChildFile("Library")
                            .getChildFile("Logs");
        logDir.createDirectory();
        
        juce::File logFile = logDir.getChildFile("AudioCoPilot.log");
        juce::Logger::setCurrentLogger(new juce::FileLogger(logFile, "AudioCoPilot", 0));
        
        juce::Logger::writeToLog("=== Audio Co-Pilot Started ===");
        juce::Logger::writeToLog("Version: " + getApplicationVersion());
        
        // Force Dock icon at runtime (macOS cache can show generic icon otherwise)
        AudioCoPilot::setMacDockIconFromPngInBundle();
        
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }
    
    void shutdown() override
    {
        juce::Logger::writeToLog("=== Audio Co-Pilot Shutting Down ===");
        juce::Logger::setCurrentLogger(nullptr);
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
