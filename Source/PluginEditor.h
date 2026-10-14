#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "LevelMeterComponent.h"
#include "RangeFaderComponent.h"
#include "LiveGainFaderComponent.h"
#include "MetalLookAndFeel.h"

class VocalRiderAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit VocalRiderAudioProcessorEditor (VocalRiderAudioProcessor&);
    ~VocalRiderAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setListenSource (int index);

    VocalRiderAudioProcessor& audioProcessor;

    MetalLookAndFeel metalLookAndFeel;

    juce::Label titleLabel;

    // Live input peak-level readout, shown above Target so the current dB
    // of the signal is always visible without an external meter.
    juce::Label peakCaptionLabel, peakValueLabel;

    // Target: horizontal bar fader, like the reference plugin's Target strip.
    juce::Slider targetSlider { juce::Slider::LinearBar, juce::Slider::NoTextBox };
    juce::Label targetTitleLabel, targetValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> targetAttachment;

    // Rider on/off, styled as an LED-lit pill toggle.
    juce::TextButton riderOnButton { "RIDER" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> riderOnAttachment;

    // Listen source: Vocal / Music segmented selector.
    juce::TextButton vocalButton { "VOCAL" };
    juce::TextButton musicButton { "MUSIC" };
    juce::Label listenTitleLabel;

    // Attack / Release knobs - independent rise and fall time of the rider's
    // gain smoothing, so the user can dial in how aggressive vs. gentle the
    // dynamic response sounds.
    juce::Slider attackSlider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label attackLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;

    juce::Slider releaseSlider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label releaseLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    // Range / Rider / Output fader row.
    RangeFaderComponent rangeFader;
    LiveGainFaderComponent riderFader;

    juce::Slider outputSlider { juce::Slider::LinearVertical, juce::Slider::NoTextBox };
    juce::Label outputTitleLabel, outputValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;

    LevelMeterComponent meter;

    // Shows a component's tooltip after hovering motionless for 2 seconds;
    // hides again as soon as the cursor moves.
    juce::TooltipWindow tooltipWindow { this, 2000 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalRiderAudioProcessorEditor)
};
