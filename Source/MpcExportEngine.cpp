#include "MpcExportEngine.h"

MpcExportEngine::MpcExportEngine()
{
    formatManager.registerBasicFormats();
}

//==============================================================================
MpcExportResult MpcExportEngine::exportKit (const MpcKitPanel& kit,
                                            const juce::String& kitName,
                                            const juce::File&   outputRoot)
{
    MpcExportResult result;

    // Create MPC folder structure
    auto samplesDir  = outputRoot.getChildFile ("MPC").getChildFile ("Samples")
                                 .getChildFile (kitName);
    auto programsDir = outputRoot.getChildFile ("MPC").getChildFile ("Programs");

    if (! samplesDir.createDirectory())
    {
        result.message = "Could not create MPC/Samples/" + kitName;
        return result;
    }
    programsDir.createDirectory();

    int written = 0;
    int occupied = 0;
    for (int i = 0; i < MpcKitPanel::kNumPads; ++i)
        if (kit.getPad(i).occupied) ++occupied;

    int done = 0;
    for (int i = 0; i < MpcKitPanel::kNumPads; ++i)
    {
        const auto& pad = kit.getPad (i);
        if (! pad.occupied) continue;

        auto r = exportPad (pad, kitName, i, samplesDir);
        if (r.success) ++written;
        ++done;
        if (onProgress) onProgress ((float)done / (float)juce::jmax (1, occupied));
    }

    // Write XPM program file
    auto xpmFile = programsDir.getChildFile (kitName + ".xpm");
    auto xpmText = buildXpmProgram (kit, kitName);
    xpmFile.replaceWithText (xpmText);

    result.success      = (written > 0);
    result.filesWritten = written;
    result.outputFolder = samplesDir;
    result.message      = result.success
        ? juce::String (written) + " samples exported to " + samplesDir.getFullPathName()
        : "No occupied pads to export";
    return result;
}

MpcExportResult MpcExportEngine::exportPad (const MpcPadSlot& pad,
                                            const juce::String& kitName,
                                            int                 padIndex,
                                            const juce::File&   samplesFolder)
{
    MpcExportResult result;
    juce::File src (pad.filePath);
    if (! src.existsAsFile())
    {
        result.message = "File not found: " + pad.filePath;
        return result;
    }

    double srcRate = 44100.0;
    auto buf = readAudio (src, srcRate);
    if (! buf)
    {
        result.message = "Could not read: " + src.getFileName();
        return result;
    }

    // Resample to 44100 if needed
    resampleIfNeeded (*buf, srcRate);

    // Normalise to -0.5 dBFS
    normalise (*buf, 0.944f);

    // Build MPC filename: KITNAME_A01_C2.wav
    auto noteName = juce::MidiMessage::getMidiNoteName (pad.rootNote, true, true, 4)
                        .replaceCharacter ('#', 's'); // MPC doesn't like # in filenames
    auto fileName = kitName + "_" + MpcKitPanel::padLabel (padIndex)
                    + "_" + noteName + ".wav";
    auto dest = samplesFolder.getChildFile (fileName);

    if (! writeWav (*buf, 44100.0, dest))
    {
        result.message = "Failed to write " + dest.getFileName();
        return result;
    }

    result.success      = true;
    result.filesWritten = 1;
    result.outputFolder = samplesFolder;
    result.message      = dest.getFileName();
    return result;
}

//==============================================================================
std::unique_ptr<juce::AudioBuffer<float>> MpcExportEngine::readAudio (
    const juce::File& src, double& outSampleRate)
{
    std::unique_ptr<juce::AudioFormatReader> reader (
        formatManager.createReaderFor (src));
    if (! reader) return nullptr;

    outSampleRate = reader->sampleRate;
    auto numSamples  = (int)reader->lengthInSamples;
    auto numChannels = (int)juce::jmin ((int)reader->numChannels, 2);

    auto buf = std::make_unique<juce::AudioBuffer<float>> (numChannels, numSamples);
    reader->read (buf.get(), 0, numSamples, 0, true, numChannels > 1);
    return buf;
}

void MpcExportEngine::normalise (juce::AudioBuffer<float>& buf, float targetPeak)
{
    float peak = 0.0f;
    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
        peak = juce::jmax (peak, buf.getMagnitude (ch, 0, buf.getNumSamples()));

    if (peak > 0.0001f)
        buf.applyGain (targetPeak / peak);
}

bool MpcExportEngine::writeWav (const juce::AudioBuffer<float>& buf,
                                double sourceSampleRate,
                                const juce::File& dest)
{
    juce::WavAudioFormat wavFmt;
    dest.deleteFile();
    auto stream = std::unique_ptr<juce::FileOutputStream> (dest.createOutputStream());
    if (! stream) return false;

    std::unique_ptr<juce::AudioFormatWriter> writer (
        wavFmt.createWriterFor (stream.get(),
                                sourceSampleRate,
                                (unsigned int)buf.getNumChannels(),
                                24, {}, 0));
    if (! writer) return false;
    stream.release(); // writer owns the stream
    return writer->writeFromAudioSampleBuffer (buf, 0, buf.getNumSamples());
}

void MpcExportEngine::resampleIfNeeded (juce::AudioBuffer<float>& buf,
                                        double sourceSampleRate,
                                        double targetSampleRate)
{
    if (std::abs (sourceSampleRate - targetSampleRate) < 1.0) return;

    double ratio = targetSampleRate / sourceSampleRate;
    int newLen   = (int)(buf.getNumSamples() * ratio + 0.5);
    juce::AudioBuffer<float> resampled (buf.getNumChannels(), newLen);

    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
    {
        juce::LagrangeInterpolator interp;
        interp.reset();
        interp.process (1.0 / ratio,
                        buf.getReadPointer (ch),
                        resampled.getWritePointer (ch),
                        newLen);
    }
    buf = std::move (resampled);
}

//==============================================================================
juce::String MpcExportEngine::buildXpmProgram (const MpcKitPanel& kit,
                                               const juce::String& kitName)
{
    // FIX: XML-escape all user-provided strings so characters like <, >, &, ', "
    // in kit names or sample names don't produce malformed XPM XML.
    auto xmlEscape = [](const juce::String& s) -> juce::String
    {
        return s.replace("&",  "&amp;")
                .replace("<",  "&lt;")
                .replace(">",  "&gt;")
                .replace("\"", "&quot;")
                .replace("'",  "&apos;");
    };

    const juce::String safeKitName = xmlEscape(kitName);

    juce::String xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<MPCVObject version=\"2.1\" type=\"Program\">\n";
    xml << "  <Program name=\"" << safeKitName << "\" type=\"Drum\">\n";
    xml << "    <Pads>\n";

    for (int i = 0; i < MpcKitPanel::kNumPads; ++i)
    {
        const auto& pad = kit.getPad (i);
        auto noteName = juce::MidiMessage::getMidiNoteName (pad.rootNote, true, true, 4)
                            .replaceCharacter ('#', 's');
        auto rawFileName = pad.occupied
            ? kitName + "_" + MpcKitPanel::padLabel(i) + "_" + noteName + ".wav"
            : juce::String();
        const juce::String safeFileName = xmlEscape(rawFileName);

        xml << "      <Pad index=\"" << i << "\""
            << " note=\"" << pad.rootNote << "\""
            << " bank=\"A\""
            << " label=\"" << xmlEscape(MpcKitPanel::padLabel(i)) << "\">\n";
        if (pad.occupied)
            xml << "        <Layer file=\"" << safeFileName << "\" level=\"100\"/>\n";
        xml << "      </Pad>\n";
    }

    xml << "    </Pads>\n";
    xml << "  </Program>\n";
    xml << "</MPCVObject>\n";
    return xml;
}
