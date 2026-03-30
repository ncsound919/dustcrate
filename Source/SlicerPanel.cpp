#include "SlicerPanel.h"

SlicerPanel::SlicerPanel()
    : thumbnail (512, formatManager, thumbCache)
{
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener (this);
}

SlicerPanel::~SlicerPanel()
{
    thumbnail.removeChangeListener (this);
}

//==============================================================================
void SlicerPanel::loadFile (const juce::File& f)
{
    if (! f.existsAsFile()) return;
    currentFile  = f;
    fileLoaded   = false;
    totalSamples = 0;
    markers.clear();

    std::unique_ptr<juce::AudioFormatReader> reader (
        formatManager.createReaderFor (f));
    if (reader)
    {
        totalSamples = (juce::int64)reader->lengthInSamples;
        sampleRate   = reader->sampleRate;
        thumbnail.setSource (new juce::FileInputSource (f));
    }
    repaint();
}

void SlicerPanel::clearFile()
{
    thumbnail.clear();
    markers.clear();
    totalSamples = 0;
    fileLoaded   = false;
    currentFile  = {};
    repaint();
}

void SlicerPanel::changeListenerCallback (juce::ChangeBroadcaster*)
{
    fileLoaded = true;
    repaint();
}

//==============================================================================
void SlicerPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.fillAll (juce::Colour (0xff141517));
    g.setColour (juce::Colour (0xff2a2c2f));
    g.drawRect (bounds, 1.0f);

    if (! fileLoaded || totalSamples == 0)
    {
        g.setFont (juce::Font ("Courier New", 11.f, juce::Font::plain));
        g.setColour (juce::Colour (0xff3a3c3e));
        g.drawText ("Load a sample to slice", bounds, juce::Justification::centred);
        return;
    }

    // Draw waveform
    g.setColour (juce::Colour (0xff3a3010));
    thumbnail.drawChannels (g, getLocalBounds().reduced(1), 0.0,
                            thumbnail.getTotalLength(), 0.8f);

    // Overlay amber tint on waveform
    g.setColour (juce::Colour (0x18c8921a));
    g.fillRect (bounds.reduced (1));

    // Draw slice markers
    for (int i = 0; i < markers.size(); ++i)
        paintMarker (g, markers[i], i == draggingMarker);

    // Slice count hint
    g.setFont (juce::Font ("Courier New", 9.f, juce::Font::bold));
    g.setColour (juce::Colour (0xff6b4a0a));
    g.drawText (juce::String (markers.size()) + " SLICES",
                bounds.reduced(4).removeFromBottom(14),
                juce::Justification::bottomRight, false);
}

void SlicerPanel::paintMarker (juce::Graphics& g,
                               const SliceMarker& m, bool highlight)
{
    float x = sampleToX (m.samplePos);
    juce::Colour col = highlight ? juce::Colour (0xffffd060)
                                 : juce::Colour (0xffc8921a);
    g.setColour (col);
    g.fillRect (x - 0.75f, 0.0f, 1.5f, (float)getHeight());

    // Triangle at top
    juce::Path tri;
    tri.addTriangle (x, 0, x - 4, 8, x + 4, 8);
    g.fillPath (tri);

    // Note name badge
    g.setFont (juce::Font ("Courier New", 8.f, juce::Font::plain));
    g.setColour (col);
    g.drawText (juce::MidiMessage::getMidiNoteName (m.midiNote, true, true, 4),
                juce::Rectangle<float>(x + 2, 2, 24, 10),
                juce::Justification::centredLeft, false);
}

void SlicerPanel::resized() {}

//==============================================================================
juce::int64 SlicerPanel::xToSample (float x) const
{
    if (getWidth() <= 0 || totalSamples <= 0) return 0;
    return juce::jlimit<juce::int64> (0, totalSamples - 1,
        (juce::int64)((x / (float)getWidth()) * (float)totalSamples));
}

float SlicerPanel::sampleToX (juce::int64 pos) const
{
    if (totalSamples <= 0) return 0.f;
    return ((float)pos / (float)totalSamples) * (float)getWidth();
}

int SlicerPanel::nearestMarker (float x, int tolerancePx) const
{
    for (int i = 0; i < markers.size(); ++i)
    {
        float mx = sampleToX (markers[i].samplePos);
        if (std::abs (mx - x) <= (float)tolerancePx)
            return i;
    }
    return -1;
}

//==============================================================================
void SlicerPanel::mouseDown (const juce::MouseEvent& e)
{
    if (! fileLoaded) return;
    float x = (float)e.x;
    int near = nearestMarker (x, 8);

    if (e.mods.isRightButtonDown())
    {
        if (near >= 0) removeMarker (near);
        return;
    }

    if (near >= 0)
    {
        draggingMarker = near;
        isDragging     = true;
        if (onMarkerClicked) onMarkerClicked (markers[near].samplePos);
    }
    else
    {
        addMarker ((int)xToSample (x));
    }
    repaint();
}

void SlicerPanel::mouseDrag (const juce::MouseEvent& e)
{
    if (! isDragging || draggingMarker < 0) return;
    markers.getReference (draggingMarker).samplePos = (int)xToSample ((float)e.x);
    markers.getReference (draggingMarker).normPos   = (float)e.x / (float)getWidth();
    reassignMidiNotes();
    repaint();
}

void SlicerPanel::mouseUp (const juce::MouseEvent&)
{
    isDragging     = false;
    draggingMarker = -1;
    repaint();
}

//==============================================================================
void SlicerPanel::addMarker (int samplePos)
{
    SliceMarker m;
    m.samplePos = samplePos;
    m.normPos   = totalSamples > 0 ? (float)samplePos / (float)totalSamples : 0.f;
    markers.add (m);
    markers.sort ([](const SliceMarker& a, const SliceMarker& b) {
        return a.samplePos < b.samplePos; });
    reassignMidiNotes();
}

void SlicerPanel::removeMarker (int index)
{
    if (index >= 0 && index < markers.size())
    {
        markers.remove (index);
        reassignMidiNotes();
    }
}

void SlicerPanel::clearMarkers()
{
    markers.clear();
    repaint();
}

void SlicerPanel::reassignMidiNotes()
{
    for (int i = 0; i < markers.size(); ++i)
        markers.getReference(i).midiNote = 36 + i; // C2 upward
}

void SlicerPanel::detectTransients (float threshold)
{
    if (! currentFile.existsAsFile() || totalSamples == 0) return;
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (currentFile));
    if (! reader) return;

    int blockSize  = 512;
    int numBlocks  = (int)(totalSamples / blockSize);
    float prevRms  = 0.f;
    markers.clear();

    juce::AudioBuffer<float> block (1, blockSize);
    for (int b = 0; b < numBlocks; ++b)
    {
        reader->read (&block, 0, blockSize, (juce::int64)b * blockSize, true, false);
        float rms = block.getRMSLevel (0, 0, blockSize);
        if (rms - prevRms > threshold && b > 0)
            addMarker (b * blockSize);
        prevRms = rms;
    }
    repaint();
}

void SlicerPanel::sliceEven (int numSlices)
{
    if (totalSamples == 0 || numSlices < 1) return;
    markers.clear();
    int step = (int)(totalSamples / numSlices);
    for (int i = 1; i < numSlices; ++i)
        addMarker (i * step);
    repaint();
}

//==============================================================================
int SlicerPanel::exportSlices (const juce::File& outputFolder,
                               const juce::String& stemName)
{
    if (! currentFile.existsAsFile() || totalSamples == 0) return 0;
    outputFolder.createDirectory();

    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (currentFile));
    if (! reader) return 0;

    // Build slice boundaries (start markers)
    juce::Array<int> starts { 0 };
    for (const auto& m : markers) starts.add (m.samplePos);
    starts.add ((int)totalSamples);

    // MIDI file for chop map
    juce::MidiMessageSequence seq;
    double ticksPerBeat = 960.0;
    double bpm = 120.0;
    double samplesPerBeat = sampleRate * 60.0 / bpm;

    juce::WavAudioFormat wavFmt;
    int written = 0;

    for (int i = 0; i + 1 < starts.size(); ++i)
    {
        int startSmp = starts[i];
        int endSmp   = starts[i + 1];
        int len      = endSmp - startSmp;
        if (len <= 0) continue;

        juce::AudioBuffer<float> slice ((int)reader->numChannels, len);
        reader->read (&slice, 0, len, (juce::int64)startSmp, true, true);

        auto fileName = stemName + "_slice" +
                        juce::String (i + 1).paddedLeft ('0', 2) + ".wav";
        auto dest = outputFolder.getChildFile (fileName);
        dest.deleteFile();
        auto stream = std::unique_ptr<juce::FileOutputStream>(dest.createOutputStream());
        if (! stream) continue;

        std::unique_ptr<juce::AudioFormatWriter> writer (
            wavFmt.createWriterFor (stream.get(), sampleRate,
                                    (unsigned int)slice.getNumChannels(), 24, {}, 0));
        if (! writer) continue;
        stream.release();
        writer->writeFromAudioSampleBuffer (slice, 0, len);
        ++written;

        // Add MIDI note to chop map
        int note      = 36 + i;
        double ticks  = ((double)startSmp / samplesPerBeat) * ticksPerBeat;
        double durTks = ((double)len      / samplesPerBeat) * ticksPerBeat;
        seq.addEvent (juce::MidiMessage::noteOn  (1, note, (juce::uint8)100), ticks);
        seq.addEvent (juce::MidiMessage::noteOff (1, note),                   ticks + durTks);
    }

    // Write chop-map MIDI file
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote ((int)ticksPerBeat);
    midiFile.addTrack (seq);
    auto midiDest = outputFolder.getChildFile (stemName + "_chopmap.mid");
    juce::FileOutputStream midiStream (midiDest);
    midiFile.writeTo (midiStream);

    return written;
}
