#pragma once
#include <JuceHeader.h>
#include "SampleLibrary.h"
#include "SampleVoice.h"
#include "CharacterProcessor.h"
#include "MacroLFO.h"
#include "MidiLearnManager.h"

class DustCrateAudioProcessor : public juce::AudioProcessor
{
public:
    DustCrateAudioProcessor();
    ~DustCrateAudioProcessor() override;

    void prepareToPlay  (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

   #if ! JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor()              const override { return true; }
    const juce::String getName() const override  { return JucePlugin_Name; }
    bool acceptsMidi()            const override { return true; }
    bool producesMidi()           const override { return false; }
    bool isMidiEffect()           const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int  getNumPrograms()               override { return 1; }
    int  getCurrentProgram()            override { return 0; }
    void setCurrentProgram(int)         override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&)          override;
    void setStateInformation (const void*, int sizeInBytes) override;

    SampleLibrary& getSampleLibrary() { return sampleLibrary; }

    void triggerSample (const juce::String& filePath, int midiNote, float velocity);
    void stopAllVoices ();

    std::function<void(const float*, int)> onAudioBlock;

    juce::AudioProcessorValueTreeState apvts;
    MidiLearnManager                   midiLearn { apvts };
    MacroLFO                           macroLFO;

    // Tempo info (set from processBlock via PlayHead)
    std::atomic<double> currentBPM { 120.0 };

private:
    SampleLibrary      sampleLibrary;
    juce::Synthesiser  synth;
    juce::AudioFormatManager formatManager;
    CharacterProcessor characterProcessor;
    juce::CriticalSection    synthLock;
    juce::MidiBuffer         pendingMidi;
    double currentSampleRate { 44100.0 };

    bool   selectSample          (const juce::String& filePath, int rootNote);
    void   updateVoiceParameters ();
    void   applyMacroLFO         ();

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DustCrateAudioProcessor)
};
