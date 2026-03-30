#pragma once
#include <JuceHeader.h>

//==============================================================================
// CharacterProcessor — stereo (or mono-safe) post-voice bus processor.
// Handles Drift, VHS, Cassette, and Noise macros independently (0–1 range).
//
// Thread model: all setters called from audio thread only.
// prepare() called from prepareToPlay() only.
//==============================================================================
class CharacterProcessor
{
public:
    CharacterProcessor();

    void prepare     (double sampleRate, int maxBlockSize);
    void processBlock(juce::AudioBuffer<float>& buffer);

    // Macro setters — call from audio thread
    void setDrift    (float amount);   // 0–1
    void setVHS      (float amount);   // 0–1
    void setCassette (float amount);   // 0–1
    void setNoise    (float amount);   // 0–1

private:
    double sr       { 44100.0 };
    int    maxBlock { 512 };

    float driftAmount    { 0.0f };
    float vhsAmount      { 0.0f };
    float cassetteAmount { 0.0f };
    float noiseAmount    { 0.0f };

    //=== Drift =============================================================
    float driftPhaseL { 0.0f }, driftPhaseR { 0.100f };
    float driftRateL  { 0.0f }, driftRateR  { 0.0f };
    static constexpr float driftTargetRateL { 0.41f };
    static constexpr float driftTargetRateR { 0.37f };
    static constexpr int   kDriftDelayLen   { 2048 };
    float driftDelayL[kDriftDelayLen] {};
    float driftDelayR[kDriftDelayLen] {};
    int   driftWriteL { 0 }, driftWriteR { 0 };

    //=== VHS ===============================================================
    float vhsWowPhase     { 0.0f };
    float vhsFlutterPhase { 0.0f };
    float srReduceAccL    { 0.0f }, srReduceAccR  { 0.0f };
    float srReduceHoldL   { 0.0f }, srReduceHoldR { 0.0f };
    juce::dsp::StateVariableTPTFilter<float> vhsFilter;

    //=== Cassette ==========================================================
    float cassetteWowPhase { 0.0f };
    juce::dsp::StateVariableTPTFilter<float> cassHFFilter;
    juce::dsp::StateVariableTPTFilter<float> cassLFBoost;

    //=== Noise =============================================================
    juce::Random rng;
    float noiseLPL { 0.0f }, noiseLPR { 0.0f };

    //=== Helpers ===========================================================
    void  prepareFilters (const juce::dsp::ProcessSpec&);
    float readDelay      (const float* buf, int writeHead, float delaySamples) const;
    static float softClip(float x, float drive);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CharacterProcessor)
};
