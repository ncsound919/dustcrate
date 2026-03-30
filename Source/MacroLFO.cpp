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
    inc = (float)(juce::MathConstants<double>::twoPi * hz / sr);
}

void MacroLFO::setTempoSync(bool on, double newBpm, float divisionBeats)
{
    tempoSync   = on;
    bpm         = newBpm;
    divBeats    = divisionBeats;
    if (on) recomputeInc();
}

void MacroLFO::recomputeInc()
{
    if (tempoSync && bpm > 0.0 && divBeats > 0.0f)
    {
        const double hz = bpm / (60.0 * divBeats);
        inc = (float)(juce::MathConstants<double>::twoPi * hz / sr);
    }
    else
    {
        inc = (float)(juce::MathConstants<double>::twoPi * 1.0 / sr); // 1 Hz default
    }
}

float MacroLFO::computeShape() const
{
    const float p = phase;
    const float pi = juce::MathConstants<float>::pi;
    const float twoPi = juce::MathConstants<float>::twoPi;
    switch (shape)
    {
        case Shape::Sine:     return std::sin(p);
        case Shape::Triangle:
        {
            const float norm = p / twoPi; // 0–1
            return (norm < 0.5f) ? (norm * 4.0f - 1.0f) : (3.0f - norm * 4.0f);
        }
        case Shape::Saw:      return (p / pi) - 1.0f; // -1 to +1
        case Shape::Square:   return (p < pi) ? 1.0f : -1.0f;
        default:              return std::sin(p);
    }
}

float MacroLFO::tick(int numSamples)
{
    phase += inc * (float)numSamples;
    while (phase > juce::MathConstants<float>::twoPi)
        phase -= juce::MathConstants<float>::twoPi;
    lastValue = computeShape() * depth;
    return lastValue;
}
