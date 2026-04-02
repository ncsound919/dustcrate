#include "SampleVoice.h"

SampleVoice::SampleVoice()
{
    prepareFilter(44100.0);
    recomputeIncrements();
}

void SampleVoice::prepareFilter(double sr)
{
    cachedSR = sr;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sr;
    spec.maximumBlockSize = 2048;
    spec.numChannels      = 2;
    filter.prepare(spec);
    filter.reset();
    using T = juce::dsp::StateVariableTPTFilterType;
    filter.setType(filterTypeIdx == 1 ? T::highpass : filterTypeIdx == 2 ? T::bandpass : T::lowpass);
    filter.setCutoffFrequency(filterCutoff);
    filter.setResonance(filterRes);
}

void SampleVoice::setCurrentPlaybackSampleRate(double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);
    if (newRate > 0.0 && newRate != cachedSR)
        prepareFilter(newRate);
    recomputeIncrements();
}

bool SampleVoice::canPlaySound(juce::SynthesiserSound* s)
{ return dynamic_cast<SampleSound*>(s) != nullptr; }

void SampleVoice::recomputeIncrements()
{
    const double sr = (getSampleRate() > 0.0 ? getSampleRate() : 44100.0);
    attackInc  = (adsrAttack  > 0.0001f) ? float(1.0 / (adsrAttack  * sr)) : 1.0f;
    decayInc   = (adsrDecay   > 0.0001f) ? float(1.0 / (adsrDecay   * sr)) : 1.0f;
    // FIX: release increment is 1/(release * sr) — not sustain-scaled at definition time
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
            // FIX: release decrements by fixed inc, not sustain-scaled — correct tail regardless of sustain level
            adsrLevel -= releaseInc;
            if (adsrLevel <= 0.0f) { adsrLevel = 0.0f; adsrStage = AdsrStage::Idle; }
            break;
        default:
            adsrLevel = 0.0f;
            break;
    }
    return juce::jlimit(0.0f, 1.0f, adsrLevel);
}

void SampleVoice::setReader(juce::AudioFormatReader* newReader, int rootNote)
{
    rootMidiNote = rootNote;
    if (newReader == nullptr) return;
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset(new juce::AudioFormatReaderSource(newReader, true));
    transportSource.setSource(readerSource.get(), 32768, nullptr, newReader->sampleRate);
}

void SampleVoice::setADSR(float a, float d, float s, float r)
{
    adsrAttack  = juce::jmax(0.001f, a);
    adsrDecay   = juce::jmax(0.001f, d);
    adsrSustain = juce::jlimit(0.0f, 1.0f, s);
    adsrRelease = juce::jmax(0.001f, r);
    recomputeIncrements();
}

void SampleVoice::setFilter(float cutoff, float resonance, bool /*hp*/)
{
    filterCutoff  = juce::jlimit(20.0f, 20000.0f, cutoff);
    filterRes     = juce::jmax(0.1f, resonance);
    // FIX: do NOT overwrite filterTypeIdx here — setFilterType() is called
    // first by updateVoiceParameters() and already handles LP/HP/BP correctly.
    // The 'hp' bool cannot represent bandpass, so using it to override
    // filterTypeIdx would clear any bandpass selection.
    filter.setCutoffFrequency(filterCutoff);
    filter.setResonance(filterRes);
    using T = juce::dsp::StateVariableTPTFilter<float>::Type;
    filter.setType(filterTypeIdx == 1 ? T::highpass
                 : filterTypeIdx == 2 ? T::bandpass
                 :                      T::lowpass);
}

void SampleVoice::setFilterType(int idx)
{
    filterTypeIdx = idx;
    using T = juce::dsp::StateVariableTPTFilterType;
    filter.setType(idx == 1 ? T::highpass : idx == 2 ? T::bandpass : T::lowpass);
}

void SampleVoice::setPitchShift(float semitones)
{ pitchRatio = std::pow(2.0, (double)semitones / 12.0); }

void SampleVoice::setDriftRatio(float ratio)
{ driftDepth = (double) juce::jlimit(0.0f, 1.0f, ratio); }

void SampleVoice::startNote(int midiNote, float velocity,
                              juce::SynthesiserSound*, int)
{
    velocityGain = velocity;
    const double noteDiff = midiNote - rootMidiNote;
    cachedNoteRatio = std::pow(2.0, noteDiff / 12.0);

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
        transportSource.stop();
        adsrStage = AdsrStage::Idle;
        adsrLevel = 0.0f;
        clearCurrentNote();
    }
}

void SampleVoice::renderNextBlock(juce::AudioBuffer<float>& output,
                                   int startSample, int numSamples)
{
    if (adsrStage == AdsrStage::Idle)
    {
        // FIX: don't call clearCurrentNote() inside renderNextBlock;
        // only stop transport here — clearCurrentNote is called in stopNote.
        if (transportSource.isPlaying()) transportSource.stop();
        return;
    }

    const int numCh = juce::jmax(1, output.getNumChannels());
    tempBuffer.setSize(numCh, numSamples, false, false, true);
    tempBuffer.clear();

    if (driftDepth > 0.001)
    {
        driftPhase += driftRate * (float)numSamples;
        if (driftPhase > juce::MathConstants<float>::twoPi)
            driftPhase -= juce::MathConstants<float>::twoPi;
        const double lfoVal   = std::sin((double)driftPhase);
        const double cents    = lfoVal * driftDepth * 8.0;
        const double driftMul = std::pow(2.0, cents / 1200.0);
        resamplingSource.setResamplingRatio(cachedNoteRatio * pitchRatio * driftMul);
    }
    else
    {
        resamplingSource.setResamplingRatio(cachedNoteRatio * pitchRatio);
    }

    juce::AudioSourceChannelInfo info(&tempBuffer, 0, numSamples);
    resamplingSource.getNextAudioBlock(info);

    // ADSR — sample-accurate, per-channel
    {
        // FIX: advance ADSR once and apply to all channels (was only writing to ch0 and ch1 separately with same env)
        const int ch0 = 0;
        const int ch1 = (tempBuffer.getNumChannels() > 1) ? 1 : 0;
        float* L = tempBuffer.getWritePointer(ch0);
        float* R = tempBuffer.getWritePointer(ch1);
        for (int n = 0; n < numSamples; ++n)
        {
            const float env = nextAdsrSample();
            L[n] *= env;
            if (ch1 != ch0) R[n] *= env;
        }
    }

    // Filter (block-level, post-ADSR)
    if (numCh == 2)
    {
        juce::dsp::AudioBlock<float> block(tempBuffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        filter.process(ctx);
    }
    else
    {
        // Mono path: wrap single-channel block
        juce::dsp::AudioBlock<float> block(tempBuffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        filter.process(ctx);
    }

    for (int ch = 0; ch < output.getNumChannels(); ++ch)
        output.addFrom(ch, startSample,
                       tempBuffer, ch % tempBuffer.getNumChannels(),
                       0, numSamples, velocityGain);

    // If ADSR reaches Idle mid-block (during the sample loop above), the second
    // check below will catch it and call clearCurrentNote(). This is the correct
    // path: clearCurrentNote() must be called from within renderNextBlock(), not
    // from stopNote(), to satisfy JUCE's voice-management invariants.
    if (adsrStage == AdsrStage::Idle)
    {
        transportSource.stop();
        clearCurrentNote();
    }
}
