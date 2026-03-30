#pragma once
#include <JuceHeader.h>

struct SampleEntry
{
    juce::String name;       // display name
    juce::String filePath;   // relative path under assets/samples/
    juce::String category;   // drums | melodic | textures | vocal | noise
    juce::String pack;       // pack name (e.g. "Factory Pack")
    int          rootNote;   // MIDI root note (default 60 = C3)
    juce::String license;    // CC0, Public Domain, etc.
    juce::String source;     // source URL or attribution
};

class SampleLibrary
{
public:
    SampleLibrary();

    void loadPackFromJSON(const juce::File& jsonFile);
    void loadPackFromBinaryData(const char* data, int dataSize);

    const juce::Array<SampleEntry>& getAllSamples() const { return samples; }
    juce::Array<SampleEntry> getByCategory(const juce::String& category) const;
    juce::Array<SampleEntry> getByPack(const juce::String& pack) const;
    juce::StringArray getCategories() const;
    juce::StringArray getPacks() const;

    // Returns absolute path for playback
    juce::File resolveFilePath(const SampleEntry& entry) const;
    void setAssetsRoot(const juce::File& root) { assetsRoot = root; }

private:
    juce::Array<SampleEntry> samples;
    juce::File assetsRoot;
    void parseJSON(const juce::var& json);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleLibrary)
};
