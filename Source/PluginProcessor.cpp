#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace ParamID
{
    static const juce::String target       = "target";
    static const juce::String rangeUp      = "rangeUp";
    static const juce::String rangeDown    = "rangeDown";
    static const juce::String attack       = "attack";
    static const juce::String release      = "release";
    static const juce::String listenSource = "listenSource";
    static const juce::String riderEnabled = "riderEnabled";
    static const juce::String outputGain   = "outputGain";
}

VocalRiderAudioProcessor::BusesProperties VocalRiderAudioProcessor::makeBusesProperties()
{
    return BusesProperties()
        .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
        .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), false);
}

VocalRiderAudioProcessor::VocalRiderAudioProcessor()
    : AudioProcessor (makeBusesProperties()),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    targetParam       = apvts.getRawParameterValue (ParamID::target);
    rangeUpParam      = apvts.getRawParameterValue (ParamID::rangeUp);
    rangeDownParam    = apvts.getRawParameterValue (ParamID::rangeDown);
    attackParam       = apvts.getRawParameterValue (ParamID::attack);
    releaseParam      = apvts.getRawParameterValue (ParamID::release);
    outputGainParam   = apvts.getRawParameterValue (ParamID::outputGain);
    riderEnabledParam = apvts.getRawParameterValue (ParamID::riderEnabled);
    listenSourceParam = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (ParamID::listenSource));
}

juce::AudioProcessorValueTreeState::ParameterLayout VocalRiderAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::target, 1 }, "Target",
        juce::NormalisableRange<float> (-40.0f, 0.0f, 0.1f), -18.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::rangeUp, 1 }, "Range Up",
        juce::NormalisableRange<float> (0.0f, 20.0f, 0.1f), 10.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::rangeDown, 1 }, "Range Down",
        juce::NormalisableRange<float> (0.0f, 20.0f, 0.1f), 10.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::attack, 1 }, "Attack",
        juce::NormalisableRange<float> (5.0f, 1500.0f, 0.1f, 0.35f), 250.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::release, 1 }, "Release",
        juce::NormalisableRange<float> (20.0f, 3000.0f, 0.1f, 0.35f), 900.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::listenSource, 1 }, "Listen",
        juce::StringArray { "Vocal", "Music" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamID::riderEnabled, 1 }, "Rider", true));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::outputGain, 1 }, "Output",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    return { params.begin(), params.end() };
}

void VocalRiderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    riderEngine.prepare (sampleRate, samplesPerBlock);
    sidechainMonoScratch.setSize (1, samplesPerBlock);
}

bool VocalRiderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainIn  = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    if (mainIn != mainOut)
        return false;

    const auto sideSet = layouts.getChannelSet (true, 1);
    if (! sideSet.isDisabled() && sideSet != mainOut)
        return false;

    return true;
}

void VocalRiderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    auto mainInBlock  = getBusBuffer (buffer, true, 0);
    auto mainOutBlock = getBusBuffer (buffer, false, 0);

    // Push live parameter values into the DSP engine.
    riderEngine.setTargetDb (targetParam->load());
    riderEngine.setRangeUpDb (rangeUpParam->load());
    riderEngine.setRangeDownDb (rangeDownParam->load());
    riderEngine.setAttackMs (attackParam->load());
    riderEngine.setReleaseMs (releaseParam->load());
    riderEngine.setEnabled (riderEnabledParam->load() > 0.5f);

    // Build mono detector signal from either the main (vocal) input or the
    // sidechain (music) bus, depending on the Listen selector.
    sidechainMonoScratch.setSize (1, numSamples, false, false, true);
    auto* detector = sidechainMonoScratch.getWritePointer (0);
    std::fill (detector, detector + numSamples, 0.0f);

    const bool useMusicBus = isMusicListenMode() && getBus (true, 1) != nullptr && getBus (true, 1)->isEnabled();
    auto sideBlock = useMusicBus ? getBusBuffer (buffer, true, 1) : juce::AudioBuffer<float>();

    const auto& sourceForDetector = useMusicBus ? sideBlock : mainInBlock;
    const int sourceChannels = sourceForDetector.getNumChannels();

    if (sourceChannels > 0)
    {
        const float norm = 1.0f / (float) sourceChannels;
        for (int ch = 0; ch < sourceChannels; ++ch)
        {
            const float* src = sourceForDetector.getReadPointer (ch);
            for (int i = 0; i < numSamples; ++i)
                detector[i] += src[i] * norm;
        }
    }

    riderEngine.updateEnvelopeBlock (detector, numSamples);

    const float riderGain  = riderEngine.getCurrentGainLinear();
    const float outputTrim = juce::Decibels::decibelsToGain (outputGainParam->load());
    const float totalGain  = riderGain * outputTrim;

    for (int ch = 0; ch < mainOutBlock.getNumChannels(); ++ch)
    {
        if (ch < mainInBlock.getNumChannels()
            && mainOutBlock.getWritePointer (ch) != mainInBlock.getReadPointer (ch))
            mainOutBlock.copyFrom (ch, 0, mainInBlock, ch, 0, numSamples);

        mainOutBlock.applyGainRamp (ch, 0, numSamples, previousTotalGain, totalGain);
    }
    previousTotalGain = totalGain;

    // Live post-gain output level, for the Output fader's meter.
    const int outChannels = mainOutBlock.getNumChannels();
    if (outChannels > 0 && numSamples > 0)
    {
        float sumSquares = 0.0f;
        for (int ch = 0; ch < outChannels; ++ch)
        {
            const float* out = mainOutBlock.getReadPointer (ch);
            for (int i = 0; i < numSamples; ++i)
                sumSquares += out[i] * out[i];
        }
        const float rms = std::sqrt (sumSquares / (float) (outChannels * numSamples));
        outputLevelDb.store (juce::Decibels::gainToDecibels (rms, -100.0f), std::memory_order_relaxed);
    }
}

juce::AudioProcessorEditor* VocalRiderAudioProcessor::createEditor()
{
    return new VocalRiderAudioProcessorEditor (*this);
}

void VocalRiderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void VocalRiderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VocalRiderAudioProcessor();
}
