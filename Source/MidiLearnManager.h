#pragma once
#include <JuceHeader.h>

//==============================================================================
// MidiLearnManager
// FIX: pendingLearnSlider is now cleared to nullptr when the slider
// is unregistered, preventing a dangling-pointer dereference if the
// editor is destroyed while learn is pending.
//==============================================================================
class MidiLearnManager
{
public:
    explicit MidiLearnManager(juce::AudioProcessorValueTreeState& apvts);

    void registerSlider   (juce::Slider*, const juce::String& paramID);
    void unregisterSlider (juce::Slider*); // call from slider destructor

    bool processMidiBuffer (const juce::MidiBuffer& midi,
                            juce::AudioProcessorValueTreeState& apvts);

    void showContextMenu (juce::Slider*);
    void saveToState     (juce::ValueTree& extraState) const;
    void loadFromState   (const juce::ValueTree& extraState);

private:
    juce::AudioProcessorValueTreeState& apvts;

    struct SliderInfo
    {
        juce::Slider* slider  { nullptr };
        juce::String  paramID;
        int           learnedCC { -1 };
    };

    juce::Array<SliderInfo> sliders;
    // FIX: use ComponentSafePointer to guard against dangling learn pointer
    juce::Component::SafePointer<juce::Slider> pendingLearnSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLearnManager)
};
