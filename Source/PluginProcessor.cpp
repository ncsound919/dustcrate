#include "PluginProcessor.h"
#include "PluginEditor.h"

DustCrateAudioProcessor::DustCrateAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    formatManager.registerBasicFormats(); // WAV, AIFF, FLAC, OGG

    // Polyphonic rompler-style synth
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new SampleVoice());

    synth.addSound(new SampleSound());
}

DustCrateAudioProcessor::~DustCrateAudioProcessor() {}

//==============================================================================
void DustCrateAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void DustCrateAudioProcessor::releaseResources() {}

void DustCrateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Merge any UI-generated preview MIDI into the host MIDI stream
    if (! pendingMidi.isEmpty())
    {
        for (const auto meta : pendingMidi)
            midiMessages.addEvent(meta.getMessage(), meta.samplePosition);
        pendingMidi.clear();
    }

    buffer.clear();
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // TODO: apply global character processing (Drift / VHS / Cassette) here
}

//==============================================================================
void DustCrateAudioProcessor::triggerSample(const juce::String& filePath,
                                            int midiNote, float velocity)
{
    const juce::File file(filePath);
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
        return;

    // Install reader into all SampleVoice instances
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SampleVoice*>(synth.getVoice(i)))
        {
            // AudioFormatReaderSource takes ownership of the reader when
            // configured to delete the reader when done.
            voice->setReader(reader.release(), midiNote);
            break; // one shared reader is enough for now
        }
    }

    juce::MidiMessage on = juce::MidiMessage::noteOn(1, midiNote, velocity);
    pendingMidi.addEvent(on, 0);
}

void DustCrateAudioProcessor::stopAllVoices()
{
    synth.allNotesOff(0, true);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
DustCrateAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Amplitude envelope
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "attack",  "Attack",  0.001f, 2.0f,  0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "decay",   "Decay",   0.001f, 2.0f,  0.10f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "sustain", "Sustain", 0.0f,   1.0f,  0.80f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "release", "Release", 0.001f, 4.0f,  0.20f));

    // Filter
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterCutoff", "Filter Cutoff", 20.0f, 20000.0f, 8000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterRes", "Filter Resonance", 0.1f, 10.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "filterType", "Filter Type",
        juce::StringArray { "Lowpass", "Highpass" }, 0));

    // Global pitch offset
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "pitchSemitones", "Pitch (semitones)", -24.0f, 24.0f, 0.0f));

    // Noise layer macro (for future use)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "noiseLevel", "Noise Level", 0.0f, 1.0f, 0.35f));

    // Character macros
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "driftAmount", "Drift", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "vhsAmount", "VHS", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "cassetteAmount", "Cassette", 0.0f, 1.0f, 0.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
void DustCrateAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DustCrateAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
juce::AudioProcessorEditor* DustCrateAudioProcessor::createEditor()
{
    return new DustCrateAudioProcessorEditor(*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DustCrateAudioProcessor();
}
