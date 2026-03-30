#include "CharacterProcessor.h"

CharacterProcessor::CharacterProcessor()
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = 44100.0;
    spec.maximumBlockSize = 512;
    spec.numChannels      = 2;
    prepareFilters(spec);
}

void CharacterProcessor::prepareFilters(const juce::dsp::ProcessSpec& spec)
{
    vhsFilter.prepare(spec);
    vhsFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    vhsFilter.setCutoffFrequency(9000.0f);
    vhsFilter.setResonance(0.5f);

    cassHFFilter.prepare(spec);
    cassHFFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    cassHFFilter.setCutoffFrequency(12000.0f);
    cassHFFilter.setResonance(0.6f);

    cassLFBoost.prepare(spec);
    cassLFBoost.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    cassLFBoost.setCutoffFrequency(200.0f);
    cassLFBoost.setResonance(0.7f);
}

void CharacterProcessor::prepare(double sampleRate, int maxBlockSize)
{
    // FIX: guard against degenerate values
    jassert(sampleRate > 0.0);
    jassert(maxBlockSize > 0);
    if (sampleRate <= 0.0 || maxBlockSize <= 0) return;

    sr       = sampleRate;
    maxBlock = maxBlockSize;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) maxBlockSize;
    spec.numChannels      = 2;
    prepareFilters(spec);

    // Recompute drift LFO increments at the new sample rate
    driftRateL = (float)(juce::MathConstants<double>::twoPi * driftTargetRateL / sampleRate);
    driftRateR = (float)(juce::MathConstants<double>::twoPi * driftTargetRateR / sampleRate);

    // Reset stateful elements so reentrant prepare() is clean
    srReduceAccL  = srReduceAccR  = 0.0f;
    srReduceHoldL = srReduceHoldR = 0.0f;
    vhsWowPhase = vhsFlutterPhase = cassetteWowPhase = 0.0f;
    noiseLPL = noiseLPR = 0.0f;
    juce::zeromem(driftDelayL, sizeof(driftDelayL));
    juce::zeromem(driftDelayR, sizeof(driftDelayR));
    driftWriteL = driftWriteR = 0;
}

//==============================================================================
void CharacterProcessor::setDrift    (float v) { driftAmount    = juce::jlimit(0.0f,1.0f,v); }
void CharacterProcessor::setVHS      (float v) { vhsAmount      = juce::jlimit(0.0f,1.0f,v); }
void CharacterProcessor::setCassette (float v) { cassetteAmount = juce::jlimit(0.0f,1.0f,v); }
void CharacterProcessor::setNoise    (float v) { noiseAmount    = juce::jlimit(0.0f,1.0f,v); }

//==============================================================================
float CharacterProcessor::readDelay(const float* buf, int writeHead, float delaySamples) const
{
    // FIX: ensure rd is positive before truncation so modulo always gives valid index
    float rd = (float)writeHead - delaySamples;
    // Bring into positive range before floor
    while (rd < 0.0f) rd += (float)kDriftDelayLen;
    int ri   = (int)rd;
    float frac = rd - (float)ri;
    ri   = ri % kDriftDelayLen;
    int ri1  = (ri + 1) % kDriftDelayLen;
    return buf[ri] * (1.0f - frac) + buf[ri1] * frac;
}

float CharacterProcessor::softClip(float x, float drive)
{
    x *= (1.0f + drive * 2.0f);
    return x / (1.0f + std::abs(x));
}

//==============================================================================
void CharacterProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // FIX: guard zero-sample and mono; for mono, mirror processing to ch0 only
    if (numSamples <= 0) return;

    // FIX: mono-safe — work on ch0 only, ch1 mirrors ch0 if stereo
    const bool stereo = (numChannels >= 2);

    float* L = buffer.getWritePointer(0);
    float* R = stereo ? buffer.getWritePointer(1) : buffer.getWritePointer(0);

    const float twoPi = juce::MathConstants<float>::twoPi;

    // FIX: max delay clamped to kDriftDelayLen - 2 to guarantee readDelay never overflows
    const float maxDelaySmp = juce::jmin(
        driftAmount * 2.0f * 0.001f * (float)sr,
        (float)(kDriftDelayLen - 2));

    for (int n = 0; n < numSamples; ++n)
    {
        float sL = L[n];
        float sR = stereo ? R[n] : sL;

        //=== DRIFT =========================================================
        if (driftAmount > 0.001f)
        {
            driftDelayL[driftWriteL] = sL;
            driftDelayR[driftWriteR] = sR;

            const float lfoL = 0.5f + 0.5f * std::sin(driftPhaseL);
            const float lfoR = 0.5f + 0.5f * std::sin(driftPhaseR);
            const float delayL = driftAmount * lfoL * maxDelaySmp + 1.0f;
            const float delayR = driftAmount * lfoR * maxDelaySmp + 1.0f;

            const float dw = driftAmount * 0.1f;
            sL = readDelay(driftDelayL, driftWriteL, delayL) * (1.0f - dw) + sL * dw;
            sR = readDelay(driftDelayR, driftWriteR, delayR) * (1.0f - dw) + sR * dw;

            driftPhaseL += driftRateL;
            driftPhaseR += driftRateR;
            if (driftPhaseL > twoPi) driftPhaseL -= twoPi;
            if (driftPhaseR > twoPi) driftPhaseR -= twoPi;

            driftWriteL = (driftWriteL + 1) % kDriftDelayLen;
            driftWriteR = (driftWriteR + 1) % kDriftDelayLen;
        }

        //=== VHS ===========================================================
        if (vhsAmount > 0.001f)
        {
            const float wowRate     = (float)(twoPi * 0.8  / sr);
            const float flutterRate = (float)(twoPi * 8.0  / sr);
            vhsWowPhase     += wowRate;
            vhsFlutterPhase += flutterRate;
            if (vhsWowPhase     > twoPi) vhsWowPhase     -= twoPi;
            if (vhsFlutterPhase > twoPi) vhsFlutterPhase -= twoPi;

            const float modDepth = vhsAmount * 0.004f;
            const float totalMod = 1.0f + std::sin(vhsWowPhase) * modDepth
                                        + std::sin(vhsFlutterPhase) * modDepth * 0.3f;
            sL *= totalMod;
            sR *= totalMod;

            const float srRatio = 1.0f + vhsAmount * 7.0f;
            srReduceAccL += 1.0f;
            srReduceAccR += 1.0f;
            if (srReduceAccL >= srRatio) { srReduceHoldL = sL; srReduceAccL -= srRatio; }
            if (srReduceAccR >= srRatio) { srReduceHoldR = sR; srReduceAccR -= srRatio; }
            const float vh = vhsAmount * 0.5f;
            sL = sL * (1.0f - vh) + srReduceHoldL * vh;
            sR = sR * (1.0f - vh) + srReduceHoldR * vh;
        }

        //=== CASSETTE ======================================================
        if (cassetteAmount > 0.001f)
        {
            const float cWowRate = (float)(twoPi * 1.2 / sr);
            cassetteWowPhase += cWowRate;
            if (cassetteWowPhase > twoPi) cassetteWowPhase -= twoPi;
            const float cWow = 1.0f + std::sin(cassetteWowPhase) * cassetteAmount * 0.003f;
            sL *= cWow;  sR *= cWow;
            sL = softClip(sL, cassetteAmount * 0.5f);
            sR = softClip(sR, cassetteAmount * 0.5f);
        }

        L[n] = sL;
        if (stereo) R[n] = sR;
    }

    //=== Post-loop filter passes (block-level) =============================
    // FIX: only apply filters when stereo; for mono, wrap single-channel block
    if (vhsAmount > 0.001f)
    {
        const float vhsCutoff = juce::jmax(100.0f, 9000.0f - vhsAmount * 6000.0f);
        vhsFilter.setCutoffFrequency(vhsCutoff);
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        vhsFilter.process(ctx);
    }

    if (cassetteAmount > 0.001f)
    {
        const float cassHFCutoff = juce::jmax(100.0f, 14000.0f - cassetteAmount * 6000.0f);
        cassHFFilter.setCutoffFrequency(cassHFCutoff);
        {
            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> ctx(block);
            cassHFFilter.process(ctx);
        }
        // Parallel LF warmth blend
        juce::AudioBuffer<float> lfBuf(numChannels, numSamples);
        lfBuf.makeCopyOf(buffer);
        {
            juce::dsp::AudioBlock<float> lfBlock(lfBuf);
            juce::dsp::ProcessContextReplacing<float> lfCtx(lfBlock);
            cassLFBoost.process(lfCtx);
        }
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addFrom(ch, 0, lfBuf, ch, 0, numSamples, cassetteAmount * 0.06f);
    }

    //=== Noise layer =======================================================
    if (noiseAmount > 0.001f)
    {
        const float noiseGain = noiseAmount * 0.08f;
        const float lp = 0.92f;
        for (int n = 0; n < numSamples; ++n)
        {
            float wL = rng.nextFloat() * 2.0f - 1.0f;
            float wR = stereo ? (rng.nextFloat() * 2.0f - 1.0f) : wL;
            noiseLPL = noiseLPL * lp + wL * (1.0f - lp);
            noiseLPR = noiseLPR * lp + wR * (1.0f - lp);
            L[n] += (wL * 0.4f + noiseLPL * 0.6f) * noiseGain;
            if (stereo) R[n] += (wR * 0.4f + noiseLPR * 0.6f) * noiseGain;
        }
    }
}
