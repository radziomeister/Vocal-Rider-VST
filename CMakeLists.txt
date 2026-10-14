cmake_minimum_required(VERSION 3.22)

project(VocalRiderVST VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures")

add_subdirectory(third_party/JUCE)

juce_add_plugin(VocalRiderVST
    COMPANY_NAME "OpenRider"
    PLUGIN_MANUFACTURER_CODE Rdrx
    PLUGIN_CODE Vrid
    FORMATS VST3 Standalone
    PRODUCT_NAME "Vocal Rider VST"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Fx" "Dynamics"
)

juce_generate_juce_header(VocalRiderVST)

target_sources(VocalRiderVST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/LevelMeterComponent.cpp
        Source/RangeFaderComponent.cpp
        Source/LiveGainFaderComponent.cpp
        Source/MetalLookAndFeel.cpp
)

target_compile_definitions(VocalRiderVST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(VocalRiderVST
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
