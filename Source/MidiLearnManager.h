#pragma once
#include <JuceHeader.h>

//==============================================================================
// MidiLearnManager
//
// Enables right-click MIDI learn on any juce::Slider. When a slider is right-
// clicked, it enters "learn" mode; the next CC message received maps that CC
// to the slider's APVTS parameter. The CC map is persisted inside the APVTS
// state XML so it survives save/load.
//
// Usage:
//   1. Create one instance (owned by the editor).
//   2. Call registerSlider() for every Slider + paramID pair.
//   3. Call processMidiBuffer() from processBlock() to drive CC remapping.
//   4. Override mouseDown on sliders via attachRightClickMenu().
//==============================================================================
class MidiLearnManager
{
public:
    explicit MidiLearnManager(juce::AudioProcessorValueTreeState& apvts);

    // Register a slider so it can be MIDI-learned
    void registerSlider(juce::Slider* slider, const juce::String& paramID);

    // Call from processBlock to apply incoming CC -> param values
    // Returns true if any parameter was updated
    bool processMidiBuffer(const juce::MidiBuffer& midi,
                           juce::AudioProcessorValueTreeState& apvts);

    // Show a right-click context menu for a slider (call from mouseDown)
    void showContextMenu(juce::Slider* slider);

    // Notify manager that a CC was received (called from processMidiBuffer)
    void notifyCC(int ccNumber);

    // Persist / restore CC map to/from ValueTree extra state
    void saveToState(juce::ValueTree& extraState) const;
    void loadFromState(const juce::ValueTree& extraState);

private:
    juce::AudioProcessorValueTreeState& apvts;

    struct SliderInfo
    {
        juce::Slider* slider { nullptr };
        juce::String  paramID;
        int           learnedCC { -1 };
    };

    juce::Array<SliderInfo> sliders;
    juce::Slider* pendingLearnSlider { nullptr }; // waiting for next CC

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLearnManager)
};
