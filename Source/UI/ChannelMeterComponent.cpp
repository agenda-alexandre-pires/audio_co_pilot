#include "ChannelMeterComponent.h"
#include <cmath>

ChannelMeterComponent::ChannelMeterComponent(DeviceStateModel& stateModel, bool isInput)
    : stateModel(stateModel)
    , isInputChannel(isInput)
{
    using namespace AudioCoPilot::DesignSystem;
    
    // Initialize for up to 64 channels
    meterStates.resize(64);
    labelFont = Typography::labelMedium();
    
    // Get initial channel count
    numChannels = isInputChannel 
        ? stateModel.getNumInputChannels() 
        : stateModel.getNumOutputChannels();
    
    // Force initial update
    if (numChannels > 0)
    {
        repaint();
    }
    
    stateModel.addChangeListener(this);
    // Timer will be started/stopped based on device availability
    updateTimerState();
}

ChannelMeterComponent::~ChannelMeterComponent()
{
    stopTimer();
    stateModel.removeChangeListener(this);
}

void ChannelMeterComponent::paint(juce::Graphics& g)
{
    using namespace AudioCoPilot::DesignSystem;
    
    auto bounds = getLocalBounds();
    
    // Background
    g.setColour(Colours::getColour(Colours::Surface::Panel));
    g.fillRect(bounds);
    
    // Title (centered)
    g.setColour(Colours::getColour(Colours::Text::Primary));
    g.setFont(Typography::labelLarge());
    auto& strings = LocalizedStrings::getInstance();
    juce::String title = isInputChannel ? strings.getInputChannels() : strings.getOutputChannels();
    const int titleHeight = 20;
    g.drawText(title, bounds.removeFromTop(titleHeight), juce::Justification::centred);
    
    // Calculate channels to draw (up to 64)
    // Always check stateModel directly to get latest count
    int currentChannelCount = isInputChannel 
        ? stateModel.getNumInputChannels() 
        : stateModel.getNumOutputChannels();
    
    int channelsToDraw = (currentChannelCount > 0) ? juce::jmin(currentChannelCount, 64) : 0;
    
    // Update numChannels if it changed
    if (currentChannelCount != numChannels)
    {
        numChannels = currentChannelCount;
    }
    
    if (channelsToDraw == 0)
    {
        // Show placeholder message
        g.setColour(Colours::getColour(Colours::Text::Tertiary));
        g.setFont(Typography::bodySmall());
        g.drawText("Select a device to see meters", bounds, juce::Justification::centred);
        return;
    }
    
    // GRID LAYOUT: Vertical meters in grid (max 16 per row)
    int currentY = titleHeight + 5;  // Start below title
    
    // Calculate number of rows needed (max 16 meters per row)
    const int numRows = (channelsToDraw + maxMetersPerRow - 1) / maxMetersPerRow;  // Ceiling division
    
    // Calculate meter cell dimensions
    const int availableWidth = bounds.getWidth() - 20;  // Padding on sides
    const int cellWidth = (availableWidth - (maxMetersPerRow - 1) * horizontalSpacing) / maxMetersPerRow;
    const int meterXOffset = (cellWidth - meterBarWidth) / 2;  // Center meter in cell
    
    for (int row = 0; row < numRows; ++row)
    {
        const int rowY = currentY;
        const int metersInThisRow = juce::jmin(maxMetersPerRow, channelsToDraw - row * maxMetersPerRow);
        
        for (int col = 0; col < metersInThisRow; ++col)
        {
            const int channelIndex = row * maxMetersPerRow + col;
            
            if (channelIndex >= channelsToDraw)
                break;
            
            const int cellX = 10 + col * (cellWidth + horizontalSpacing);
            const int meterX = cellX + meterXOffset;
            
            // Vertical meter bar (from bottom to top)
            const int meterY = rowY;
            
            // Meter background (vertical bar)
            g.setColour(Colours::getColour(Colours::Surface::MeterBackground));
            g.fillRect(meterX, meterY, meterBarWidth, meterBarHeight);
            
            // Meter scale (dB marks) - horizontal lines across the bar
            g.setColour(Colours::getColour(Colours::Text::Tertiary));
            for (float db = -60.0f; db <= 0.0f; db += 20.0f)
            {
                float y = dbToYPosition(db, meterBarHeight);
                g.drawHorizontalLine(meterY + y, meterX, meterX + meterBarWidth);
            }
            
            // RMS level - vertical bar from BOTTOM to TOP
            if (channelIndex < static_cast<int>(meterStates.size()))
            {
                auto& meterState = meterStates[channelIndex];
                float rmsY = dbToYPosition(meterState.rmsDb, meterBarHeight);
                
                // Draw RMS level from bottom (min) to current level (top)
                juce::Rectangle<float> rmsRect(
                    static_cast<float>(meterX),
                    static_cast<float>(meterY + rmsY),  // Start from RMS level
                    static_cast<float>(meterBarWidth),
                    static_cast<float>(meterBarHeight - rmsY)  // Height from RMS to top
                );
                
                // Color gradient: green -> yellow -> orange -> red (bottom to top)
                juce::Colour rmsColour;
                if (meterState.rmsDb > -6.0f)
                    rmsColour = Colours::getColour(Colours::Meter::Red);
                else if (meterState.rmsDb > -18.0f)
                    rmsColour = Colours::getColour(Colours::Meter::Amber);
                else if (meterState.rmsDb > -30.0f)
                    rmsColour = Colours::getColour(Colours::Meter::Yellow);
                else
                    rmsColour = Colours::getColour(Colours::Meter::Green);
                
                g.setColour(rmsColour);
                g.fillRect(rmsRect);
                
                // Peak hold - horizontal line across the bar (white line)
                if (meterState.peakHoldDb > minDb)
                {
                    float peakY = dbToYPosition(meterState.peakHoldDb, meterBarHeight);
                    g.setColour(Colours::getColour(Colours::Meter::PeakHold));
                    g.drawHorizontalLine(meterY + peakY, meterX, meterX + meterBarWidth);
                }
            }
            
            // Channel number label BELOW the meter
            g.setColour(Colours::getColour(Colours::Text::Secondary));
            g.setFont(labelFont);
            juce::String label = juce::String(channelIndex + 1);
            g.drawText(label, cellX, rowY + meterBarHeight + 2, cellWidth, labelHeight, juce::Justification::centred);
        }
        
        // Move to next row
        currentY += meterBarHeight + labelHeight + verticalSpacing;
    }
}

void ChannelMeterComponent::resized()
{
    // Always recalculate height based on current channel count
    int requiredHeight = getRequiredHeight();
    
    // Force resize to match required height
    if (getHeight() != requiredHeight || getWidth() != getParentWidth())
    {
        setSize(getParentWidth() > 0 ? getParentWidth() : getWidth(), requiredHeight);
    }
    repaint();
}

int ChannelMeterComponent::getRequiredWidth() const
{
    // Width uses full available space (viewport width)
    return 200;  // Minimum width, will expand to viewport width
}

int ChannelMeterComponent::getRequiredHeight() const
{
    const int titleHeight = 20;
    const int padding = 10;
    
    // Calculate height based on grid layout (max 16 meters per row)
    int maxChannels = juce::jmin(numChannels, 64);
    if (maxChannels == 0)
        return titleHeight + 50;  // Minimum height for placeholder
    
    // Calculate number of rows needed
    const int numRows = (maxChannels + maxMetersPerRow - 1) / maxMetersPerRow;  // Ceiling division
    
    // Height = title + (numRows * (meterBarHeight + labelHeight + verticalSpacing)) + padding
    return titleHeight + (numRows * (meterBarHeight + labelHeight + verticalSpacing)) + padding;
}

void ChannelMeterComponent::timerCallback()
{
    // Only update if we have channels and device is active
    if (numChannels == 0)
    {
        stopTimer();
        return;
    }
    
    updateMeterLevels();
    
    // Throttle repaint: only repaint if levels actually changed
    static float lastRMS[64] = {0};
    static float lastPeak[64] = {0};
    bool needsRepaint = false;
    
    for (int i = 0; i < numChannels && i < 64; ++i)
    {
        if (i < static_cast<int>(meterStates.size()))
        {
            float currentRMS = meterStates[i].rmsDb;
            float currentPeak = meterStates[i].peakDb;
            
            // Only repaint if change is significant (>0.5dB)
            if (std::abs(currentRMS - lastRMS[i]) > 0.5f || 
                std::abs(currentPeak - lastPeak[i]) > 0.5f)
            {
                needsRepaint = true;
                lastRMS[i] = currentRMS;
                lastPeak[i] = currentPeak;
            }
        }
    }
    
    if (needsRepaint)
        repaint();
}

void ChannelMeterComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &stateModel)
    {
        int newNumChannels = isInputChannel 
            ? stateModel.getNumInputChannels() 
            : stateModel.getNumOutputChannels();
        
        // Always update, even if same count, to ensure meters are visible
        int oldNumChannels = numChannels;
        numChannels = juce::jmin(newNumChannels, 64);  // Cap at 64 channels
        
        // Ensure meterStates array is large enough (up to 64 channels)
        if (64 > static_cast<int>(meterStates.size()))
        {
            meterStates.resize(64);
        }
        
        // CRITICAL: If channel count changed (device switch), reset all meter states
        // This prevents freeze when switching devices
        if (newNumChannels != oldNumChannels)
        {
            for (auto& state : meterStates)
            {
                state.rmsDb = minDb;
                state.peakDb = minDb;
                state.peakHoldDb = minDb;
                state.peakHoldTime = 0.0;
            }
        }
        
        // Update timer state based on channel availability
        updateTimerState();
        
        // CRITICAL: Recalculate and resize component to fit all channels
        int requiredHeight = getRequiredHeight();
        int currentWidth = getWidth();
        if (currentWidth == 0 && getParentComponent() != nullptr)
        {
            currentWidth = getParentComponent()->getWidth() / 2;
        }
        
        // Resize component to fit all channels
        setSize(currentWidth > 0 ? currentWidth : 200, requiredHeight);
        
        // Repaint when channel count changes
        repaint();
        
        // Notify parent to recalculate size
        juce::MessageManager::callAsync([this]()
        {
            if (getParentComponent() != nullptr)
            {
                getParentComponent()->resized();
            }
        });
    }
}

void ChannelMeterComponent::updateMeterLevels()
{
    auto channels = isInputChannel 
        ? stateModel.getInputChannels() 
        : stateModel.getOutputChannels();
    
    double currentTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    
    // Debug: Log first few reads to verify we're getting data
    static int readCount = 0;
    if (readCount < 5 && isInputChannel && numChannels > 0 && channels.size() > 0)
    {
        juce::Logger::writeToLog("Meter read ch0 - RMS: " + juce::String(channels[0].rmsLevel) + 
                                 " Peak: " + juce::String(channels[0].peakLevel) + 
                                 " numChannels: " + juce::String(numChannels));
        readCount++;
    }
    
    // Update meters for all available channels (up to 64)
    int channelsToUpdate = juce::jmin(numChannels, static_cast<int>(channels.size()), 64);
    
    for (int i = 0; i < channelsToUpdate && i < static_cast<int>(meterStates.size()); ++i)
    {
        auto& meterState = meterStates[i];
        auto& channelInfo = channels[i];
        
        float rms = channelInfo.rmsLevel;
        float peak = channelInfo.peakLevel;
        
        meterState.rmsDb = levelToDb(rms);
        meterState.peakDb = levelToDb(peak);
        
        // Peak hold
        if (meterState.peakDb > meterState.peakHoldDb)
        {
            meterState.peakHoldDb = meterState.peakDb;
            meterState.peakHoldTime = currentTime;
        }
        else if (currentTime - meterState.peakHoldTime > meterHoldTime)
        {
            meterState.peakHoldDb = meterState.peakDb;
        }
    }
}

float ChannelMeterComponent::levelToDb(float level) const
{
    if (level <= 0.0f)
        return minDb;
    
    float db = 20.0f * std::log10(level);
    return juce::jlimit(minDb, maxDb, db);
}

float ChannelMeterComponent::dbToYPosition(float db, float height) const
{
    float normalized = (db - minDb) / (maxDb - minDb);
    return height * (1.0f - normalized);
}

float ChannelMeterComponent::dbToXPosition(float db, float width) const
{
    float normalized = (db - minDb) / (maxDb - minDb);
    return width * normalized;  // Left to right: 0dB at right, -60dB at left
}

void ChannelMeterComponent::updateTimerState()
{
    // Only run timer if we have active channels
    if (numChannels > 0)
    {
        if (!isTimerRunning())
            startTimer(30); // ~30 FPS update rate
    }
    else
    {
        if (isTimerRunning())
            stopTimer();
    }
}
