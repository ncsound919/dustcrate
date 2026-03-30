#include "PluginProcessor.h"
#include "PluginEditor.h"

DustCrateAudioProcessor::DustCrateAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    formatManager.registerBasicFormats(); // WAV, AIFF, FLAC, OGG

    // Add 16 voices to the synthesiser
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new SampleVoice());

    synth.addSound(new SampleSound());
}

DustCrateAudioProcessor::~DustCrateAudioProcessor() {}

void DustCrateAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    // Prepare DSP chain here if adding convolution/reverb later
}

void DustCrateAudioProcessor::releaseResources() {}

void DustCrateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

void DustCrateAudioProcessor::triggerSample(const juce::String& filePath,
                                             int midiNote, float velocity)
{
    // Load sample into voices and send a MIDI note-on
    auto* file = new juce::File(filePath);
    auto* reader = formatManager.createReaderFor(*file);
    if (reader == nullptr) return;

    // Pass reader to voices via a shared ptr approach in SampleVoice
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SampleVoice*>(synth.getVoice(i)))
        {
            voice->setReader(formatManager.createReaderFor(*file), midiNote);
            break;
        }
    }

    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, midiNote, velocity), 0);
    synth.renderNextBlock(
        *new juce::AudioBuffer<float>(2, 0), midi, 0, 0);
}

void DustCrateAudioProcessor::stopAllVoices()
{
    synth.allNotesOff(0, true);
}

juce::AudioProcessorValueTreeState::ParameterLayout
DustCrateAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "attack",  "Attack",  0.001f, 2.0f,  0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "decay",   "Decay",   0.001f, 2.0f,  0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "sustain", "Sustain", 0.0f,   1.0f,  0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "release", "Release", 0.001f, 4.0f,  0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterCutoff", "Filter Cutoff", 20.0f, 20000.0f, 8000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterRes", "Filter Resonance", 0.1f, 10.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "filterType", "Filter Type",
        juce::StringArray{"Lowpass", "Highpass"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "pitchSemitones", "Pitch (semitones)", -24.0f, 24.0f, 0.0f));

    return { params.begin(), params.end() };
}

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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DustCrateAudioProcessor();
}
