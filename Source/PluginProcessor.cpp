#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <vector>

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
    currentSampleRate = sampleRate;
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void DustCrateAudioProcessor::releaseResources() {}

void DustCrateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    {
        const juce::ScopedLock sl (synthLock);

        // Merge any UI-generated preview MIDI into the host MIDI stream
        if (! pendingMidi.isEmpty())
        {
            for (const auto meta : pendingMidi)
                midiMessages.addEvent(meta.getMessage(), meta.samplePosition);
            pendingMidi.clear();
        }

        synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    }

    // TODO: apply global character processing (Drift / VHS / Cassette / Noise) here
}

//==============================================================================
bool DustCrateAudioProcessor::selectSample(const juce::String& filePath, int rootNote)
{
    const juce::File file(filePath);

    // Perform file I/O and decoder creation outside the synth lock
    const int numVoices = synth.getNumVoices();
    std::vector<std::unique_ptr<juce::AudioFormatReader>> readers;
    readers.resize(static_cast<size_t>(numVoices));

    bool anyLoaded = false;
    for (int i = 0; i < numVoices; ++i)
    {
        readers[(size_t) i].reset(formatManager.createReaderFor(file));
        if (readers[(size_t) i] != nullptr)
            anyLoaded = true;
    }

    if (! anyLoaded)
        return false;

    // Safely install readers into voices under the synth lock
    const juce::ScopedLock sl (synthLock);

    for (int i = 0; i < numVoices; ++i)
    {
        if (auto* voice = dynamic_cast<SampleVoice*>(synth.getVoice(i)))
        {
            if (readers[(size_t) i] != nullptr)
                voice->setReader(readers[(size_t) i].release(), rootNote);
        }
    }

    return true;
}

void DustCrateAudioProcessor::triggerSample(const juce::String& filePath,
                                            int midiNote, float velocity)
{
    // Load sample readers first; if that fails, do not enqueue a note-on.
    if (! selectSample(filePath, midiNote))
        return;

    const juce::ScopedLock sl (synthLock);
    juce::MidiMessage on = juce::MidiMessage::noteOn(1, midiNote, velocity);
    pendingMidi.addEvent(on, 0);
}

void DustCrateAudioProcessor::stopAllVoices()
{
    const juce::ScopedLock sl (synthLock);
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
