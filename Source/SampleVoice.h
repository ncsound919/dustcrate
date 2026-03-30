#pragma once
#include <JuceHeader.h>

// A minimal SynthesiserSound that accepts all notes/channels
struct SampleSound : public juce::SynthesiserSound
{
    bool appliesToNote(int) override    { return true; }
    bool appliesToChannel(int) override { return true; }
};

class SampleVoice : public juce::SynthesiserVoice
{
public:
    SampleVoice();

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound*, int pitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioBuffer<float>& buffer,
                         int startSample, int numSamples) override;

    // Called by processor before triggering note
    void setReader(juce::AudioFormatReader* newReader, int rootNote);
    void setADSR(float attack, float decay, float sustain, float release);
    void setFilter(float cutoff, float resonance, bool isHighpass);
    void setPitchShift(float semitones);
    // Called per-block so drift continuously modulates pitch while playing
    void setDriftRatio(float ratio);

private:
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resamplingSource { &transportSource, false, 2 };

    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;

    juce::dsp::StateVariableTPTFilter<float> filter;
    double pitchRatio   { 1.0 };
    double driftRatio   { 1.0 };
    double cachedNoteRatio { 1.0 }; // set in startNote, reused per-block
    float  velocityGain { 1.0f };
    int    rootMidiNote { 60 };

    juce::AudioBuffer<float> tempBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleVoice)
};
