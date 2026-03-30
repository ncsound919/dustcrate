#pragma once
#include <JuceHeader.h>

//==============================================================================
// MacroLFO
//
// A free-running or tempo-sync LFO that modulates any subset of the four
// CHARACTER parameters: Noise, Drift, VHS, Cassette.
//
// - Shape: Sine | Triangle | Saw | Square
// - Rate:  0.01 – 20 Hz (free) or note division (tempo-sync)
// - Depth: 0 – 1 per target
// - Offset: bipolar centre-point for each target
//
// Call prepare() once. Call tick() each processBlock; it returns a 0–1 value
// that the processor adds (scaled) on top of the current APVTS param value.
//==============================================================================
class MacroLFO
{
public:
    enum class Shape { Sine, Triangle, Saw, Square };

    struct Target
    {
        juce::String paramID;   // APVTS parameter to modulate
        float        depth { 0.0f }; // 0–1 bipolar depth
    };

    MacroLFO();

    void prepare  (double sampleRate);
    void setRate  (float hz);          // free-running rate in Hz
    void setShape (Shape s)            { shape = s; }
    void setDepth (float d)            { depth = juce::jlimit(0.0f, 1.0f, d); }
    void setTempoSync (bool on, double bpm, float divisionBeats);

    // Advance one block, return current LFO value (-1 to +1)
    float tick (int numSamples);

    // List of APVTS param IDs this LFO modulates
    void setTargets (const juce::Array<Target>& t) { targets = t; }
    const juce::Array<Target>& getTargets() const  { return targets; }

    float getPhase() const { return phase / juce::MathConstants<float>::twoPi; } // 0–1
    float getLastValue() const { return lastValue; }

private:
    double sr      { 44100.0 };
    float  phase   { 0.0f };
    float  inc     { 0.0f }; // radians per sample
    Shape  shape   { Shape::Sine };
    float  depth   { 0.5f };
    float  lastValue { 0.0f };

    bool   tempoSync  { false };
    double bpm        { 120.0 };
    float  divBeats   { 1.0f };

    juce::Array<Target> targets;

    float computeShape() const;
    void  recomputeInc();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroLFO)
};
