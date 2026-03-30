#include "CharacterProcessor.h"

CharacterProcessor::CharacterProcessor()
{
    // Prepare with placeholder spec; proper values set in prepare()
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = 44100.0;
    spec.maximumBlockSize = 512;
    spec.numChannels      = 2;

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
    sr = sampleRate;
    maxBlock = maxBlockSize;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) maxBlockSize;
    spec.numChannels      = 2;

    vhsFilter.prepare(spec);
    vhsFilter.setCutoffFrequency(9000.0f);

    cassHFFilter.prepare(spec);
    cassHFFilter.setCutoffFrequency(12000.0f);

    cassLFBoost.prepare(spec);
    cassLFBoost.setCutoffFrequency(200.0f);

    // Compute drift LFO rates in phase-per-sample
    driftRateL = (float)(juce::MathConstants<double>::twoPi * driftTargetRateL / sampleRate);
    driftRateR = (float)(juce::MathConstants<double>::twoPi * driftTargetRateR / sampleRate);
}

//==============================================================================
void CharacterProcessor::setDrift    (float v) { driftAmount    = v; }
void CharacterProcessor::setVHS      (float v) { vhsAmount      = v; }
void CharacterProcessor::setCassette (float v) { cassetteAmount = v; }
void CharacterProcessor::setNoise    (float v) { noiseAmount    = v; }

//==============================================================================
float CharacterProcessor::readDelay(const float* buf,
                                     int writeHead,
                                     float delaySamples) const
{
    const float rd = (float) writeHead - delaySamples;
    int ri = (int) rd;
    const float frac = rd - (float) ri;
    ri = (ri % kDriftDelayLen + kDriftDelayLen) % kDriftDelayLen;
    const int ri1 = (ri + 1) % kDriftDelayLen;
    return buf[ri] * (1.0f - frac) + buf[ri1] * frac;
}

float CharacterProcessor::softClip(float x, float drive)
{
    x *= (1.0f + drive * 2.0f);
    return x / (1.0f + std::abs(x)); // cheap tanh-like
}

//==============================================================================
void CharacterProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numChannels < 2) return;

    float* L = buffer.getWritePointer(0);
    float* R = buffer.getWritePointer(1);

    const float twoPi = juce::MathConstants<float>::twoPi;

    for (int n = 0; n < numSamples; ++n)
    {
        float sL = L[n];
        float sR = R[n];

        //=== DRIFT =========================================================
        if (driftAmount > 0.001f)
        {
            // Write into delay lines
            driftDelayL[driftWriteL] = sL;
            driftDelayR[driftWriteR] = sR;

            // LFO-modulated delay read (very short: 0–2 ms)
            const float maxDelayMs  = 2.0f;
            const float maxDelaySmp = maxDelayMs * 0.001f * (float) sr;

            const float lfoL = 0.5f + 0.5f * std::sin(driftPhaseL); // 0–1
            const float lfoR = 0.5f + 0.5f * std::sin(driftPhaseR);

            const float delayL = driftAmount * lfoL * maxDelaySmp + 1.0f;
            const float delayR = driftAmount * lfoR * maxDelaySmp + 1.0f;

            sL = readDelay(driftDelayL, driftWriteL, delayL) * (1.0f - driftAmount * 0.1f)
               + sL * driftAmount * 0.1f;
            sR = readDelay(driftDelayR, driftWriteR, delayR) * (1.0f - driftAmount * 0.1f)
               + sR * driftAmount * 0.1f;

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
            // Wow (0.5–2 Hz)
            const float wowRate = (float)(twoPi * 0.8 / sr);
            vhsWowPhase += wowRate;
            if (vhsWowPhase > twoPi) vhsWowPhase -= twoPi;

            // Flutter (5–12 Hz)
            const float flutterRate = (float)(twoPi * 8.0 / sr);
            vhsFlutterPhase += flutterRate;
            if (vhsFlutterPhase > twoPi) vhsFlutterPhase -= twoPi;

            const float modDepth = vhsAmount * 0.004f;
            const float wowMod   = std::sin(vhsWowPhase)     * modDepth;
            const float fltMod   = std::sin(vhsFlutterPhase) * modDepth * 0.3f;
            const float totalMod = 1.0f + wowMod + fltMod;

            // Amplitude modulation (VHS level instability)
            sL *= totalMod;
            sR *= totalMod;

            // Sample-rate reduction (bit/sample degradation)
            const float srRatio = 1.0f + vhsAmount * 7.0f; // 1–8x decimation
            srReduceAccL += 1.0f;
            srReduceAccR += 1.0f;
            if (srReduceAccL >= srRatio) { srReduceHoldL = sL; srReduceAccL -= srRatio; }
            if (srReduceAccR >= srRatio) { srReduceHoldR = sR; srReduceAccR -= srRatio; }
            sL = sL * (1.0f - vhsAmount * 0.5f) + srReduceHoldL * vhsAmount * 0.5f;
            sR = sR * (1.0f - vhsAmount * 0.5f) + srReduceHoldR * vhsAmount * 0.5f;
        }

        //=== CASSETTE ======================================================
        if (cassetteAmount > 0.001f)
        {
            // Gentle wow
            const float cWowRate = (float)(twoPi * 1.2 / sr);
            cassetteWowPhase += cWowRate;
            if (cassetteWowPhase > twoPi) cassetteWowPhase -= twoPi;
            const float cWow = 1.0f + std::sin(cassetteWowPhase) * cassetteAmount * 0.003f;
            sL *= cWow;
            sR *= cWow;

            // Soft tape saturation
            sL = softClip(sL, cassetteAmount * 0.5f);
            sR = softClip(sR, cassetteAmount * 0.5f);
        }

        L[n] = sL;
        R[n] = sR;
    }

    //=== VHS bandwidth roll-off (sample-domain, applied to whole block) ====
    if (vhsAmount > 0.001f)
    {
        const float vhsCutoff = 9000.0f - vhsAmount * 6000.0f; // 3k–9k
        vhsFilter.setCutoffFrequency(juce::jmax(100.0f, vhsCutoff));
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        vhsFilter.process(ctx);
    }

    //=== Cassette HF roll-off + subtle LF warmth ===========================
    if (cassetteAmount > 0.001f)
    {
        const float cassHFCutoff = 14000.0f - cassetteAmount * 6000.0f; // 8k–14k
        cassHFFilter.setCutoffFrequency(juce::jmax(100.0f, cassHFCutoff));
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        cassHFFilter.process(ctx);
        // Mix in a touch of low warmth
        // (gentle parallel LF boost: blend ~5% of LF-filtered signal back in)
        juce::AudioBuffer<float> lfBuf(buffer.getNumChannels(), numSamples);
        lfBuf.makeCopyOf(buffer);
        juce::dsp::AudioBlock<float> lfBlock(lfBuf);
        juce::dsp::ProcessContextReplacing<float> lfCtx(lfBlock);
        cassLFBoost.process(lfCtx);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addFrom(ch, 0, lfBuf, ch, 0, numSamples, cassetteAmount * 0.06f);
    }

    //=== Noise layer =======================================================
    if (noiseAmount > 0.001f)
    {
        const float noiseGain = noiseAmount * 0.08f; // keeps it tasteful
        for (int n = 0; n < numSamples; ++n)
        {
            // White noise
            float wL = rng.nextFloat() * 2.0f - 1.0f;
            float wR = rng.nextFloat() * 2.0f - 1.0f;

            // One-pole LP to tint toward pink
            const float lp = 0.92f;
            noiseLPL = noiseLPL * lp + wL * (1.0f - lp);
            noiseLPR = noiseLPR * lp + wR * (1.0f - lp);

            // Blend white + pink
            L[n] += (wL * 0.4f + noiseLPL * 0.6f) * noiseGain;
            R[n] += (wR * 0.4f + noiseLPR * 0.6f) * noiseGain;
        }
    }
}
