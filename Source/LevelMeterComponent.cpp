#include "LevelMeterComponent.h"
#include "PluginProcessor.h"

LevelMeterComponent::LevelMeterComponent (VocalRiderAudioProcessor& proc)
    : processor (proc)
{
    setTooltip (juce::CharPointer_UTF8 (
        "Zolta przerywana: Target (poziom docelowy)\n"
        "Niebieska: aktualny poziom sygnalu wejsciowego\n"
        "Zielona: wzmocnienie stosowane przez Rider"));

    startTimerHz (30);
}

LevelMeterComponent::~LevelMeterComponent()
{
    stopTimer();
}

void LevelMeterComponent::timerCallback()
{
    levelHistory.push_back (processor.riderEngine.getMeterLevelDb());
    gainHistory.push_back (processor.riderEngine.getMeterGainDb());

    while ((int) levelHistory.size() > historySize) levelHistory.pop_front();
    while ((int) gainHistory.size()  > historySize) gainHistory.pop_front();

    repaint();
}

void LevelMeterComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient bgGrad (juce::Colour (0xff222429), bounds.getTopLeft(),
                                  juce::Colour (0xff121316), bounds.getBottomRight(), false);
    g.setGradientFill (bgGrad);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, 1.0f);

    const float w = bounds.getWidth();
    const float h = bounds.getHeight();

    // dB gridlines
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    for (float db = 0.0f; db >= minDb; db -= 12.0f)
    {
        const float y = dbToY (db, h);
        g.drawHorizontalLine ((int) y, 0.0f, w);
    }

    // Target line
    const auto* targetParam = processor.apvts.getRawParameterValue ("target");
    const float targetDb = targetParam != nullptr ? targetParam->load() : -18.0f;
    const float targetY = dbToY (targetDb, h);
    g.setColour (juce::Colour (0xffe0b23a));
    {
        juce::Path dashed;
        dashed.startNewSubPath (0.0f, targetY);
        dashed.lineTo (w, targetY);
        float dashLengths[] = { 6.0f, 4.0f };
        juce::Path dashedPath;
        juce::PathStrokeType (1.5f).createDashedStroke (dashedPath, dashed, dashLengths, 2);
        g.strokePath (dashedPath, juce::PathStrokeType (1.5f));
    }

    // Level history trace
    if (levelHistory.size() > 1)
    {
        juce::Path p;
        const float step = w / (float) (historySize - 1);
        const int startIdx = historySize - (int) levelHistory.size();

        bool first = true;
        for (int i = 0; i < (int) levelHistory.size(); ++i)
        {
            const float x = (startIdx + i) * step;
            const float y = dbToY (levelHistory[(size_t) i], h);
            if (first) { p.startNewSubPath (x, y); first = false; }
            else       { p.lineTo (x, y); }
        }
        g.setColour (juce::Colour (0xff5ec9f0));
        g.strokePath (p, juce::PathStrokeType (1.8f));
    }

    // Gain-riding amount as a filled band around unity
    if (gainHistory.size() > 1)
    {
        juce::Path p;
        const float step = w / (float) (historySize - 1);
        const int startIdx = historySize - (int) gainHistory.size();
        const float zeroY = dbToY (targetDb, h);

        bool first = true;
        for (int i = 0; i < (int) gainHistory.size(); ++i)
        {
            const float x = (startIdx + i) * step;
            const float y = zeroY - gainHistory[(size_t) i] * 4.0f;
            if (first) { p.startNewSubPath (x, y); first = false; }
            else       { p.lineTo (x, y); }
        }
        g.setColour (juce::Colour (0xff77e0a0));
        g.strokePath (p, juce::PathStrokeType (1.2f));
    }

    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.setFont (11.0f);
    g.drawText ("Target " + juce::String (targetDb, 1) + " dB", bounds.reduced (6.0f),
                juce::Justification::topLeft, false);
}
