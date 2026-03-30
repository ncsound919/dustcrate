#pragma once
#include <JuceHeader.h>

//==============================================================================
// MIDI Output panel: broadcast notes to the MPC Sample (or any MIDI device)
// over USB-C.  Includes:
//   - Device selector ComboBox (refreshes available MIDI outputs)
//   - MIDI channel selector (1-16)
//   - Chord mode (root + selectable voicing)
//   - Scale lock (quantise all notes to a chosen root + scale)
//   - Octave transpose (+/-3)
class MidiOutputPanel : public juce::Component,
                        public juce::Timer
{
public:
    MidiOutputPanel();
    ~MidiOutputPanel() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

    // Called by the keyboard component with raw note-on/off
    void sendNoteOn  (int midiNote, float velocity);
    void sendNoteOff (int midiNote);
    void sendAllNotesOff ();

    // Chord + scale processing
    // Returns the set of actual MIDI notes to send (may be chords)
    juce::Array<int> processNote (int rawNote);

    // Timer: refresh device list every 2 seconds
    void timerCallback() override;

    // Available scale names
    static juce::StringArray scaleNames();
    // Scale intervals (semitones from root) for named scales
    static juce::Array<int> scaleIntervals (const juce::String& scaleName);

private:
    // MIDI output device
    std::unique_ptr<juce::MidiOutput> midiOut;
    juce::StringArray deviceNames;
    int  selectedDevice { -1 };

    // UI
    juce::Label     deviceLabel   { {}, "MIDI OUT" };
    juce::ComboBox  deviceCombo;
    juce::Label     channelLabel  { {}, "CH" };
    juce::ComboBox  channelCombo;
    juce::Label     octaveLabel   { {}, "OCT" };
    juce::TextButton octDownBtn   { "-" };
    juce::TextButton octUpBtn     { "+" };
    juce::Label     octaveDisplay { {}, "0" };
    juce::ToggleButton chordBtn   { "CHORD" };
    juce::ComboBox  chordCombo;   // voicing type
    juce::ToggleButton scaleBtn   { "SCALE" };
    juce::ComboBox  scaleRootCombo;
    juce::ComboBox  scaleTypeCombo;

    int  midiChannel { 1 };
    int  octaveShift { 0 };
    bool chordMode   { false };
    bool scaleMode   { false };

    void refreshDevices();
    void openDevice (int index);
    void setupUI();

    // Snap note to scale; returns closest scale note
    int  snapToScale (int note) const;

    // Build chord notes from a root
    juce::Array<int> buildChord (int root) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiOutputPanel)
};
