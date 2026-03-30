#include "MidiOutputPanel.h"

//==============================================================================
juce::StringArray MidiOutputPanel::scaleNames()
{
    return { "Chromatic", "Major", "Natural Minor", "Pentatonic Major",
             "Pentatonic Minor", "Blues", "Dorian", "Mixolydian",
             "Lydian", "Phrygian", "Harmonic Minor", "Whole Tone" };
}

juce::Array<int> MidiOutputPanel::scaleIntervals (const juce::String& name)
{
    if (name == "Major")            return { 0,2,4,5,7,9,11 };
    if (name == "Natural Minor")    return { 0,2,3,5,7,8,10 };
    if (name == "Pentatonic Major") return { 0,2,4,7,9 };
    if (name == "Pentatonic Minor") return { 0,3,5,7,10 };
    if (name == "Blues")            return { 0,3,5,6,7,10 };
    if (name == "Dorian")           return { 0,2,3,5,7,9,10 };
    if (name == "Mixolydian")       return { 0,2,4,5,7,9,10 };
    if (name == "Lydian")           return { 0,2,4,6,7,9,11 };
    if (name == "Phrygian")         return { 0,1,3,5,7,8,10 };
    if (name == "Harmonic Minor")   return { 0,2,3,5,7,8,11 };
    if (name == "Whole Tone")       return { 0,2,4,6,8,10 };
    // Chromatic: all 12
    juce::Array<int> c;
    for (int i = 0; i < 12; ++i) c.add(i);
    return c;
}

//==============================================================================
MidiOutputPanel::MidiOutputPanel()
{
    setupUI();
    refreshDevices();
    startTimer (2000);
}

MidiOutputPanel::~MidiOutputPanel()
{
    stopTimer();
    if (midiOut) midiOut->sendMessageNow (juce::MidiMessage::allNotesOff (midiChannel));
}

void MidiOutputPanel::setupUI()
{
    // Device
    addAndMakeVisible (deviceLabel);
    addAndMakeVisible (deviceCombo);
    deviceCombo.onChange = [this] { openDevice (deviceCombo.getSelectedId() - 1); };

    // Channel
    addAndMakeVisible (channelLabel);
    addAndMakeVisible (channelCombo);
    for (int ch = 1; ch <= 16; ++ch)
        channelCombo.addItem ("Ch " + juce::String(ch), ch);
    channelCombo.setSelectedId (1, juce::dontSendNotification);
    channelCombo.onChange = [this] { midiChannel = channelCombo.getSelectedId(); };

    // Octave
    addAndMakeVisible (octaveLabel);
    addAndMakeVisible (octDownBtn);
    addAndMakeVisible (octUpBtn);
    addAndMakeVisible (octaveDisplay);
    octDownBtn.onClick = [this] {
        octaveShift = juce::jmax (-3, octaveShift - 1);
        octaveDisplay.setText (juce::String(octaveShift), juce::dontSendNotification);
    };
    octUpBtn.onClick = [this] {
        octaveShift = juce::jmin (3, octaveShift + 1);
        octaveDisplay.setText (juce::String(octaveShift), juce::dontSendNotification);
    };

    // Chord
    addAndMakeVisible (chordBtn);
    addAndMakeVisible (chordCombo);
    chordCombo.addItem ("Major Triad",   1);
    chordCombo.addItem ("Minor Triad",   2);
    chordCombo.addItem ("Major 7",       3);
    chordCombo.addItem ("Minor 7",       4);
    chordCombo.addItem ("Dominant 7",    5);
    chordCombo.addItem ("Sus2",          6);
    chordCombo.addItem ("Sus4",          7);
    chordCombo.addItem ("Power (5th)",   8);
    chordCombo.setSelectedId (1, juce::dontSendNotification);
    chordBtn.onStateChange = [this] { chordMode = chordBtn.getToggleState(); };

    // Scale
    addAndMakeVisible (scaleBtn);
    addAndMakeVisible (scaleRootCombo);
    addAndMakeVisible (scaleTypeCombo);
    static const char* noteNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    for (int i = 0; i < 12; ++i)
        scaleRootCombo.addItem (noteNames[i], i + 1);
    scaleRootCombo.setSelectedId (1, juce::dontSendNotification);
    auto scales = scaleNames();
    for (int i = 0; i < scales.size(); ++i)
        scaleTypeCombo.addItem (scales[i], i + 1);
    scaleTypeCombo.setSelectedId (2, juce::dontSendNotification); // Major
    scaleBtn.onStateChange = [this] { scaleMode = scaleBtn.getToggleState(); };
}

void MidiOutputPanel::refreshDevices()
{
    auto infos = juce::MidiOutput::getAvailableDevices();
    juce::StringArray newNames;
    for (const auto& info : infos) newNames.add (info.name);

    if (newNames == deviceNames) return;
    deviceNames = newNames;

    deviceCombo.clear (juce::dontSendNotification);
    deviceCombo.addItem ("-- none --", 1);
    for (int i = 0; i < deviceNames.size(); ++i)
        deviceCombo.addItem (deviceNames[i], i + 2);
    deviceCombo.setSelectedId (1, juce::dontSendNotification);
}

void MidiOutputPanel::openDevice (int index)
{
    midiOut.reset();
    selectedDevice = index;
    if (index < 0 || index >= deviceNames.size()) return;
    auto infos = juce::MidiOutput::getAvailableDevices();
    if (index < (int)infos.size())
        midiOut = juce::MidiOutput::openDevice (infos[index].identifier);
    repaint();
}

void MidiOutputPanel::timerCallback()
{
    refreshDevices();
}

//==============================================================================
void MidiOutputPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e2022));
    g.setColour (juce::Colour (0xff2a2c2f));
    g.drawRect (getLocalBounds(), 1);

    // Status dot
    g.setColour (midiOut ? juce::Colour (0xff44cc44) : juce::Colour (0xff443322));
    g.fillEllipse (getWidth() - 14.f, 6.f, 8.f, 8.f);

    g.setFont (juce::Font ("Courier New", 9.f, juce::Font::bold));
    g.setColour (juce::Colour (0xff6b4a0a));
    g.drawText (midiOut ? "CONNECTED" : "NOT CONNECTED",
                getWidth() - 110, 4, 100, 12, juce::Justification::centredRight);
}

void MidiOutputPanel::resized()
{
    auto area = getLocalBounds().reduced (6, 4);
    int rowH = 20, gap = 4;

    // Row 1: device
    auto row1 = area.removeFromTop (rowH); area.removeFromTop (gap);
    deviceLabel.setBounds (row1.removeFromLeft (52));
    deviceCombo.setBounds (row1);

    // Row 2: channel + octave
    auto row2 = area.removeFromTop (rowH); area.removeFromTop (gap);
    channelLabel.setBounds (row2.removeFromLeft (22));
    channelCombo.setBounds (row2.removeFromLeft (68));
    row2.removeFromLeft (8);
    octaveLabel.setBounds  (row2.removeFromLeft (24));
    octDownBtn.setBounds   (row2.removeFromLeft (18));
    octaveDisplay.setBounds(row2.removeFromLeft (20));
    octUpBtn.setBounds     (row2.removeFromLeft (18));

    // Row 3: chord
    auto row3 = area.removeFromTop (rowH); area.removeFromTop (gap);
    chordBtn.setBounds  (row3.removeFromLeft (54));
    chordCombo.setBounds (row3);

    // Row 4: scale
    auto row4 = area.removeFromTop (rowH);
    scaleBtn.setBounds       (row4.removeFromLeft (50));
    scaleRootCombo.setBounds (row4.removeFromLeft (44));
    row4.removeFromLeft (4);
    scaleTypeCombo.setBounds (row4);
}

//==============================================================================
juce::Array<int> MidiOutputPanel::processNote (int rawNote)
{
    int note = rawNote + octaveShift * 12;
    note = juce::jlimit (0, 127, note);

    if (scaleMode)
        note = snapToScale (note);

    if (chordMode)
        return buildChord (note);

    return { note };
}

void MidiOutputPanel::sendNoteOn (int midiNote, float velocity)
{
    if (! midiOut) return;
    auto notes = processNote (midiNote);
    for (int n : notes)
        midiOut->sendMessageNow (juce::MidiMessage::noteOn (
            midiChannel, n, (juce::uint8)(velocity * 127.f)));
}

void MidiOutputPanel::sendNoteOff (int midiNote)
{
    if (! midiOut) return;
    auto notes = processNote (midiNote);
    for (int n : notes)
        midiOut->sendMessageNow (juce::MidiMessage::noteOff (midiChannel, n));
}

void MidiOutputPanel::sendAllNotesOff()
{
    if (midiOut)
        midiOut->sendMessageNow (juce::MidiMessage::allNotesOff (midiChannel));
}

//==============================================================================
int MidiOutputPanel::snapToScale (int note) const
{
    int rootIdx = scaleRootCombo.getSelectedId() - 1;
    auto scaleName = scaleTypeCombo.getText();
    auto intervals  = scaleIntervals (scaleName);
    if (intervals.isEmpty()) return note;

    int octave = note / 12;
    int pitch  = note % 12;
    int adjusted = pitch - rootIdx;
    if (adjusted < 0) adjusted += 12;

    // Find nearest interval
    int bestDist = 99, bestInterval = 0;
    for (int iv : intervals)
    {
        int dist = std::abs (iv - adjusted);
        if (dist < bestDist) { bestDist = dist; bestInterval = iv; }
    }
    int snappedPitch = (rootIdx + bestInterval) % 12;
    return juce::jlimit (0, 127, octave * 12 + snappedPitch);
}

juce::Array<int> MidiOutputPanel::buildChord (int root) const
{
    int voicing = chordCombo.getSelectedId();
    switch (voicing)
    {
        case 1: return { root, root+4, root+7 };           // Major
        case 2: return { root, root+3, root+7 };           // Minor
        case 3: return { root, root+4, root+7, root+11 };  // Major 7
        case 4: return { root, root+3, root+7, root+10 };  // Minor 7
        case 5: return { root, root+4, root+7, root+10 };  // Dom 7
        case 6: return { root, root+2, root+7 };           // Sus2
        case 7: return { root, root+5, root+7 };           // Sus4
        case 8: return { root, root+7 };                   // Power
        default: return { root };
    }
}
