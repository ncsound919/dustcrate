#include "SampleVoice.h"

SampleVoice::SampleVoice()
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = 44100.0;
    spec.maximumBlockSize = 512;
    spec.numChannels      = 2;
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency(8000.0f);
    filter.setResonance(1.0f);
    recomputeIncrements();
}

bool SampleVoice::canPlaySound(juce::SynthesiserSound* s)
{ return dynamic_cast<SampleSound*>(s) != nullptr; }

//==============================================================================
void SampleVoice::recomputeIncrements()
{
    const double sr = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;
    attackInc  = (adsrAttack  > 0.0001f) ? float(1.0 / (adsrAttack  * sr)) : 1.0f;
    decayInc   = (adsrDecay   > 0.0001f) ? float(1.0 / (adsrDecay   * sr)) : 1.0f;
    releaseInc = (adsrRelease > 0.0001f) ? float(1.0 / (adsrRelease * sr)) : 1.0f;
}

float SampleVoice::nextAdsrSample()
{
    switch (adsrStage)
    {
        case AdsrStage::Attack:
            adsrLevel += attackInc;
            if (adsrLevel >= 1.0f) { adsrLevel = 1.0f; adsrStage = AdsrStage::Decay; }
            break;
        case AdsrStage::Decay:
            adsrLevel -= decayInc * (1.0f - adsrSustain);
            if (adsrLevel <= adsrSustain) { adsrLevel = adsrSustain; adsrStage = AdsrStage::Sustain; }
            break;
        case AdsrStage::Sustain:
            adsrLevel = adsrSustain;
            break;
        case AdsrStage::Release:
            adsrLevel -= releaseInc * adsrSustain;
            if (adsrLevel <= 0.0f) { adsrLevel = 0.0f; adsrStage = AdsrStage::Idle; }
            break;
        default:
            adsrLevel = 0.0f;
            break;
    }
    return adsrLevel;
}

//==============================================================================
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
    adsrAttack  = juce::jmax(0.001f, a);
    adsrDecay   = juce::jmax(0.001f, d);
    adsrSustain = juce::jlimit(0.0f, 1.0f, s);
    adsrRelease = juce::jmax(0.001f, r);
    recomputeIncrements();
}

void SampleVoice::setFilter(float cutoff, float resonance, bool hp)
{
    filterCutoff  = juce::jlimit(20.0f, 20000.0f, cutoff);
    filterRes     = juce::jmax(0.1f, resonance);
    filterTypeIdx = hp ? 1 : 0;
    filter.setCutoffFrequency(filterCutoff);
    filter.setResonance(filterRes);
    filter.setType(hp ? juce::dsp::StateVariableTPTFilterType::highpass
                      : juce::dsp::StateVariableTPTFilterType::lowpass);
}

void SampleVoice::setFilterType(int idx)
{
    filterTypeIdx = idx;
    using T = juce::dsp::StateVariableTPTFilterType;
    filter.setType(idx == 1 ? T::highpass : idx == 2 ? T::bandpass : T::lowpass);
}

void SampleVoice::setPitchShift(float semitones)
{
    pitchRatio = std::pow(2.0, (double)semitones / 12.0);
}

void SampleVoice::setDriftRatio(float ratio)
{
    driftDepth = (double) juce::jlimit(0.0f, 1.0f, ratio);
}

//==============================================================================
void SampleVoice::startNote(int midiNote, float velocity,
                              juce::SynthesiserSound*, int)
{
    velocityGain = velocity;
    const double noteDiff = midiNote - rootMidiNote;
    cachedNoteRatio = std::pow(2.0, noteDiff / 12.0);

    // Randomise drift LFO rate slightly per voice (0.25–0.75 Hz)
    const double sr = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;
    driftRate  = float(juce::MathConstants<double>::twoPi *
                       (0.25 + (double)juce::Random::getSystemRandom().nextFloat() * 0.5) / sr);
    driftPhase = juce::Random::getSystemRandom().nextFloat() * juce::MathConstants<float>::twoPi;

    resamplingSource.setResamplingRatio(cachedNoteRatio * pitchRatio);
    transportSource.setPosition(0.0);
    transportSource.start();

    recomputeIncrements();
    adsrStage = AdsrStage::Attack;
    adsrLevel = 0.0f;
}

void SampleVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        adsrStage = AdsrStage::Release;
    else
    {
        adsrStage = AdsrStage::Idle;
        adsrLevel = 0.0f;
        transportSource.stop();
        clearCurrentNote();
    }
}

//==============================================================================
void SampleVoice::renderNextBlock(juce::AudioBuffer<float>& output,
                                   int startSample, int numSamples)
{
    if (adsrStage == AdsrStage::Idle && !transportSource.isPlaying())
    { clearCurrentNote(); return; }

    tempBuffer.setSize(2, numSamples, false, false, true);
    tempBuffer.clear();

    // Update resampling ratio with drift LFO
    if (driftDepth > 0.001)
    {
        driftPhase += driftRate * (float)numSamples;
        if (driftPhase > juce::MathConstants<float>::twoPi)
            driftPhase -= juce::MathConstants<float>::twoPi;
        const double lfoVal  = std::sin((double)driftPhase);
        const double cents   = lfoVal * driftDepth * 8.0;  // max ±8 cents drift
        const double driftMul = std::pow(2.0, cents / 1200.0);
        resamplingSource.setResamplingRatio(cachedNoteRatio * pitchRatio * driftMul);
    }
    else
    {
        resamplingSource.setResamplingRatio(cachedNoteRatio * pitchRatio);
    }

    // Pull samples
    juce::AudioSourceChannelInfo info(&tempBuffer, 0, numSamples);
    resamplingSource.getNextAudioBlock(info);

    // Apply hand-rolled ADSR sample-by-sample (zipper-free)
    {
        float* L = tempBuffer.getWritePointer(0);
        float* R = tempBuffer.getWritePointer(tempBuffer.getNumChannels() > 1 ? 1 : 0);
        for (int n = 0; n < numSamples; ++n)
        {
            const float env = nextAdsrSample();
            L[n] *= env;
            R[n] *= env;
        }
    }

    // Apply filter (block-level, post-envelope)
    juce::dsp::AudioBlock<float> block(tempBuffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    filter.process(ctx);

    // Mix into output
    for (int ch = 0; ch < output.getNumChannels(); ++ch)
        output.addFrom(ch, startSample,
                       tempBuffer, ch % tempBuffer.getNumChannels(),
                       0, numSamples, velocityGain);

    // Auto-clear when ADSR idle and transport stopped
    if (adsrStage == AdsrStage::Idle)
    { transportSource.stop(); clearCurrentNote(); }
}
