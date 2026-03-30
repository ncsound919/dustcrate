#include "SamplePreview.h"

SamplePreview::SamplePreview()
{
    formatManager.registerBasicFormats();
}

SamplePreview::~SamplePreview()
{
    stop();
    // FIX: only remove callback from the manager we actually registered with
    if (activeManager != nullptr)
        activeManager->removeAudioCallback(this);
    activeManager = nullptr;
}

void SamplePreview::initialise(juce::AudioDeviceManager* ext)
{
    // FIX: do not double-initialise if called twice
    if (activeManager != nullptr) return;

    if (ext != nullptr)
    {
        activeManager = ext;
    }
    else
    {
        const juce::String err = ownManager.initialiseWithDefaultDevices(0, 2);
        // If no device is available (headless/plugin host), gracefully degrade
        if (err.isNotEmpty())
        {
            // Preview disabled — not fatal
            return;
        }
        activeManager = &ownManager;
    }
    activeManager->addAudioCallback(this);
}

void SamplePreview::previewFile(const juce::File& file, float trimGain)
{
    if (activeManager == nullptr) return; // graceful no-op when device unavailable
    stop();
    trim = juce::jlimit(0.0f, 1.0f, trimGain);
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr) return;

    const juce::ScopedLock sl(lock);
    readerSource     = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    resamplingSource = std::make_unique<juce::ResamplingAudioSource>(readerSource.get(), false, 2);
    // FIX: prepareToPlay before setSource so transport is in valid state
    resamplingSource->prepareToPlay(512, deviceSR);
    transportSource.prepareToPlay(512, deviceSR);
    transportSource.setSource(resamplingSource.get(), 0, nullptr, reader->sampleRate);
    if (deviceSR > 0.0)
        resamplingSource->setResamplingRatio(reader->sampleRate / deviceSR);
    transportSource.setPosition(0.0);
    transportSource.start();
    playing.store(true);
}

void SamplePreview::stop()
{
    const juce::ScopedLock sl(lock);
    transportSource.stop();
    transportSource.setSource(nullptr);
    // FIX: release resources before resetting sources to avoid UAF
    transportSource.releaseResources();
    readerSource.reset();
    resamplingSource.reset();
    playing.store(false);
}

void SamplePreview::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    deviceSR = device ? device->getCurrentSampleRate() : 44100.0;
    transportSource.prepareToPlay(device ? device->getCurrentBufferSizeSamples() : 512, deviceSR);
}

void SamplePreview::audioDeviceStopped()
{
    transportSource.releaseResources();
}

void SamplePreview::audioDeviceIOCallbackWithContext(
    const float* const*, int,
    float* const* out, int numOut,
    int numSamples,
    const juce::AudioIODeviceCallbackContext&)
{
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
