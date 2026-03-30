#pragma once
#include <JuceHeader.h>

//==============================================================================
// WaveformDisplay
//
// Lock-free ring buffer fed from the audio thread (pushSamples).
// Timer snapshots the buffer to displayBuffer on the message thread for paint.
//
// FIX: displayBuffer size is now matched exactly to kDisplaySize so no
// out-of-bounds access is possible even if kRingSize != kDisplaySize.
//==============================================================================
class WaveformDisplay : public juce::Component, private juce::Timer
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    // Called from audio thread — lock-free
    void pushSamples(const float* data, int numSamples);

    // Colours (set by editor)
    juce::Colour waveColour       { juce::Colours::orange };
    juce::Colour backgroundColour { juce::Colours::black };
    juce::Colour gridColour       { juce::Colour(0xff252729) };

private:
    static constexpr int kRingSize    = 4096;
    static constexpr int kDisplaySize = 512;

    // Ring buffer: written on audio thread, read on timer thread
    float ringBuffer[kRingSize] {};
    std::atomic<int> writePos { 0 };

    // Display snapshot: only accessed on message thread
    float displayBuffer[kDisplaySize] {};

    void timerCallback() override;
    void paint(juce::Graphics&) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
