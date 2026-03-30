#include "SamplePreview.h"

SamplePreview::SamplePreview()
{
    formatManager.registerBasicFormats();
}

SamplePreview::~SamplePreview()
{
    stop();
    if (activeManager)
        activeManager->removeAudioCallback(this);
}

void SamplePreview::initialise(juce::AudioDeviceManager* ext)
{
    if (ext)
    {
        activeManager = ext;
    }
    else
    {
        ownManager.initialiseWithDefaultDevices(0, 2);
        activeManager = &ownManager;
    }
    activeManager->addAudioCallback(this);
}

void SamplePreview::previewFile(const juce::File& file, float trimGain)
{
    stop();
    trim = trimGain;
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr) return;

    const juce::ScopedLock sl(lock);
    readerSource    = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    resamplingSource = std::make_unique<juce::ResamplingAudioSource>(readerSource.get(), false, 2);
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
