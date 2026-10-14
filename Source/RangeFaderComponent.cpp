#include "RangeFaderComponent.h"
#include "PluginProcessor.h"
#include "Theme.h"

RangeFaderComponent::RangeFaderComponent (VocalRiderAudioProcessor& proc)
    : processor (proc)
{
    slider.setSliderStyle (juce::Slider::TwoValueVertical);
    slider.setRange (-20.0, 20.0, 0.1);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    slider.setColour (juce::Slider::thumbColourId, Theme::faderThumb);
    slider.setColour (juce::Slider::trackColourId, Theme::accentBlue.withAlpha (0.35f));
    slider.setColour (juce::Slider::backgroundColourId, Theme::faderTrack);
    addAndMakeVisible (slider);

    slider.onValueChange = [this]
    {
        if (! internalChange)
            pushToProcessor();
    };

    slider.onDragEnd = [this] { pushToProcessor(); };

    titleLabel.setText ("Range", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, Theme::textPrimary);
    titleLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setColour (juce::Label::textColourId, Theme::textSecondary);
    valueLabel.setFont (juce::FontOptions (11.0f));
    addAndMakeVisible (valueLabel);

    pullFromProcessor();
    startTimerHz (20);
}

RangeFaderComponent::~RangeFaderComponent()
{
    stopTimer();
}

void RangeFaderComponent::resized()
{
    auto area = getLocalBounds();
    titleLabel.setBounds (area.removeFromTop (18));
    valueLabel.setBounds (area.removeFromBottom (16));
    slider.setBounds (area.reduced (getWidth() / 3, 4));
}

void RangeFaderComponent::paintOverChildren (juce::Graphics& g)
{
    auto trackBounds = slider.getBounds().toFloat();
    auto groove = trackBounds.withSizeKeepingCentre (juce::jmin (10.0f, trackBounds.getWidth()), trackBounds.getHeight());

    const float t = juce::jlimit (0.0f, 1.0f, (liveGainDb + 20.0f) / 40.0f);
    const float y = groove.getBottom() - t * groove.getHeight();

    g.setColour (Theme::accentGreen);
    g.fillRoundedRectangle (groove.getX() - 5.0f, y - 1.4f, groove.getWidth() + 10.0f, 2.8f, 1.4f);
    g.setColour (Theme::accentGreen.withAlpha (0.55f));
    g.drawEllipse (groove.getCentreX() - 3.5f, y - 3.5f, 7.0f, 7.0f, 1.4f);
}

void RangeFaderComponent::timerCallback()
{
    liveGainDb = processor.riderEngine.getMeterGainDb();
    repaint();

    if (! isMouseButtonDown())
        pullFromProcessor();
}

void RangeFaderComponent::pullFromProcessor()
{
    auto* downVal = processor.apvts.getRawParameterValue ("rangeDown");
    auto* upVal   = processor.apvts.getRawParameterValue ("rangeUp");
    if (downVal == nullptr || upVal == nullptr)
        return;

    const float down = downVal->load();
    const float up    = upVal->load();

    internalChange = true;
    slider.setMinAndMaxValues (-(double) down, (double) up, juce::dontSendNotification);
    internalChange = false;

    valueLabel.setText ("-" + juce::String (down, 1) + " / +" + juce::String (up, 1), juce::dontSendNotification);
}

void RangeFaderComponent::pushToProcessor()
{
    if (auto* downParam = processor.apvts.getParameter ("rangeDown"))
        downParam->setValueNotifyingHost (downParam->convertTo0to1 ((float) -slider.getMinValue()));

    if (auto* upParam = processor.apvts.getParameter ("rangeUp"))
        upParam->setValueNotifyingHost (upParam->convertTo0to1 ((float) slider.getMaxValue()));

    valueLabel.setText ("-" + juce::String (-slider.getMinValue(), 1) + " / +" + juce::String (slider.getMaxValue(), 1),
                         juce::dontSendNotification);
}
