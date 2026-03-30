#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <vector>

DustCrateAudioProcessor::DustCrateAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    formatManager.registerBasicFormats();
    for (int i = 0; i < 16; ++i) synth.addVoice(new SampleVoice());
    synth.addSound(new SampleSound());
}

DustCrateAudioProcessor::~DustCrateAudioProcessor() {}

void DustCrateAudioProcessor::prepareToPlay(double sr, int blockSize)
{
    currentSampleRate = sr;
    synth.setCurrentPlaybackSampleRate(sr);
    characterProcessor.prepare(sr, blockSize);
    macroLFO.prepare(sr);
}

//==============================================================================
void DustCrateAudioProcessor::updateVoiceParameters()
{
    const float attack   = *apvts.getRawParameterValue("attack");
    const float decay    = *apvts.getRawParameterValue("decay");
    const float sustain  = *apvts.getRawParameterValue("sustain");
    const float release  = *apvts.getRawParameterValue("release");
    const float cutoff   = *apvts.getRawParameterValue("filterCutoff");
    const float res      = *apvts.getRawParameterValue("filterRes");
    const int   fType    = (int)*apvts.getRawParameterValue("filterType");
    const float pitch    = *apvts.getRawParameterValue("pitchSemitones");
    const float drift    = *apvts.getRawParameterValue("driftAmount");

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SampleVoice*>(synth.getVoice(i)))
        {
            v->setADSR(attack, decay, sustain, release);
            v->setFilter(cutoff, res, fType == 1);
            v->setPitchShift(pitch);
            v->setDriftRatio(drift);
        }
}

void DustCrateAudioProcessor::applyMacroLFO()
{
    // Tick LFO and modulate its target parameters
    const float lfoVal = macroLFO.tick(0); // 0 = value already ticked in processBlock
    for (const auto& target : macroLFO.getTargets())
    {
        if (auto* param = apvts.getParameter(target.paramID))
        {
            const float base  = param->getValue();
            const float mod   = lfoVal * target.depth;
            param->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, base + mod));
        }
    }
}

void DustCrateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Update BPM from DAW playhead
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                currentBPM.store(*bpm);
    }

    // MIDI learn CC handling
    midiLearn.processMidiBuffer(midi, apvts);

    {
        const juce::ScopedLock sl(synthLock);
        if (! pendingMidi.isEmpty())
        {
            for (const auto meta : pendingMidi)
                midi.addEvent(meta.getMessage(), meta.samplePosition);
            pendingMidi.clear();
        }
        updateVoiceParameters();
        synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
    }

    // LFO tick (per block)
    macroLFO.tick(buffer.getNumSamples());

    characterProcessor.setDrift    (*apvts.getRawParameterValue("driftAmount"));
    characterProcessor.setVHS      (*apvts.getRawParameterValue("vhsAmount"));
    characterProcessor.setCassette (*apvts.getRawParameterValue("cassetteAmount"));
    characterProcessor.setNoise    (*apvts.getRawParameterValue("noiseLevel"));
    characterProcessor.processBlock(buffer);

    if (onAudioBlock)
        onAudioBlock(buffer.getReadPointer(0), buffer.getNumSamples());
}

//==============================================================================
bool DustCrateAudioProcessor::selectSample(const juce::String& filePath, int rootNote)
{
    const juce::File file(filePath);
    const int nv = synth.getNumVoices();
    std::vector<std::unique_ptr<juce::AudioFormatReader>> readers((size_t)nv);
    bool any = false;
    for (int i = 0; i < nv; ++i)
    { readers[(size_t)i].reset(formatManager.createReaderFor(file)); if (readers[(size_t)i]) any = true; }
    if (! any) return false;
    const juce::ScopedLock sl(synthLock);
    for (int i = 0; i < nv; ++i)
        if (auto* v = dynamic_cast<SampleVoice*>(synth.getVoice(i)))
            if (readers[(size_t)i]) v->setReader(readers[(size_t)i].release(), rootNote);
    return true;
}

void DustCrateAudioProcessor::triggerSample(const juce::String& path, int note, float vel)
{
    if (! selectSample(path, note)) return;
    const juce::ScopedLock sl(synthLock);
    pendingMidi.addEvent(juce::MidiMessage::noteOn(1, note, vel), 0);
}

void DustCrateAudioProcessor::stopAllVoices()
{
    const juce::ScopedLock sl(synthLock);
    synth.allNotesOff(0, true);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
DustCrateAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>("attack",         "Attack",          0.001f,  2.0f,   0.01f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("decay",          "Decay",           0.001f,  2.0f,   0.10f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("sustain",        "Sustain",         0.0f,    1.0f,   0.80f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("release",        "Release",         0.001f,  4.0f,   0.20f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("filterCutoff",   "Filter Cutoff",   20.0f,   20000.0f, 8000.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("filterRes",      "Filter Resonance",0.1f,   10.0f,  1.0f));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("filterType",    "Filter Type",
        juce::StringArray{"Lowpass","Highpass","Bandpass"}, 0));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("pitchSemitones", "Pitch",          -24.0f,  24.0f,  0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("noiseLevel",     "Noise Level",     0.0f,   1.0f,   0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("driftAmount",    "Drift",           0.0f,   1.0f,   0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("vhsAmount",      "VHS",             0.0f,   1.0f,   0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("cassetteAmount", "Cassette",        0.0f,   1.0f,   0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("lfoRate",        "LFO Rate",        0.01f,  20.0f,  1.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("lfoDepth",       "LFO Depth",       0.0f,   1.0f,   0.0f));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("lfoShape",      "LFO Shape",
        juce::StringArray{"Sine","Triangle","Saw","Square"}, 0));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("lfoTarget",     "LFO Target",
        juce::StringArray{"None","Noise","Drift","VHS","Cassette"}, 0));
    return {p.begin(), p.end()};
}

void DustCrateAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    juce::ValueTree extra("Extra");
    midiLearn.saveToState(extra);
    state.addChild(extra, -1, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void DustCrateAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml == nullptr) return;
    auto tree = juce::ValueTree::fromXml(*xml);
    if (tree.isValid() && tree.hasType(apvts.state.getType()))
    {
        const auto extra = tree.getChildWithName("Extra");
        if (extra.isValid()) midiLearn.loadFromState(extra);
        tree.removeAllChildren(nullptr); // strip Extra before restoring
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorEditor* DustCrateAudioProcessor::createEditor()
{ return new DustCrateAudioProcessorEditor(*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{ return new DustCrateAudioProcessor(); }
