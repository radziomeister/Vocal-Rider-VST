#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class VocalRiderAudioProcessor;

// Dual-handle vertical fader for Range Down / Range Up, mirroring the
// reference plugin's single "Range" fader with two thumbs converging on 0.
class RangeFaderComponent : public juce::Component,
                             private juce::Timer
{
public:
    explicit RangeFaderComponent (VocalRiderAudioProcessor& proc);
    ~RangeFaderComponent() override;

    void resized() override;
    void paintOverChildren (juce::Graphics&) override;

private:
    void timerCallback() override;
    void pullFromProcessor();
    void pushToProcessor();

    VocalRiderAudioProcessor& processor;

    juce::Slider slider;
    juce::Label titleLabel, valueLabel;

    bool internalChange = false;
    float liveGainDb = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RangeFaderComponent)
};
