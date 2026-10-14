#include "MetalLookAndFeel.h"

namespace
{
    const juce::Colour metalLight { 0xff4a4d54 };
    const juce::Colour metalMid   { 0xff2c2f35 };
    const juce::Colour metalDark  { 0xff0e0f12 };
    const juce::Colour capLight   { 0xffe4e6ea };
    const juce::Colour capDark    { 0xff8f9298 };
}

MetalLookAndFeel::MetalLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe8e9ec));
}

void MetalLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    auto outer = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);

    juce::ColourGradient bezelGrad (metalLight, outer.getTopLeft(), metalDark, outer.getBottomRight(), false);
    g.setGradientFill (bezelGrad);
    g.fillEllipse (outer);

    auto inner = outer.reduced (radius * 0.16f);
    juce::ColourGradient faceGrad (metalMid, inner.getTopLeft(), metalDark, inner.getBottomRight(), false);
    g.setGradientFill (faceGrad);
    g.fillEllipse (inner);

    // Specular highlight, upper-left light source.
    auto highlight = inner.reduced (inner.getWidth() * 0.22f).translated (-inner.getWidth() * 0.08f, -inner.getHeight() * 0.12f);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.fillEllipse (highlight);

    const float arcRadius = radius - 2.0f;

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.strokePath (track, juce::PathStrokeType (2.0f));

    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
    g.strokePath (valueArc, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path pointer;
    const float pointerLength = radius * 0.6f;
    const float pointerThickness = 2.4f;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength * 0.6f, 1.2f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.fillPath (pointer);

    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.drawEllipse (outer, 1.0f);
}

void MetalLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    const bool isVertical = (style == juce::Slider::LinearVertical
                              || style == juce::Slider::TwoValueVertical
                              || style == juce::Slider::ThreeValueVertical
                              || style == juce::Slider::LinearBarVertical);

    juce::Rectangle<float> groove = isVertical
        ? bounds.withSizeKeepingCentre (juce::jmin (10.0f, bounds.getWidth()), bounds.getHeight())
        : bounds.withSizeKeepingCentre (bounds.getWidth(), juce::jmin (10.0f, bounds.getHeight()));

    juce::ColourGradient grooveGrad (metalDark, groove.getTopLeft(), metalMid, groove.getBottomRight(), false);
    g.setGradientFill (grooveGrad);
    g.fillRoundedRectangle (groove, 4.0f);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawRoundedRectangle (groove, 4.0f, 1.0f);

    const auto accent = slider.findColour (juce::Slider::trackColourId);

    auto drawThumb = [&] (float pos)
    {
        juce::Rectangle<float> thumb = isVertical
            ? juce::Rectangle<float> (bounds.getWidth(), 14.0f).withCentre ({ bounds.getCentreX(), pos })
            : juce::Rectangle<float> (10.0f, bounds.getHeight()).withCentre ({ pos, bounds.getCentreY() });

        juce::ColourGradient capGrad (capLight, thumb.getTopLeft(), capDark, thumb.getBottomRight(), false);
        g.setGradientFill (capGrad);
        g.fillRoundedRectangle (thumb, 3.0f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawRoundedRectangle (thumb, 3.0f, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        if (isVertical)
            g.drawHorizontalLine ((int) thumb.getCentreY(), thumb.getX() + 2.0f, thumb.getRight() - 2.0f);
        else
            g.drawVerticalLine ((int) thumb.getCentreX(), thumb.getY() + 2.0f, thumb.getBottom() - 2.0f);
    };

    if (style == juce::Slider::TwoValueVertical || style == juce::Slider::TwoValueHorizontal)
    {
        if (isVertical)
        {
            auto fillRect = groove.withY (juce::jmin (minSliderPos, maxSliderPos))
                                   .withBottom (juce::jmax (minSliderPos, maxSliderPos));
            g.setColour (accent);
            g.fillRoundedRectangle (fillRect, 4.0f);
        }
        else
        {
            auto fillRect = groove.withX (juce::jmin (minSliderPos, maxSliderPos))
                                   .withRight (juce::jmax (minSliderPos, maxSliderPos));
            g.setColour (accent);
            g.fillRoundedRectangle (fillRect, 4.0f);
        }

        drawThumb (minSliderPos);
        drawThumb (maxSliderPos);
    }
    else if (style == juce::Slider::LinearBar || style == juce::Slider::LinearBarVertical)
    {
        juce::Rectangle<float> fillRect = isVertical
            ? groove.withTop (sliderPos)
            : groove.withRight (sliderPos);

        juce::ColourGradient fillGrad (accent.brighter (0.2f), fillRect.getTopLeft(),
                                        accent.darker (0.25f), fillRect.getBottomRight(), false);
        g.setGradientFill (fillGrad);
        g.fillRoundedRectangle (fillRect, 4.0f);

        g.setColour (juce::Colours::white.withAlpha (0.85f));
        if (isVertical)
            g.drawHorizontalLine ((int) sliderPos, groove.getX(), groove.getRight());
        else
            g.drawVerticalLine ((int) sliderPos, groove.getY(), groove.getBottom());
    }
    else
    {
        drawThumb (sliderPos);
    }
}

void MetalLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const bool isOn = button.getToggleState();

    const juce::Colour base = isOn ? backgroundColour : metalMid;
    juce::ColourGradient grad (base.brighter (isOn ? 0.3f : 0.1f), bounds.getTopLeft(),
                                base.darker (0.4f), bounds.getBottomRight(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, 5.0f);

    if (isOn)
    {
        g.setColour (base.withAlpha (0.45f));
        g.drawRoundedRectangle (bounds.expanded (1.5f), 6.0f, 3.0f);
    }

    g.setColour (juce::Colours::white.withAlpha (shouldDrawButtonAsHighlighted ? 0.18f : 0.08f));
    g.drawRoundedRectangle (bounds, 5.0f, 1.0f);

    if (shouldDrawButtonAsDown)
    {
        g.setColour (juce::Colours::black.withAlpha (0.25f));
        g.fillRoundedRectangle (bounds, 5.0f);
    }
}
