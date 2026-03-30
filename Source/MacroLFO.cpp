#include "MacroLFO.h"

MacroLFO::MacroLFO() { recomputeInc(); }

void MacroLFO::prepare(double sampleRate)
{
    sr = sampleRate;
    recomputeInc();
}

void MacroLFO::setRate(float hz)
{
    tempoSync = false;
    // FIX: guard against zero / negative sr
    if (sr > 0.0)
        inc = (float)(juce::MathConstants<double>::twoPi * juce::jmax(0.001f, hz) / sr);
}

void MacroLFO::setTempoSync(bool on, double newBpm, float divisionBeats)
{
    tempoSync   = on;
    bpm         = newBpm;
    divBeats    = juce::jmax(0.001f, divisionBeats);
    recomputeInc();
}

void MacroLFO::recomputeInc()
{
    if (sr <= 0.0) { inc = 0.0f; return; }
    if (tempoSync && bpm > 0.0 && divBeats > 0.0f)
    {
        const double hz = bpm / (60.0 * divBeats);
        inc = (float)(juce::MathConstants<double>::twoPi * hz / sr);
    }
    else
    {
        // Default 1 Hz
        inc = (float)(juce::MathConstants<double>::twoPi * 1.0 / sr);
    }
}

float MacroLFO::computeShape() const
{
    const float p     = phase;
    const float pi    = juce::MathConstants<float>::pi;
    const float twoPi = juce::MathConstants<float>::twoPi;
    switch (shape)
    {
        case Shape::Sine:     return std::sin(p);
        case Shape::Triangle:
        {
            const float norm = p / twoPi;
            return (norm < 0.5f) ? (norm * 4.0f - 1.0f) : (3.0f - norm * 4.0f);
        }
        case Shape::Saw:      return (p / pi) - 1.0f;
        case Shape::Square:   return (p < pi) ? 1.0f : -1.0f;
        default:              return std::sin(p);
    }
}

// FIX: tick() now advances by numSamples (was always called twice per block —
// once from applyMacroLFO(tick(0)) and once from processBlock(tick(N))).
// applyMacroLFO is removed; processBlock calls tick() exactly once.
float MacroLFO::tick(int numSamples)
{
    if (numSamples > 0)
    {
        phase += inc * (float)numSamples;
        while (phase > juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;
    }
    lastValue = computeShape() * depth;
    return lastValue;
}
