#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <deque>

class VocalRiderAudioProcessor;

// Read-only vertical fader visualizing the gain the rider engine is
// currently applying - mirrors the reference plugin's automated "Rider"
// fader, which moves on its own while the automation is live.
class LiveGainFaderComponent : public juce::Component,
                                private juce::Timer
{
public:
    explicit LiveGainFaderComponent (VocalRiderAudioProcessor& proc);
    ~LiveGainFaderComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    static constexpr float minDb = -12.0f;
    static constexpr float maxDb = 12.0f;

    float dbToY (float db, juce::Rectangle<float> track) const
    {
        const float t = juce::jlimit (0.0f, 1.0f, (db - minDb) / (maxDb - minDb));
        return track.getBottom() - t * track.getHeight();
    }

    VocalRiderAudioProcessor& processor;
    float displayedGainDb = 0.0f;
    juce::Label titleLabel, valueLabel;

    static constexpr int trailLength = 45; // ~1.5s at 30Hz
    std::deque<float> trail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveGainFaderComponent)
};
