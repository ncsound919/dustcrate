#pragma once
#include <JuceHeader.h>
#include "SampleLibrary.h"
#include "SampleVoice.h"
#include "CharacterProcessor.h"

class DustCrateAudioProcessor : public juce::AudioProcessor
{
public:
    DustCrateAudioProcessor();
    ~DustCrateAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #if ! JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;
        return true;
    }
   #endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    SampleLibrary& getSampleLibrary() { return sampleLibrary; }

    void triggerSample(const juce::String& filePath, int midiNote, float velocity);
    void stopAllVoices();

    // For WaveformDisplay: latest rendered L channel block
    std::function<void(const float*, int)> onAudioBlock;

    juce::AudioProcessorValueTreeState apvts;

private:
    SampleLibrary sampleLibrary;
    juce::Synthesiser synth;
    juce::AudioFormatManager formatManager;
    CharacterProcessor characterProcessor;

    juce::CriticalSection synthLock;
    juce::MidiBuffer pendingMidi;

    double currentSampleRate { 44100.0 };

    // Returns true if at least one reader was successfully created
    bool selectSample(const juce::String& filePath, int rootNote);

    // Per-block: push APVTS param values into all voices
    void updateVoiceParameters();

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DustCrateAudioProcessor)
};
