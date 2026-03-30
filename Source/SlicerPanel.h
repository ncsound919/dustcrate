#pragma once
#include <JuceHeader.h>

// One transient slice marker
struct SliceMarker
{
    int   samplePos  { 0 };   // position in samples
    int   midiNote   { 60 };  // C4 default; reassigned on export
    float normPos    { 0.f }; // 0.0-1.0 normalised
};

//==============================================================================
// Waveform display with transient detection, manual slice editing,
// and MIDI chop-map export compatible with MPC 3.7+ slice modulation.
class SlicerPanel : public juce::Component,
                   public juce::ChangeListener
{
public:
    SlicerPanel();
    ~SlicerPanel() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

    // Load audio file for display and slicing
    void loadFile (const juce::File& f);
    void clearFile ();

    // Auto-slice by transient onset threshold (0.0-1.0)
    void detectTransients (float threshold = 0.15f);

    // Auto-slice to a fixed number of equal slices
    void sliceEven (int numSlices);

    // Add / remove a marker manually
    void addMarker    (int samplePos);
    void removeMarker (int markerIndex);
    void clearMarkers ();

    const juce::Array<SliceMarker>& getMarkers() const { return markers; }

    // Export slice WAVs + MIDI chop map to outputFolder
    // Returns number of slices written
    int exportSlices (const juce::File& outputFolder, const juce::String& stemName);

    // Callback when user clicks a marker — for live preview
    std::function<void(int samplePos)> onMarkerClicked;

    // Mouse interaction for adding/removing markers
    void mouseDown  (const juce::MouseEvent&) override;
    void mouseDrag  (const juce::MouseEvent&) override;
    void mouseUp    (const juce::MouseEvent&) override;

    // juce::ChangeListener — thumbnail ready
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    // Expose total sample count for mapping
    juce::int64 getTotalSamples() const { return totalSamples; }
    double      getSampleRate()   const { return sampleRate; }

private:
    juce::AudioFormatManager      formatManager;
    juce::AudioThumbnailCache     thumbCache { 5 };
    juce::AudioThumbnail          thumbnail;
    juce::Array<SliceMarker>      markers;

    juce::int64 totalSamples { 0 };
    double      sampleRate   { 44100.0 };
    juce::File  currentFile;
    bool        fileLoaded   { false };

    int  draggingMarker { -1 };
    bool isDragging     { false };

    // Convert x pixel to sample position
    juce::int64 xToSample (float x) const;
    float       sampleToX (juce::int64 pos) const;

    int  nearestMarker (float x, int tolerancePx = 6) const;
    void paintMarker   (juce::Graphics&, const SliceMarker&, bool highlight);
    void reassignMidiNotes ();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlicerPanel)
};
