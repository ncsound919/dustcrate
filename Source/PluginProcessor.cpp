#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <BinaryData.h>

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

    // Load factory pack from bundled binary data
    sampleLibrary.loadPackFromBinaryData(BinaryData::factory_pack_json,
                                         BinaryData::factory_pack_jsonSize);
}

DustCrateAudioProcessor::~DustCrateAudioProcessor() {}

//==============================================================================
void DustCrateAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    currentSampleRate = sampleRate;
    synth.setCurrentPlaybackSampleRate(sampleRate);

    // Reset LFO phases so behaviour is deterministic after transport stop/start
    driftPhase           = 0.0f;
    cassetteWowPhase     = 0.0f;
    cassetteFlutterPhase = 0.0f;
}

void DustCrateAudioProcessor::releaseResources() {}

void DustCrateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // --- Read APVTS parameters ---
    const float attack      = *apvts.getRawParameterValue("attack");
    const float decay       = *apvts.getRawParameterValue("decay");
    const float sustain     = *apvts.getRawParameterValue("sustain");
    const float release     = *apvts.getRawParameterValue("release");
    const float cutoff      = *apvts.getRawParameterValue("filterCutoff");
    const float res         = *apvts.getRawParameterValue("filterRes");
    const int   filterTypeIdx = (int)*apvts.getRawParameterValue("filterType"); // 0=LP 1=HP
    const float pitchSemi   = *apvts.getRawParameterValue("pitchSemitones");
    const float noiseLevel  = *apvts.getRawParameterValue("noiseLevel");
    const float driftAmt    = *apvts.getRawParameterValue("driftAmount");
    const float vhsAmt      = *apvts.getRawParameterValue("vhsAmount");
    const float cassAmt     = *apvts.getRawParameterValue("cassetteAmount");

    // --- Drift LFO (advances at block rate) ---
    static constexpr float kDriftFreq  = 0.4f;   // analog wander rate, Hz
    static constexpr float kDriftDepth = 0.015f;  // ±1.5% speed ≈ ±0.26 semitones
    driftPhase += kDriftFreq * (float)numSamples / (float)currentSampleRate;
    if (driftPhase > 1.0f) driftPhase -= 1.0f;
    const float driftRatio = 1.0f + driftAmt * kDriftDepth
                             * std::sin(juce::MathConstants<float>::twoPi * driftPhase);

    // --- Apply per-block params to all voices ---
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SampleVoice*>(synth.getVoice(i)))
        {
            voice->setADSR(attack, decay, sustain, release);
            voice->setFilter(cutoff, res, filterTypeIdx == 1);
            voice->setPitchShift(pitchSemi);
            voice->setDriftRatio(driftRatio);
        }
    }

    // --- Merge any UI-generated preview MIDI into the host MIDI stream ---
    if (!pendingMidi.isEmpty())
    {
        for (const auto meta : pendingMidi)
            midiMessages.addEvent(meta.getMessage(), meta.samplePosition);
        pendingMidi.clear();
    }

    buffer.clear();
    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);

    // --- Noise layer (white noise, scaled by noiseLevel) ---
    // 0.12f keeps noise subtle even at noiseLevel=1 relative to full-scale audio
    static constexpr float kNoiseScale = 0.12f;
    if (noiseLevel > 0.001f)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int s = 0; s < numSamples; ++s)
                data[s] += noiseLevel * kNoiseScale * (rng.nextFloat() * 2.0f - 1.0f);
        }
    }

    // --- Character: VHS (tape saturation via tanh waveshaping) ---
    // drive of 5× at full VHS; makeup compensates for tanh headroom reduction
    static constexpr float kVhsMaxDrive  = 4.0f;
    static constexpr float kVhsMakeupDiv = 0.4f;
    if (vhsAmt > 0.001f)
    {
        const float drive  = 1.0f + vhsAmt * kVhsMaxDrive;
        const float makeup = 1.0f / (1.0f + vhsAmt * kVhsMakeupDiv);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int s = 0; s < numSamples; ++s)
                data[s] = std::tanh(data[s] * drive) * makeup;
        }
    }

    // --- Character: Cassette (wow/flutter amplitude modulation + soft clip) ---
    static constexpr float kWowFreq      = 1.2f;   // Hz – slow wow
    static constexpr float kFlutterFreq  = 9.0f;   // Hz – fast flutter
    static constexpr float kWowDepth     = 0.008f;  // 0.8% amplitude swing
    static constexpr float kFlutterDepth = 0.003f;  // 0.3% amplitude swing
    static constexpr float kCassMaxDrive = 1.5f;
    if (cassAmt > 0.001f)
    {
        const float clipDrive  = 1.0f + cassAmt * kCassMaxDrive;
        const float wowInc     = kWowFreq    / (float)currentSampleRate;
        const float flutterInc = kFlutterFreq / (float)currentSampleRate;

        for (int s = 0; s < numSamples; ++s)
        {
            cassetteWowPhase    += wowInc;
            cassetteFlutterPhase += flutterInc;
            if (cassetteWowPhase    > 1.0f) cassetteWowPhase    -= 1.0f;
            if (cassetteFlutterPhase > 1.0f) cassetteFlutterPhase -= 1.0f;

            const float modDepth = cassAmt
                * (kWowDepth    * std::sin(juce::MathConstants<float>::twoPi * cassetteWowPhase)
                 + kFlutterDepth * std::sin(juce::MathConstants<float>::twoPi * cassetteFlutterPhase));
            const float ampMod = 1.0f - modDepth;

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                float& sample = buffer.getWritePointer(ch)[s];
                sample = std::tanh(sample * clipDrive) * ampMod;
            }
        }
    }
}

//==============================================================================
void DustCrateAudioProcessor::selectSample(const juce::String& filePath, int rootNote)
{
    // Create an independent reader per voice so all 16 can play simultaneously.
    const juce::File file(filePath);
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SampleVoice*>(synth.getVoice(i)))
        {
            std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
            if (reader != nullptr)
                voice->setReader(reader.release(), rootNote);
        }
    }
}

void DustCrateAudioProcessor::triggerSample(const juce::String& filePath,
                                            int midiNote, float velocity)
{
    selectSample(filePath, midiNote);

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
