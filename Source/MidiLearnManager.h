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

    // Toggle global MIDI-learn arm — next incoming CC binds to any active slider
    void toggleLearnMode ();
    bool isLearnMode     () const { return learnMode; }

    // Clear all CC assignments for all registered sliders
    void clearAll ();

    // Flush queued CC changes to the APVTS — call this on the message thread
    // (e.g. from a juce::Timer or juce::AsyncUpdater in the editor).
    void flushCcQueue (juce::AudioProcessorValueTreeState& ap);

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
    bool learnMode { false };

    // Protects 'sliders' and 'pendingLearnSlider' which are accessed from both
    // the audio thread (processMidiBuffer) and the message thread (register,
    // unregister, context menu callback).  The audio thread uses ScopedTryLock
    // so it never blocks; if it cannot acquire the lock it skips that buffer.
    // mutable so saveToState() (const) can still lock.
    mutable juce::CriticalSection mappingLock;

    // Lock-free queue for audio-thread CC → message-thread param update.
    // Audio thread writes; message thread (flushCcQueue) reads and applies.
    static constexpr int kCcQueueSize = 64;
    struct CcEvent { int cc { -1 }; float value { 0.0f }; };
    juce::AbstractFifo           ccFifo { kCcQueueSize };
    std::array<CcEvent, kCcQueueSize> ccQueue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLearnManager)
};
