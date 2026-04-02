#include "SamplePreview.h"

SamplePreview::SamplePreview()
{
    formatManager.registerBasicFormats();
}

SamplePreview::~SamplePreview()
{
    stop();
    if (activeManager != nullptr)
    {
        activeManager->removeAudioCallback(this);
        activeManager = nullptr;
    }
}

void SamplePreview::initialise(juce::AudioDeviceManager* ext)
{
    if (activeManager != nullptr) return; // guard double-init

    if (ext != nullptr)
    {
        activeManager = ext;
    }
    else
    {
        const juce::String err = ownManager.initialiseWithDefaultDevices(0, 2);
        if (err.isNotEmpty()) return; // graceful: preview disabled, not fatal
        activeManager = &ownManager;
    }
    activeManager->addAudioCallback(this);
}

void SamplePreview::previewFile(const juce::File& file, float trimGain)
{
    if (activeManager == nullptr) return;
    if (! file.existsAsFile())    return; // FIX: guard missing file before allocation
    stop();
    trim = juce::jlimit(0.0f, 1.0f, trimGain);

    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr) return;

    // FIX: use a safe default SR if audioDeviceAboutToStart hasn’t fired yet
    const double effectiveSR = (deviceSR > 0.0) ? deviceSR : 44100.0;

    const juce::ScopedLock sl(lock);
    readerSource     = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    resamplingSource = std::make_unique<juce::ResamplingAudioSource>(readerSource.get(), false, 2);
    resamplingSource->prepareToPlay(512, effectiveSR);
    transportSource.prepareToPlay(512, effectiveSR);
    transportSource.setSource(resamplingSource.get(), 0, nullptr, reader->sampleRate);
    resamplingSource->setResamplingRatio(reader->sampleRate / effectiveSR);
    transportSource.setPosition(0.0);
    transportSource.start();
    playing.store(true);
}

void SamplePreview::previewFrom (const juce::File& file, double positionSeconds, float trimGain)
{
    if (activeManager == nullptr) return;
    if (! file.existsAsFile()) return;
    stop();
    trim = juce::jlimit (0.0f, 1.0f, trimGain);

    auto* reader = formatManager.createReaderFor (file);
    if (reader == nullptr) return;

    const double effectiveSR = (deviceSR > 0.0) ? deviceSR : 44100.0;

    const juce::ScopedLock sl (lock);
    readerSource     = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
    resamplingSource = std::make_unique<juce::ResamplingAudioSource> (readerSource.get(), false, 2);
    resamplingSource->prepareToPlay (512, effectiveSR);
    transportSource.prepareToPlay (512, effectiveSR);
    transportSource.setSource (resamplingSource.get(), 0, nullptr, reader->sampleRate);
    resamplingSource->setResamplingRatio (reader->sampleRate / effectiveSR);
    // Clamp to valid range — ignore negative/out-of-bounds positions
    const double clampedPos = juce::jlimit (0.0, transportSource.getLengthInSeconds(), positionSeconds);
    transportSource.setPosition (clampedPos);
    transportSource.start();
    playing.store (true);
}

{
    const juce::ScopedLock sl(lock);
    transportSource.stop();
    transportSource.setSource(nullptr);
    transportSource.releaseResources();
    readerSource.reset();
    resamplingSource.reset();
    playing.store(false);
}

void SamplePreview::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    deviceSR = (device != nullptr && device->getCurrentSampleRate() > 0.0)
             ? device->getCurrentSampleRate() : 44100.0;
    const int bufSize = (device != nullptr) ? device->getCurrentBufferSizeSamples() : 512;
    transportSource.prepareToPlay(bufSize, deviceSR);
}

void SamplePreview::audioDeviceStopped()
{
    transportSource.releaseResources();
    deviceSR = 44100.0; // FIX: reset to safe default rather than leaving as last value
}

void SamplePreview::audioDeviceIOCallbackWithContext(
    const float* const*, int,
    float* const* out, int numOut,
    int numSamples,
    const juce::AudioIODeviceCallbackContext&)
{
    if (out == nullptr || numOut <= 0 || numSamples <= 0) return; // FIX: null-out guard
    juce::AudioBuffer<float> buf(out, numOut, numSamples);
    buf.clear();
    if (! playing.load()) return;

    const juce::ScopedTryLock stl(lock);
    if (! stl.isLocked()) return;

    juce::AudioSourceChannelInfo info(&buf, 0, numSamples);
    transportSource.getNextAudioBlock(info);
    buf.applyGain(trim);

    if (! transportSource.isPlaying())
        playing.store(false);
}
