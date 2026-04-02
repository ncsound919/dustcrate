#pragma once
#include <JuceHeader.h>

struct SampleEntry
{
    juce::String name;          // display name
    juce::String filePath;      // relative path under assets/samples/
    juce::String category;      // drums | melodic | textures | vocal | noise
    juce::String subcategory;   // VINYL | DUST | ROOM | HUM | CRACKLE | KICK | etc.
    juce::String pack;          // pack name
    int          rootNote { 60 };
    juce::String license;
    juce::String source;
};

class SampleLibrary
{
public:
    SampleLibrary();

    void loadPackFromJSON        (const juce::File& jsonFile);
    void loadPackFromBinaryData  (const char* data, int dataSize);

    // Scan a folder recursively; WAV/AIFF files become noise entries
    // if the folder name contains vinyl/dust/noise/crackle/hum/room
    void scanUserFolder (const juce::File& folder,
                         const juce::String& packName = "User");

    const juce::Array<SampleEntry>& getAllSamples()  const { return samples; }
    juce::Array<SampleEntry>  getByCategory    (const juce::String&) const;
    juce::Array<SampleEntry>  getBySubcategory (const juce::String&) const;
    juce::Array<SampleEntry>  getByPack        (const juce::String&) const;
    juce::StringArray         getCategories    () const;
    juce::StringArray         getSubcategories (const juce::String& category) const;
    juce::StringArray         getPacks         () const;

    [[nodiscard]] juce::File resolveFilePath (const SampleEntry&) const;
    void setAssetsRoot (const juce::File& root) { assetsRoot = root; }

private:
    juce::Array<SampleEntry> samples;
    juce::File               assetsRoot;
    void parseJSON (const juce::var& json);
    juce::String inferSubcategory (const juce::String& filename,
                                   const juce::String& folderName) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleLibrary)
};
