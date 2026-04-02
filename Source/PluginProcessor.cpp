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

    // Point SampleLibrary at the bundled assets folder
    const juce::File exe = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    const juce::File assets = exe.getSiblingFile("assets").getChildFile("samples");
    sampleLibrary.setAssetsRoot(assets);

    // Load factory pack if present
    const juce::File factoryJSON = exe.getSiblingFile("assets")
                                      .getChildFile("factory_pack.json");
    if (factoryJSON.existsAsFile())
        sampleLibrary.loadPackFromJSON(factoryJSON);
}

DustCrateAudioProcessor::~DustCrateAudioProcessor() {}

void DustCrateAudioProcessor::prepareToPlay(double sr, int blockSize)
{
    currentSampleRate = sr;
    synth.setCurrentPlaybackSampleRate(sr); // propagates to each SampleVoice via setCurrentPlaybackSampleRate override
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
            v->setFilterType(fType);
            v->setFilter(cutoff, res, fType == 1);
            v->setPitchShift(pitch);
            v->setDriftRatio(drift);
        }
}

void DustCrateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    // Defensive guards — hosts should never violate these, but be explicit
    jassert(buffer.getNumChannels() > 0);
    jassert(buffer.getNumSamples() > 0);
    if (buffer.getNumSamples() <= 0) return;
    buffer.clear();

    // BPM from DAW
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                currentBPM.store(*bpm);

    // MIDI learn — consume CC before rendering
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

    // FIX: Tick LFO exactly once per block (applyMacroLFO() removed — it
    // called tick(0) AND processBlock called tick(N), double-advancing).
    // LFO modulation goes directly to CharacterProcessor — never calls
    // setValueNotifyingHost from the audio thread (illegal in JUCE).
    const float lfoVal = macroLFO.tick(buffer.getNumSamples()); // -1..+1

    // Read base param values
    float noise    = *apvts.getRawParameterValue("noiseLevel");
    float drift    = *apvts.getRawParameterValue("driftAmount");
    float vhs      = *apvts.getRawParameterValue("vhsAmount");
    float cassette = *apvts.getRawParameterValue("cassetteAmount");

    // Apply LFO modulation to the correct target (audio-thread safe: just floats)
    const int lfoTarget = (int)*apvts.getRawParameterValue("lfoTarget");
    const float lfoDepth = *apvts.getRawParameterValue("lfoDepth");
    const float mod = lfoVal * lfoDepth * 0.5f; // scale to ±0.5 max
    switch (lfoTarget)
    {
        case 1: noise    = juce::jlimit(0.0f, 1.0f, noise    + mod); break;
        case 2: drift    = juce::jlimit(0.0f, 1.0f, drift    + mod); break;
        case 3: vhs      = juce::jlimit(0.0f, 1.0f, vhs      + mod); break;
        case 4: cassette = juce::jlimit(0.0f, 1.0f, cassette + mod); break;
        default: break;
    }

    // Update LFO rate/shape from params (audio thread, safe setters)
    macroLFO.setRate  (*apvts.getRawParameterValue("lfoRate"));
    macroLFO.setShape (static_cast<MacroLFO::Shape>(
                           (int)*apvts.getRawParameterValue("lfoShape")));
    macroLFO.setDepth (*apvts.getRawParameterValue("lfoDepth"));

    characterProcessor.setDrift    (drift);
    characterProcessor.setVHS      (vhs);
    characterProcessor.setCassette (cassette);
    characterProcessor.setNoise    (noise);
    characterProcessor.processBlock(buffer);

    if (onAudioBlock)
        onAudioBlock(buffer.getReadPointer(0), buffer.getNumSamples());
}

//==============================================================================
bool DustCrateAudioProcessor::selectSample(const juce::String& filePath, int rootNote)
{
    const juce::File file(filePath);
    if (! file.existsAsFile()) return false; // FIX: early-out before allocating readers

    const int nv = synth.getNumVoices();
    std::vector<std::unique_ptr<juce::AudioFormatReader>> readers((size_t)nv);
    bool any = false;
    for (int i = 0; i < nv; ++i)
    {
        readers[(size_t)i].reset(formatManager.createReaderFor(file));
        if (readers[(size_t)i]) any = true;
    }
    if (! any) return false;

    const juce::ScopedLock sl(synthLock);
    for (int i = 0; i < nv; ++i)
        if (auto* v = dynamic_cast<SampleVoice*>(synth.getVoice(i)))
            if (readers[(size_t)i])
                v->setReader(readers[(size_t)i].release(), rootNote);
    return true;
}

void DustCrateAudioProcessor::triggerSample(const juce::String& path, int note, float vel)
{
    if (! selectSample(path, note)) return;
    const juce::ScopedLock sl(synthLock);
    // FIX: add a corresponding noteOff immediately after noteOn so the voice
    // enters release rather than holding indefinitely (one-shot behaviour).
    // The note is queued at sample position 0; noteOff at position 1 so the
    // ADSR starts attack before the release is triggered.
    pendingMidi.addEvent(juce::MidiMessage::noteOn (1, note, vel), 0);
    pendingMidi.addEvent(juce::MidiMessage::noteOff(1, note, 0.0f), 1);
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
    p.push_back(std::make_unique<juce::AudioParameterFloat>("attack",         "Attack",           0.001f,  2.0f,    0.01f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("decay",          "Decay",            0.001f,  2.0f,    0.10f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("sustain",        "Sustain",          0.0f,    1.0f,    0.80f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("release",        "Release",          0.001f,  4.0f,    0.20f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("filterCutoff",   "Filter Cutoff",    20.0f,   20000.0f,8000.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("filterRes",      "Filter Resonance", 0.1f,    10.0f,   1.0f));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("filterType",    "Filter Type",
        juce::StringArray{"Lowpass","Highpass","Bandpass"}, 0));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("pitchSemitones", "Pitch",           -24.0f,  24.0f,   0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("noiseLevel",     "Noise Level",      0.0f,    1.0f,    0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("driftAmount",    "Drift",            0.0f,    1.0f,    0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("vhsAmount",      "VHS",              0.0f,    1.0f,    0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("cassetteAmount", "Cassette",         0.0f,    1.0f,    0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("lfoRate",        "LFO Rate",         0.01f,   20.0f,   1.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("lfoDepth",       "LFO Depth",        0.0f,    1.0f,    0.0f));
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
    if (! tree.isValid() || ! tree.hasType(apvts.state.getType())) return;
    const auto extra = tree.getChildWithName("Extra");
    if (extra.isValid()) midiLearn.loadFromState(extra);
    // Remove the Extra child we added in getStateInformation().
    // If the preset was saved without an "Extra" child (older preset format),
    // getChildWithName() returns an invalid ValueTree and removeChild() is a
    // safe no-op — no crash, no data loss.
    tree.removeChild(tree.getChildWithName("Extra"), nullptr);
    apvts.replaceState(tree);
}

juce::AudioProcessorEditor* DustCrateAudioProcessor::createEditor()
{ return new DustCrateAudioProcessorEditor(*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{ return new DustCrateAudioProcessor(); }
