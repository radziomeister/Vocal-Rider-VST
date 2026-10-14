#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Dark brushed-metal look and feel: gradient-filled knobs and faders with a
// soft specular highlight, used across every slider/button in the plugin.
class MetalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MetalLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                            juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPos, float minSliderPos, float maxSliderPos,
                            const juce::Slider::SliderStyle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};
