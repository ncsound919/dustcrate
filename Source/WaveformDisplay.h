#pragma once
#include <JuceHeader.h>

//==============================================================================
// WaveformDisplay
// Draws a live waveform (oscilloscope-style) from the most recent audio buffer.
// Call pushSamples() from processBlock() to feed it data.
// The Component paints asynchronously using a timer.
//==============================================================================
class WaveformDisplay : public juce::Component,
                        public juce::Timer
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    // Feed audio data from the audio thread (lock-free ring buffer)
    void pushSamples(const float* data, int numSamples);

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    // Colour tokens (set from outside to match LookAndFeel)
    juce::Colour backgroundColour { juce::Colour(0xff1a1a1a) };
    juce::Colour waveColour       { juce::Colour(0xfff0a020) };
    juce::Colour gridColour       { juce::Colour(0xff2a2a2a) };

private:
    static constexpr int kRingSize = 8192;
    float ringBuffer[kRingSize] {};
    std::atomic<int> writePos { 0 };

    // Display buffer (read on message thread)
    static constexpr int kDisplaySize = 512;
    float displayBuffer[kDisplaySize] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
