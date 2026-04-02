#pragma once
#include <JuceHeader.h>

//==============================================================================
// SamplePreview
//
// Lightweight single-click audition engine. Uses a juce::AudioDeviceManager
// with a tiny AudioFormatReaderSource so previewing a file doesn't require
// a MIDI note — it just plays once from the beginning at the trim level.
//
// Call previewFile() from the browser’s onSampleSelected callback.
// Call stop() to silence early.
//==============================================================================
class SamplePreview : public juce::AudioIODeviceCallback
{
public:
    SamplePreview();
    ~SamplePreview() override;

    // Initialise with app’s AudioDeviceManager (pass null to use own)
    void initialise (juce::AudioDeviceManager* externalManager = nullptr);

    void previewFile (const juce::File& file, float trimGain = 0.75f);
    // Preview from a specific position (seconds) — used by slicer marker playback
    void previewFrom (const juce::File& file, double positionSeconds, float trimGain = 0.75f);
    void stop        ();

    bool isPlaying() const { return playing.load(); }

    // AudioIODeviceCallback
    void audioDeviceIOCallbackWithContext (const float* const*  inputChannelData,
                                           int                  numInputChannels,
                                           float* const*        outputChannelData,
                                           int                  numOutputChannels,
                                           int                  numSamples,
                                           const juce::AudioIODeviceCallbackContext&) override;
    void audioDeviceAboutToStart (juce::AudioIODevice*) override;
    void audioDeviceStopped      ()                     override;

private:
    juce::AudioDeviceManager  ownManager;
    juce::AudioDeviceManager* activeManager { nullptr };
    juce::AudioFormatManager  formatManager;

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::ResamplingAudioSource>   resamplingSource;
    juce::AudioTransportSource transportSource;

    std::atomic<bool>  playing { false };
    float              trim    { 0.75f };
    double             deviceSR { 44100.0 };

    juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplePreview)
};
