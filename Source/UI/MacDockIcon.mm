#include "../JuceHeader.h"

#if JUCE_MAC
 #import <Cocoa/Cocoa.h>
#endif

namespace AudioCoPilot
{
// Forces the Dock icon at runtime (bypasses Finder/Dock cache issues).
void setMacDockIconFromPngInBundle() noexcept
{
   #if JUCE_MAC
    @autoreleasepool
    {
        auto appFile = juce::File::getSpecialLocation (juce::File::currentApplicationFile);
        juce::File resources;

        if (appFile.isDirectory() && appFile.hasFileExtension (".app"))
            resources = appFile.getChildFile ("Contents").getChildFile ("Resources");
        else
            resources = appFile.getParentDirectory().getParentDirectory().getChildFile ("Resources");

        auto logo = resources.getChildFile ("AudioCoPilotLogo.png");
        if (! logo.existsAsFile())
            return;

        auto img = juce::ImageFileFormat::loadFrom (logo);
        if (! img.isValid())
            return;

        // Convert JUCE image -> PNG bytes
        juce::MemoryOutputStream mos;
        juce::PNGImageFormat png;
        if (! png.writeImageToStream (img, mos))
            return;

        NSData* data = [NSData dataWithBytes: mos.getData() length: (NSUInteger) mos.getDataSize()];
        NSImage* nsImg = [[NSImage alloc] initWithData: data];
        if (nsImg == nil)
            return;

        [NSApp setApplicationIconImage: nsImg];
    }
   #endif
}
}

