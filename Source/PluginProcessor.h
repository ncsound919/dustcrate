#pragma once
#include <JuceHeader.h>
#include "SampleLibrary.h"
#include "SampleVoice.h"

class DustCrateAudioProcessor : public juce::AudioProcessor
{
public:
    DustCrateAudioProcessor();
    ~DustCrateAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // --- Public API for editor ---
    SampleLibrary& getSampleLibrary() { return sampleLibrary; }
    void triggerSample(const juce::String& filePath, int midiNote, float velocity);
    void stopAllVoices();

    // ADSR + Filter params (exposed for UI binding)
    juce::AudioProcessorValueTreeState apvts;

private:
    SampleLibrary sampleLibrary;
    juce::Synthesiser synth;
    juce::AudioFormatManager formatManager;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DustCrateAudioProcessor)
};
