#include "TransferFunctionView.h"
#include "PhasePlotComponent.h"
#include "MagnitudePlotComponent.h"
#include "TFAutoSuggestionsComponent.h"
#include "../../Localization/LocalizedStrings.h"

TransferFunctionView::TransferFunctionView(TFController& ctrl)
    : controller(ctrl)
{
    // Channel selectors
    auto& strings = LocalizedStrings::getInstance();
    
    referenceLabel = std::make_unique<juce::Label>("refLabel", strings.getTFReferenceChannel());
    referenceLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(referenceLabel.get());
    
    referenceChannelSelector = std::make_unique<juce::ComboBox>("refChannel");
    referenceChannelSelector->addListener(this);
    addAndMakeVisible(referenceChannelSelector.get());
    
    measurementLabel = std::make_unique<juce::Label>("measLabel", strings.getTFMeasurementChannel());
    measurementLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(measurementLabel.get());
    
    measurementChannelSelector = std::make_unique<juce::ComboBox>("measChannel");
    measurementChannelSelector->addListener(this);
    addAndMakeVisible(measurementChannelSelector.get());
    
    // Plots
    phasePlot = std::make_unique<PhasePlotComponent>(controller.getProcessor());
    addAndMakeVisible(phasePlot.get());
    
    magnitudePlot = std::make_unique<MagnitudePlotComponent>(controller.getProcessor());
    addAndMakeVisible(magnitudePlot.get());
    
    // Auto-suggestions component
    suggestionsComponent = std::make_unique<TFAutoSuggestionsComponent>();
    addAndMakeVisible(suggestionsComponent.get());
    
    // Listen to controller for device changes
    controller.addChangeListener(this);
    
    // Listen to localization changes
    LocalizedStrings::getInstance().addChangeListener(this);
    
    updateChannelSelectors();
    
    // Start timer to update suggestions (reduced frequency for performance and to prevent flickering)
    startTimer(2000);
}

TransferFunctionView::~TransferFunctionView()
{
    stopTimer();
    controller.removeChangeListener(this);
    LocalizedStrings::getInstance().removeChangeListener(this);
}

void TransferFunctionView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void TransferFunctionView::resized()
{
    auto bounds = getLocalBounds();
    
    // Channel selectors at top
    auto selectorArea = bounds.removeFromTop(40);
    const int selectorWidth = 200;
    const int spacing = 10;
    
    referenceLabel->setBounds(selectorArea.removeFromLeft(150).reduced(5));
    referenceChannelSelector->setBounds(selectorArea.removeFromLeft(selectorWidth).reduced(5));
    selectorArea.removeFromLeft(spacing);
    
    measurementLabel->setBounds(selectorArea.removeFromLeft(150).reduced(5));
    measurementChannelSelector->setBounds(selectorArea.removeFromLeft(selectorWidth).reduced(5));
    
    // Split remaining space: Plots (left 60%), Suggestions (right 40%)
    const int plotWidth = static_cast<int>(bounds.getWidth() * 0.6f);
    auto plotArea = bounds.removeFromLeft(plotWidth);
    bounds.removeFromLeft(5);  // Spacing
    
    // Split plot area: Phase (top 50%), Magnitude (bottom 50%)
    const int halfHeight = plotArea.getHeight() / 2;
    phasePlot->setBounds(plotArea.removeFromTop(halfHeight).reduced(5));
    magnitudePlot->setBounds(plotArea.reduced(5));
    
    // Suggestions on the right
    suggestionsComponent->setBounds(bounds.reduced(5));
}

void TransferFunctionView::updateChannelSelectors()
{
    int availableChannels = controller.getAvailableInputChannels();
    
    referenceChannelSelector->clear();
    measurementChannelSelector->clear();
    
    for (int i = 0; i < availableChannels; ++i)
    {
        juce::String channelName = "Channel " + juce::String(i + 1);
        referenceChannelSelector->addItem(channelName, i + 1);
        measurementChannelSelector->addItem(channelName, i + 1);
    }
    
    // Set current selections
    referenceChannelSelector->setSelectedId(controller.getReferenceChannel() + 1, juce::dontSendNotification);
    measurementChannelSelector->setSelectedId(controller.getMeasurementChannel() + 1, juce::dontSendNotification);
}

void TransferFunctionView::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == referenceChannelSelector.get())
    {
        int channel = referenceChannelSelector->getSelectedId() - 1;
        controller.setReferenceChannel(channel);
    }
    else if (comboBoxThatHasChanged == measurementChannelSelector.get())
    {
        int channel = measurementChannelSelector->getSelectedId() - 1;
        controller.setMeasurementChannel(channel);
    }
}

void TransferFunctionView::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &controller)
    {
        // Device changed - update channel selectors
        updateChannelSelectors();
    }
    else if (source == &LocalizedStrings::getInstance())
    {
        // Language changed - update labels
        auto& strings = LocalizedStrings::getInstance();
        referenceLabel->setText(strings.getTFReferenceChannel(), juce::dontSendNotification);
        measurementLabel->setText(strings.getTFMeasurementChannel(), juce::dontSendNotification);
        repaint();
    }
}

void TransferFunctionView::timerCallback()
{
    updateSuggestions();
}

void TransferFunctionView::updateSuggestions()
{
    auto result = controller.getAnalysisResults();
    auto suggestions = controller.getSuggestions();
    
    suggestionsComponent->updateAnalysis(result, suggestions);
}
