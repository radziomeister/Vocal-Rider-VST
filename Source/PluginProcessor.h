#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "RiderDSP.h"

class VocalRiderAudioProcessor : public juce::AudioProcessor
{
public:
    VocalRiderAudioProcessor();
    ~VocalRiderAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static BusesProperties makeBusesProperties();

    juce::AudioProcessorValueTreeState apvts;
    RiderEngine riderEngine;

    // Meter data snapshot for the editor to poll (thread-safe atomics live inside riderEngine).
    bool isMusicListenMode() const { return listenSourceParam->getIndex() == 1; }

    // Live post-gain output level (RMS, dB), measured each block after the
    // rider gain and output trim have been applied - used to draw a live
    // meter on the Output fader.
    float getOutputLevelDb() const { return outputLevelDb.load (std::memory_order_relaxed); }

private:
    std::atomic<float>* targetParam       = nullptr;
    std::atomic<float>* rangeUpParam      = nullptr;
    std::atomic<float>* rangeDownParam    = nullptr;
    std::atomic<float>* attackParam       = nullptr;
    std::atomic<float>* releaseParam      = nullptr;
    std::atomic<float>* outputGainParam   = nullptr;
    std::atomic<float>* riderEnabledParam = nullptr;
    juce::AudioParameterChoice* listenSourceParam = nullptr;

    juce::AudioBuffer<float> sidechainMonoScratch;
    float previousTotalGain = 1.0f;
    std::atomic<float> outputLevelDb { -60.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalRiderAudioProcessor)
};
