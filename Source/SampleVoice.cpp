#include "SampleVoice.h"

SampleVoice::SampleVoice()
{
    adsrParams = { 0.01f, 0.1f, 0.8f, 0.2f };
    adsr.setParameters(adsrParams);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = 44100.0;
    spec.maximumBlockSize = 512;
    spec.numChannels = 2;
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency(8000.0f);
    filter.setResonance(1.0f);
}

bool SampleVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<SampleSound*>(sound) != nullptr;
}

void SampleVoice::setReader(juce::AudioFormatReader* newReader, int rootNote)
{
    rootMidiNote = rootNote;
    if (newReader == nullptr) return;

    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset(new juce::AudioFormatReaderSource(newReader, true));
    transportSource.setSource(readerSource.get(), 32768, nullptr,
                               newReader->sampleRate);
}

void SampleVoice::setADSR(float a, float d, float s, float r)
{
    adsrParams = { a, d, s, r };
    adsr.setParameters(adsrParams);
}

void SampleVoice::setFilter(float cutoff, float resonance, bool isHighpass)
{
    filter.setCutoffFrequency(cutoff);
    filter.setResonance(resonance);
    filter.setType(isHighpass
        ? juce::dsp::StateVariableTPTFilterType::highpass
        : juce::dsp::StateVariableTPTFilterType::lowpass);
}

void SampleVoice::setPitchShift(float semitones)
{
    pitchRatio = std::pow(2.0, semitones / 12.0);
}

void SampleVoice::setDriftRatio(float ratio)
{
    driftRatio = static_cast<double>(ratio);
}

void SampleVoice::startNote(int midiNoteNumber, float velocity,
                             juce::SynthesiserSound*, int)
{
    velocityGain = velocity;

    // Cache the note ratio; reused every block to avoid repeated std::pow
    const double noteDiff = midiNoteNumber - rootMidiNote;
    cachedNoteRatio = std::pow(2.0, noteDiff / 12.0);
    resamplingSource.setResamplingRatio(cachedNoteRatio * pitchRatio * driftRatio);

    transportSource.setPosition(0.0);
    transportSource.start();
    adsr.setSampleRate(getSampleRate());
    adsr.setParameters(adsrParams);
    adsr.noteOn();
}

void SampleVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        adsr.noteOff();
    else
    {
        adsr.reset();
        transportSource.stop();
        clearCurrentNote();
    }
}

void SampleVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                   int startSample, int numSamples)
{
    if (!transportSource.isPlaying() && !adsr.isActive())
    {
        clearCurrentNote();
        return;
    }

    // Keep resampling ratio updated so drift modulates pitch while playing
    if (getCurrentlyPlayingNote() >= 0)
        resamplingSource.setResamplingRatio(cachedNoteRatio * pitchRatio * driftRatio);

    tempBuffer.setSize(2, numSamples, false, false, true);
    tempBuffer.clear();

    juce::AudioSourceChannelInfo info(&tempBuffer, 0, numSamples);
    resamplingSource.getNextAudioBlock(info);

    // Apply ADSR
    adsr.applyEnvelopeToBuffer(tempBuffer, 0, numSamples);

    // Apply filter
    juce::dsp::AudioBlock<float> block(tempBuffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    filter.process(ctx);

    // Mix into output with velocity gain
    for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        outputBuffer.addFrom(ch, startSample,
                             tempBuffer, ch % tempBuffer.getNumChannels(),
                             0, numSamples, velocityGain);

    if (!adsr.isActive())
    {
        transportSource.stop();
        clearCurrentNote();
    }
}
