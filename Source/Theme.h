#pragma once

#include <juce_graphics/juce_graphics.h>

// Shared dark color palette for the plugin UI.
namespace Theme
{
    static const juce::Colour bgOuter      { 0xff121316 };
    static const juce::Colour panelTop     { 0xff1a1c21 };
    static const juce::Colour panelMain    { 0xff1e2126 };
    static const juce::Colour panelInset   { 0xff17181c };
    static const juce::Colour strokeSubtle { 0xff2c2f36 };

    static const juce::Colour textPrimary   { 0xffe8e9ec };
    static const juce::Colour textSecondary { 0xff8b8f98 };

    static const juce::Colour accentGreen  { 0xff3ddc84 };
    static const juce::Colour accentAmber  { 0xffe0b23a };
    static const juce::Colour accentBlue   { 0xff5ec9f0 };
    static const juce::Colour accentRed    { 0xffe0555a };

    static const juce::Colour faderTrack   { 0xff0e0f12 };
    static const juce::Colour faderThumb   { 0xffd8dade };
}
