#pragma once
#include <JuceHeader.h>

//==============================================================================
// CharacterProcessor
//
// Post-voice stereo bus processing that applies Drift, VHS, and Cassette
// coloration independently on a 0–1 macro scale, plus a white/pink noise
// layer controlled by a noise-level macro.
//
// Call prepare() once, then processBlock() each audio callback.
// Set macros at any time (thread-safe reads via atomics).
//==============================================================================
class CharacterProcessor
{
public:
    CharacterProcessor();

    void prepare(double sampleRate, int maxBlockSize);
    void processBlock(juce::AudioBuffer<float>& buffer);

    // Macro setters (call from audio thread after reading APVTS params)
    void setDrift    (float amount);   // 0–1
    void setVHS      (float amount);   // 0–1
    void setCassette (float amount);   // 0–1
    void setNoise    (float amount);   // 0–1

private:
    //=== Shared state ======================================================
    double sr { 44100.0 };
    int    maxBlock { 512 };

    float driftAmount    { 0.0f };
    float vhsAmount      { 0.0f };
    float cassetteAmount { 0.0f };
    float noiseAmount    { 0.0f };

    //=== Drift (slow random pitch wander + stereo width shimmer) ===========
    // Two independent slow LFOs, one per channel
    float driftPhaseL { 0.0f }, driftPhaseR { 0.100f };
    float driftRateL  { 0.0f }, driftRateR  { 0.0f };
    float driftTargetRateL { 0.41f }, driftTargetRateR { 0.37f }; // Hz
    // Delay-line-based pitch smear (short circular buffer per channel)
    static constexpr int kDriftDelayLen = 2048;
    float driftDelayL[kDriftDelayLen] {};
    float driftDelayR[kDriftDelayLen] {};
    int   driftWriteL { 0 }, driftWriteR { 0 };

    //=== VHS (wow/flutter + sample-rate reduction) =========================
    float vhsWowPhase     { 0.0f };
    float vhsFlutterPhase { 0.0f };
    // Sample-rate reduction accumulator
    float srReduceAccL { 0.0f }, srReduceAccR { 0.0f };
    float srReduceHoldL { 0.0f }, srReduceHoldR { 0.0f };
    // Gentle low-pass for the video-bandwidth roll-off
    juce::dsp::StateVariableTPTFilter<float> vhsFilter;

    //=== Cassette (tape saturation + HF roll-off + wow) ====================
    float cassetteWowPhase { 0.0f };
    juce::dsp::StateVariableTPTFilter<float> cassHFFilter;
    juce::dsp::StateVariableTPTFilter<float> cassLFBoost; // very gentle

    //=== Noise layer =======================================================
    juce::Random rng;
    // One-pole LP to tint white noise slightly pink
    float noiseLPL { 0.0f }, noiseLPR { 0.0f };

    //=== Helpers ===========================================================
    float readDelay(const float* buf, int writeHead, float delaySamples) const;
    static float softClip(float x, float drive);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CharacterProcessor)
};
