#pragma once
#include <JuceHeader.h>

//==============================================================================
struct SampleSound : public juce::SynthesiserSound
{
    bool appliesToNote    (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

//==============================================================================
class SampleVoice : public juce::SynthesiserVoice
{
public:
    SampleVoice();

    bool canPlaySound (juce::SynthesiserSound*) override;
    void startNote    (int midiNote, float velocity,
                       juce::SynthesiserSound*, int pitchWheel) override;
    void stopNote     (float velocity, bool allowTailOff)        override;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    void renderNextBlock  (juce::AudioBuffer<float>&,
                           int startSample, int numSamples)      override;

    // Called once from Synthesiser::setCurrentPlaybackSampleRate
    void setCurrentPlaybackSampleRate (double newRate) override;

    void setReader     (juce::AudioFormatReader*, int rootNote);
    void setADSR       (float a, float d, float s, float r);
    void setFilter     (float cutoff, float resonance, bool hp);
    void setFilterType (int typeIndex);   // 0=LP 1=HP 2=BP
    void setPitchShift (float semitones);
    void setDriftRatio (float ratio);

private:
    //=== Sample playback ===================================================
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource                     transportSource;
    juce::ResamplingAudioSource                    resamplingSource { &transportSource, false, 2 };
    int    rootMidiNote   { 60 };
    double cachedNoteRatio{ 1.0 };
    float  velocityGain   { 1.0f };

    //=== ADSR ==============================================================
    enum class AdsrStage { Idle, Attack, Decay, Sustain, Release };
    AdsrStage adsrStage   { AdsrStage::Idle };
    float adsrLevel       { 0.0f };
    float adsrAttack      { 0.01f };
    float adsrDecay       { 0.10f };
    float adsrSustain     { 0.80f };
    float adsrRelease     { 0.20f };
    float attackInc       { 0.0f };
    float decayInc        { 0.0f };
    float releaseInc      { 0.0f };
    void  recomputeIncrements();
    float nextAdsrSample();

    //=== Filter ============================================================
    juce::dsp::StateVariableTPTFilter<float> filter;
    float filterCutoff { 8000.0f };
    float filterRes    { 1.0f };
    int   filterTypeIdx { 0 };
    double cachedSR    { 44100.0 };
    void  prepareFilter (double sr);

    //=== Pitch / drift =====================================================
    double pitchRatio  { 1.0 };
    double driftDepth  { 0.0 };
    float  driftPhase  { 0.0f };
    float  driftRate   { 0.0f };

    //=== Temp buffer =======================================================
    juce::AudioBuffer<float> tempBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleVoice)
};
