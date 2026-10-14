#include "LiveGainFaderComponent.h"
#include "PluginProcessor.h"
#include "Theme.h"

LiveGainFaderComponent::LiveGainFaderComponent (VocalRiderAudioProcessor& proc)
    : processor (proc)
{
    titleLabel.setText ("Rider", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, Theme::textPrimary);
    titleLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setColour (juce::Label::textColourId, Theme::accentGreen);
    valueLabel.setFont (juce::FontOptions (11.0f));
    addAndMakeVisible (valueLabel);

    startTimerHz (30);
}

LiveGainFaderComponent::~LiveGainFaderComponent()
{
    stopTimer();
}

void LiveGainFaderComponent::resized()
{
    auto area = getLocalBounds();
    titleLabel.setBounds (area.removeFromTop (18));
    valueLabel.setBounds (area.removeFromBottom (16));
}

void LiveGainFaderComponent::timerCallback()
{
    displayedGainDb = processor.riderEngine.getMeterGainDb();
    valueLabel.setText ((displayedGainDb >= 0.0f ? "+" : "") + juce::String (displayedGainDb, 1) + " dB",
                         juce::dontSendNotification);

    trail.push_back (displayedGainDb);
    while ((int) trail.size() > trailLength)
        trail.pop_front();

    repaint();
}

void LiveGainFaderComponent::paint (juce::Graphics& g)
{
    auto full = getLocalBounds().toFloat();
    auto track = full.withTrimmedTop (20.0f).withTrimmedBottom (18.0f).reduced (full.getWidth() * 0.28f, 4.0f);

    juce::ColourGradient grooveGrad (juce::Colour (0xff0e0f12), track.getTopLeft(),
                                      juce::Colour (0xff26292f), track.getBottomRight(), false);
    g.setGradientFill (grooveGrad);
    g.fillRoundedRectangle (track, 3.0f);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawRoundedRectangle (track, 3.0f, 1.0f);

    // Tick marks at -12, -6, 0, +6, +12
    g.setColour (Theme::textSecondary.withAlpha (0.6f));
    g.setFont (9.0f);
    for (float db = minDb; db <= maxDb; db += 6.0f)
    {
        const float y = dbToY (db, track);
        g.drawHorizontalLine ((int) y, track.getRight() + 2.0f, track.getRight() + 7.0f);
    }

    // Zero reference line
    const float zeroY = dbToY (0.0f, track);
    g.setColour (Theme::accentRed.withAlpha (0.5f));
    g.drawHorizontalLine ((int) zeroY, track.getX(), track.getRight());

    // Fading "comet trail" of recent gain values, echoing behind the cap so
    // the fader reads as a live graph rather than a single static value.
    for (size_t i = 0; i < trail.size(); ++i)
    {
        const size_t age = trail.size() - 1 - i;
        if (age == 0)
            continue; // the current value is drawn as the solid cap below

        const float y = dbToY (trail[i], track);
        const float alpha = 0.35f * (1.0f - (float) age / (float) trailLength);
        if (alpha <= 0.01f)
            continue;

        g.setColour (Theme::accentGreen.withAlpha (alpha));
        g.fillRoundedRectangle (track.getX(), y - 0.9f, track.getWidth(), 1.8f, 0.9f);
    }

    // Moving cap representing the live applied gain
    const float capY = dbToY (juce::jlimit (minDb, maxDb, displayedGainDb), track);
    const float capHeight = 10.0f;
    auto cap = juce::Rectangle<float> (track.getX() - 3.0f, capY - capHeight * 0.5f,
                                        track.getWidth() + 6.0f, capHeight);

    juce::ColourGradient capGrad (Theme::accentGreen.brighter (0.4f), cap.getTopLeft(),
                                   Theme::accentGreen.darker (0.3f), cap.getBottomRight(), false);
    g.setGradientFill (capGrad);
    g.fillRoundedRectangle (cap, 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.3f));
    g.drawRoundedRectangle (cap, 2.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.drawHorizontalLine ((int) cap.getCentreY(), cap.getX() + 2.0f, cap.getRight() - 2.0f);

    // Fill between zero and cap to show direction of the ride
    auto fill = (capY < zeroY)
        ? juce::Rectangle<float> (track.getX(), capY, track.getWidth(), zeroY - capY)
        : juce::Rectangle<float> (track.getX(), zeroY, track.getWidth(), capY - zeroY);
    g.setColour (Theme::accentGreen.withAlpha (0.18f));
    g.fillRect (fill);
}
