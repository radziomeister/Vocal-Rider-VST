#include "PluginEditor.h"
#include "Theme.h"

VocalRiderAudioProcessorEditor::VocalRiderAudioProcessorEditor (VocalRiderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      rangeFader (p), riderFader (p), meter (p)
{
    setLookAndFeel (&metalLookAndFeel);

    titleLabel.setText ("VOCAL RIDER VST", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (17.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, Theme::textPrimary);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    // --- Rider on/off ---
    riderOnButton.setClickingTogglesState (true);
    riderOnButton.setColour (juce::TextButton::buttonColourId, Theme::panelInset);
    riderOnButton.setColour (juce::TextButton::buttonOnColourId, Theme::accentGreen);
    riderOnButton.setColour (juce::TextButton::textColourOffId, Theme::textSecondary);
    riderOnButton.setColour (juce::TextButton::textColourOnId, Theme::bgOuter);
    addAndMakeVisible (riderOnButton);
    riderOnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        audioProcessor.apvts, "riderEnabled", riderOnButton);

    // --- Live peak readout, above Target ---
    peakCaptionLabel.setText ("SYGNAL PEAK", juce::dontSendNotification);
    peakCaptionLabel.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    peakCaptionLabel.setColour (juce::Label::textColourId, Theme::textSecondary);
    peakCaptionLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (peakCaptionLabel);

    peakValueLabel.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    peakValueLabel.setJustificationType (juce::Justification::centredRight);
    peakValueLabel.setColour (juce::Label::textColourId, Theme::accentGreen);
    addAndMakeVisible (peakValueLabel);

    // --- Target bar ---
    targetTitleLabel.setText ("TARGET", juce::dontSendNotification);
    targetTitleLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    targetTitleLabel.setColour (juce::Label::textColourId, Theme::textSecondary);
    addAndMakeVisible (targetTitleLabel);

    targetSlider.setColour (juce::Slider::trackColourId, Theme::accentAmber);
    targetSlider.setColour (juce::Slider::backgroundColourId, Theme::faderTrack);
    targetSlider.setColour (juce::Slider::thumbColourId, Theme::textPrimary);
    addAndMakeVisible (targetSlider);
    targetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "target", targetSlider);

    targetValueLabel.setJustificationType (juce::Justification::centredRight);
    targetValueLabel.setColour (juce::Label::textColourId, Theme::accentAmber);
    targetValueLabel.setFont (juce::FontOptions (12.5f));
    addAndMakeVisible (targetValueLabel);

    // --- Listen selector ---
    listenTitleLabel.setText ("LISTEN", juce::dontSendNotification);
    listenTitleLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    listenTitleLabel.setColour (juce::Label::textColourId, Theme::textSecondary);
    addAndMakeVisible (listenTitleLabel);

    for (auto* b : { &vocalButton, &musicButton })
    {
        b->setClickingTogglesState (true);
        b->setRadioGroupId (1001);
        b->setColour (juce::TextButton::buttonColourId, Theme::panelInset);
        b->setColour (juce::TextButton::buttonOnColourId, Theme::accentBlue);
        b->setColour (juce::TextButton::textColourOffId, Theme::textSecondary);
        b->setColour (juce::TextButton::textColourOnId, Theme::bgOuter);
        addAndMakeVisible (*b);
    }
    vocalButton.onClick = [this] { if (vocalButton.getToggleState()) setListenSource (0); };
    musicButton.onClick = [this] { if (musicButton.getToggleState()) setListenSource (1); };

    // --- Attack / Release knobs ---
    for (auto* s : { &attackSlider, &releaseSlider })
    {
        s->setColour (juce::Slider::rotarySliderFillColourId, Theme::accentBlue);
        s->setColour (juce::Slider::rotarySliderOutlineColourId, Theme::strokeSubtle);
        s->setColour (juce::Slider::textBoxTextColourId, Theme::textPrimary);
        s->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 16);
        addAndMakeVisible (*s);
    }
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "attack", attackSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "release", releaseSlider);

    for (auto* l : { &attackLabel, &releaseLabel })
    {
        l->setJustificationType (juce::Justification::centred);
        l->setColour (juce::Label::textColourId, Theme::textSecondary);
        l->setFont (juce::FontOptions (11.5f));
        addAndMakeVisible (*l);
    }
    attackLabel.setText ("Attack", juce::dontSendNotification);
    releaseLabel.setText ("Release", juce::dontSendNotification);

    // --- Range / Rider / Output row ---
    addAndMakeVisible (rangeFader);
    addAndMakeVisible (riderFader);

    outputTitleLabel.setText ("Output", juce::dontSendNotification);
    outputTitleLabel.setJustificationType (juce::Justification::centred);
    outputTitleLabel.setColour (juce::Label::textColourId, Theme::textPrimary);
    outputTitleLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    addAndMakeVisible (outputTitleLabel);

    outputSlider.setColour (juce::Slider::thumbColourId, Theme::faderThumb);
    outputSlider.setColour (juce::Slider::trackColourId, Theme::accentBlue.withAlpha (0.35f));
    outputSlider.setColour (juce::Slider::backgroundColourId, Theme::faderTrack);
    addAndMakeVisible (outputSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "outputGain", outputSlider);

    outputValueLabel.setJustificationType (juce::Justification::centred);
    outputValueLabel.setColour (juce::Label::textColourId, Theme::textSecondary);
    outputValueLabel.setFont (juce::FontOptions (11.0f));
    addAndMakeVisible (outputValueLabel);

    addAndMakeVisible (meter);

    setSize (640, 584);
    setResizeLimits (520, 480, 900, 780);
    setResizable (true, true);

    startTimerHz (20);
    timerCallback();
}

VocalRiderAudioProcessorEditor::~VocalRiderAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
    stopTimer();
}

void VocalRiderAudioProcessorEditor::setListenSource (int index)
{
    if (auto* p = audioProcessor.apvts.getParameter ("listenSource"))
        p->setValueNotifyingHost (index == 0 ? 0.0f : 1.0f);
}

void VocalRiderAudioProcessorEditor::timerCallback()
{
    const float peakDb = audioProcessor.riderEngine.getMeterPeakDb();
    peakValueLabel.setText ((peakDb <= -99.0f ? juce::String ("-Inf") : juce::String (peakDb, 1)) + " dB",
                             juce::dontSendNotification);
    peakValueLabel.setColour (juce::Label::textColourId,
                               peakDb >= -1.0f ? Theme::accentRed
                                                : (peakDb >= -6.0f ? Theme::accentAmber : Theme::accentGreen));

    if (auto* targetVal = audioProcessor.apvts.getRawParameterValue ("target"))
        targetValueLabel.setText (juce::String (targetVal->load(), 1) + " dB", juce::dontSendNotification);

    if (auto* outVal = audioProcessor.apvts.getRawParameterValue ("outputGain"))
        outputValueLabel.setText ((outVal->load() >= 0.0f ? "+" : "") + juce::String (outVal->load(), 1) + " dB",
                                   juce::dontSendNotification);

    if (auto* listenVal = audioProcessor.apvts.getRawParameterValue ("listenSource"))
    {
        const bool music = listenVal->load() > 0.5f;
        if (! vocalButton.isMouseButtonDown() && ! musicButton.isMouseButtonDown())
        {
            vocalButton.setToggleState (! music, juce::dontSendNotification);
            musicButton.setToggleState (music, juce::dontSendNotification);
        }
    }

    repaint();
}

void VocalRiderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::bgOuter);

    auto panel = getLocalBounds().reduced (10).toFloat();

    juce::ColourGradient panelGrad (juce::Colour (0xff32353b), panel.getTopLeft(),
                                     juce::Colour (0xff17181c), panel.getBottomRight(), false);
    panelGrad.addColour (0.5, juce::Colour (0xff23262c));
    g.setGradientFill (panelGrad);
    g.fillRoundedRectangle (panel, 10.0f);

    // Diagonal gloss sheen across the upper-left of the panel.
    juce::Path sheen;
    sheen.addQuadrilateral (panel.getX(), panel.getY(),
                             panel.getX() + panel.getWidth() * 0.58f, panel.getY(),
                             panel.getX() + panel.getWidth() * 0.28f, panel.getY() + panel.getHeight() * 0.4f,
                             panel.getX(), panel.getY() + panel.getHeight() * 0.4f);
    g.setColour (juce::Colours::white.withAlpha (0.035f));
    g.fillPath (sheen);

    // Bevel: light edge top-left, dark edge bottom-right.
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawRoundedRectangle (panel.reduced (0.5f), 10.0f, 1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.drawRoundedRectangle (panel.expanded (0.5f).translated (0.5f, 0.5f), 10.0f, 1.0f);
}

void VocalRiderAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    // --- Target: live input-level marker on the horizontal bar, so you can
    // see at a glance how far the live signal sits from the Target setting.
    {
        auto bounds = targetSlider.getBounds().toFloat();
        constexpr float minDb = -40.0f, maxDb = 0.0f;
        const float level = audioProcessor.riderEngine.getMeterLevelDb();
        const float t = juce::jlimit (0.0f, 1.0f, (level - minDb) / (maxDb - minDb));
        const float x = bounds.getX() + t * bounds.getWidth();

        g.setColour (Theme::accentBlue);
        g.fillRoundedRectangle (x - 1.0f, bounds.getY() + 2.0f, 2.0f, bounds.getHeight() - 4.0f, 1.0f);
    }

    // --- Range: handled inside RangeFaderComponent itself ---
    // --- Rider: handled inside LiveGainFaderComponent itself ---

    // --- Output: live post-gain level meter (independent dBFS scale, not
    // the trim's -12..+12 dB range) filling the track from the bottom.
    {
        auto bounds = outputSlider.getBounds().toFloat();
        auto track = bounds.withSizeKeepingCentre (juce::jmin (10.0f, bounds.getWidth()), bounds.getHeight());
        constexpr float minDb = -48.0f, maxDb = 0.0f;
        const float level = audioProcessor.getOutputLevelDb();
        const float t = juce::jlimit (0.0f, 1.0f, (level - minDb) / (maxDb - minDb));
        const float fillHeight = t * track.getHeight();

        g.setColour (Theme::accentAmber.withAlpha (0.5f));
        g.fillRoundedRectangle (track.getX(), track.getBottom() - fillHeight, track.getWidth(), fillHeight, 3.0f);
        g.setColour (Theme::accentAmber);
        g.fillRoundedRectangle (track.getX() - 2.0f, track.getBottom() - fillHeight - 1.2f,
                                 track.getWidth() + 4.0f, 2.4f, 1.2f);
    }
}

void VocalRiderAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (22);

    auto header = area.removeFromTop (30);
    riderOnButton.setBounds (header.removeFromRight (78));
    titleLabel.setBounds (header);

    area.removeFromTop (14);

    auto peakRow = area.removeFromTop (24);
    peakCaptionLabel.setBounds (peakRow.removeFromLeft (100));
    peakValueLabel.setBounds (peakRow);

    area.removeFromTop (6);

    auto targetRow = area.removeFromTop (26);
    targetTitleLabel.setBounds (targetRow.removeFromLeft (60));
    targetValueLabel.setBounds (targetRow.removeFromRight (64));
    targetRow.removeFromLeft (8);
    targetRow.removeFromRight (8);
    targetSlider.setBounds (targetRow);

    area.removeFromTop (18);

    auto controlsRow = area.removeFromTop (76);
    auto listenArea = controlsRow.removeFromLeft ((int) (controlsRow.getWidth() * 0.34f)).reduced (0, 4);
    listenTitleLabel.setBounds (listenArea.removeFromTop (16));
    auto listenButtons = listenArea.removeFromTop (34);
    vocalButton.setBounds (listenButtons.removeFromLeft (listenButtons.getWidth() / 2).reduced (3, 0));
    musicButton.setBounds (listenButtons.reduced (3, 0));

    const int knobColWidth = controlsRow.getWidth() / 2;

    auto attackArea = controlsRow.removeFromLeft (knobColWidth).reduced (12, 0);
    attackSlider.setBounds (attackArea.removeFromTop (attackArea.getHeight() - 16));
    attackLabel.setBounds (attackArea);

    auto releaseArea = controlsRow.reduced (12, 0);
    releaseSlider.setBounds (releaseArea.removeFromTop (releaseArea.getHeight() - 16));
    releaseLabel.setBounds (releaseArea);

    area.removeFromTop (16);

    auto meterArea = area.removeFromBottom (100);
    area.removeFromBottom (14);
    meter.setBounds (meterArea);

    // Range / Rider / Output fader row shares the remaining space.
    auto faderRow = area;
    const int colWidth = faderRow.getWidth() / 3;

    auto rangeCol = faderRow.removeFromLeft (colWidth);
    rangeFader.setBounds (rangeCol.reduced (6, 0));

    auto riderCol = faderRow.removeFromLeft (colWidth);
    riderFader.setBounds (riderCol.reduced (6, 0));

    auto outputCol = faderRow.reduced (6, 0);
    outputTitleLabel.setBounds (outputCol.removeFromTop (18));
    outputValueLabel.setBounds (outputCol.removeFromBottom (16));
    outputSlider.setBounds (outputCol.reduced (outputCol.getWidth() / 3, 4));
}
