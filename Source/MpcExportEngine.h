#pragma once
#include <JuceHeader.h>
#include "MpcKitPanel.h"

// Results from an MPC export operation
struct MpcExportResult
{
    bool        success { false };
    juce::String message;
    juce::File   outputFolder;
    int          filesWritten { 0 };
};

//==============================================================================
// Handles exporting a full 16-pad kit to microSD-ready MPC folder structure.
//
// Output layout:
//   <outputRoot>/MPC/Samples/<KitName>/
//       [KIT_NAME]_A01_C2.wav
//       [KIT_NAME]_A02_C#2.wav
//       ...
//       [KIT_NAME]_D04_D#3.wav
//   <outputRoot>/MPC/Programs/<KitName>.xpm   (XML kit program file)
//
// Samples are:
//   - Read from disk
//   - Normalised to -0.5 dBFS peak
//   - Written as 24-bit 44.1 kHz WAV (MPC Sample native format)
//   - Named with MPC naming convention: KITNAME_PAD_NOTE.wav
class MpcExportEngine
{
public:
    MpcExportEngine();
    ~MpcExportEngine() = default;

    // Export a full kit to the given root folder.
    // kitName   : e.g. "MyKit" -> becomes folder and file prefix
    // outputRoot: top-level folder (e.g. the microSD root or a staging folder)
    [[nodiscard]] MpcExportResult exportKit (const MpcKitPanel& kit,
                               const juce::String& kitName,
                               const juce::File&   outputRoot);

    // Export only a single pad slot
    [[nodiscard]] MpcExportResult exportPad (const MpcPadSlot& pad,
                               const juce::String& kitName,
                               int                 padIndex,
                               const juce::File&   samplesFolder);

    // Progress callback (0.0 - 1.0)
    std::function<void(float)> onProgress;

private:
    juce::AudioFormatManager formatManager;

    // Read audio file into a buffer; returns nullptr on failure
    std::unique_ptr<juce::AudioBuffer<float>> readAudio (
        const juce::File& src, double& outSampleRate);

    // Peak-normalise buffer to targetPeak (linear, e.g. 0.944f = -0.5 dBFS)
    void normalise (juce::AudioBuffer<float>& buf, float targetPeak = 0.944f);

    // Write buffer to WAV 24-bit 44100 Hz
    bool writeWav (const juce::AudioBuffer<float>& buf,
                   double                           sourceSampleRate,
                   const juce::File&                dest);

    // Resample buffer to 44100 if needed
    void resampleIfNeeded (juce::AudioBuffer<float>& buf,
                           double sourceSampleRate,
                           double targetSampleRate = 44100.0);

    // Build XPM program XML text
    juce::String buildXpmProgram (const MpcKitPanel& kit,
                                  const juce::String& kitName);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MpcExportEngine)
};
