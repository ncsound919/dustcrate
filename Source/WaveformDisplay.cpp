#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay()
{
    startTimerHz(30);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

void WaveformDisplay::pushSamples(const float* data, int numSamples)
{
    if (data == nullptr || numSamples <= 0) return; // FIX: null-data guard
    // FIX: clamp to ring capacity so a large host block never wraps more than
    // kRingSize samples and corrupts the read pointer ordering
    const int n = juce::jmin(numSamples, kRingSize);
    const int offset = (numSamples > kRingSize) ? (numSamples - kRingSize) : 0;
    int wp = writePos.load(std::memory_order_relaxed);
    for (int i = 0; i < n; ++i)
    {
        ringBuffer[wp] = data[offset + i];
        wp = (wp + 1) % kRingSize;
    }
    writePos.store(wp, std::memory_order_release);
}

void WaveformDisplay::timerCallback()
{
    // Snapshot the most recent kDisplaySize samples — message thread only
    const int wp    = writePos.load(std::memory_order_acquire);
    const int start = (wp - kDisplaySize + kRingSize) % kRingSize;
    for (int i = 0; i < kDisplaySize; ++i)
        displayBuffer[i] = ringBuffer[(start + i) % kRingSize];
    repaint();
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    g.setColour(backgroundColour);
    g.fillRoundedRectangle(b, 3.0f);

    g.setColour(gridColour);
    g.drawHorizontalLine((int)(b.getCentreY()), b.getX() + 2.0f, b.getRight() - 2.0f);

    const float w  = b.getWidth() - 4.0f;
    const float h  = b.getHeight() - 4.0f;
    const float cx = b.getX() + 2.0f;
    const float cy = b.getCentreY();

    juce::Path wave;
    for (int i = 0; i < kDisplaySize; ++i)
    {
        // FIX: clamp display samples to prevent path drawing outside bounds
        const float s = juce::jlimit(-1.0f, 1.0f, displayBuffer[i]);
        const float x = cx + (float)i / (float)(kDisplaySize - 1) * w;
        const float y = cy - s * h * 0.45f;
        if (i == 0) wave.startNewSubPath(x, y);
        else        wave.lineTo(x, y);
    }

    g.setColour(waveColour.withAlpha(0.85f));
    g.strokePath(wave, juce::PathStrokeType(1.2f,
                        juce::PathStrokeType::curved,
                        juce::PathStrokeType::rounded));

    g.setColour(juce::Colour(0xff2e3032));
    g.drawRoundedRectangle(b.reduced(0.5f), 3.0f, 0.8f);
}
