# Audio Co-Pilot

Professional audio application for macOS built with JUCE and C++20.

## Phase 1 - Device Selector

This module provides stable, crash-free audio device selection with real-time metering.

## Requirements

- CMake 3.22+
- JUCE 7.0+ (as git submodule)
- macOS 10.15+
- Xcode Command Line Tools

## Setup

1. Clone JUCE as a submodule:
```bash
git submodule add https://github.com/juce-framework/JUCE.git JUCE
git submodule update --init --recursive
```

Or if you already have JUCE cloned:
```bash
ln -s /path/to/JUCE JUCE
```

2. Build the project:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

3. Run the application:
```bash
./AudioCoPilot_artefacts/Release/AudioCoPilot.app/Contents/MacOS/AudioCoPilot
```

## Architecture

### Core Components
- **DeviceManager**: Authoritative device controller, handles safe device switching
- **AudioEngine**: High-level audio engine interface
- **DeviceStateModel**: Thread-safe shared state model

### UI Components
- **MainWindow**: Main application window
- **DeviceSelectorComponent**: Dropdown device selector
- **ChannelMeterComponent**: Real-time RMS/Peak meters

### Features
- Safe, atomic device switching
- Real-time metering (RMS + Peak, dBFS scale)
- Automatic channel discovery
- macOS-style menu bar
- Runtime language switching (English/Portuguese)
