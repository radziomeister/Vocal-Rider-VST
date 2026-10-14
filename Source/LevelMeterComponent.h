#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <deque>

class VocalRiderAudioProcessor;

// Scrolling strip showing tracked level, the target line, and the amount of
// gain currently being applied by the rider - a simplified take on the
// reference plugin's live gain-riding history graph.
class LevelMeterComponent : public juce::Component,
                             public juce::SettableTooltipClient,
                             private juce::Timer
{
public:
    explicit LevelMeterComponent (VocalRiderAudioProcessor& proc);
    ~LevelMeterComponent() override;

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;

    static constexpr float minDb = -60.0f;
    static constexpr float maxDb = 0.0f;

    float dbToY (float db, float height) const
    {
        const float t = juce::jlimit (0.0f, 1.0f, (db - minDb) / (maxDb - minDb));
        return height * (1.0f - t);
    }

    VocalRiderAudioProcessor& processor;

    static constexpr int historySize = 300;
    std::deque<float> levelHistory;
    std::deque<float> gainHistory;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeterComponent)
};
