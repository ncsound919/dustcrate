#include "SampleLibrary.h"

SampleLibrary::SampleLibrary() {}

void SampleLibrary::loadPackFromJSON(const juce::File& jsonFile)
{
    juce::var json;
    auto result = juce::JSON::parse(jsonFile.loadFileAsString(), json);
    if (result.wasOk())
        parseJSON(json);
}

void SampleLibrary::loadPackFromBinaryData(const char* data, int dataSize)
{
    juce::var json;
    auto result = juce::JSON::parse(
        juce::String::fromUTF8(data, dataSize), json);
    if (result.wasOk())
        parseJSON(json);
}

void SampleLibrary::parseJSON(const juce::var& json)
{
    if (auto* arr = json["samples"].getArray())
    {
        for (const auto& item : *arr)
        {
            SampleEntry entry;
            entry.name     = item["name"].toString();
            entry.filePath = item["filePath"].toString();
            entry.category = item["category"].toString();
            entry.pack     = item["pack"].toString();
            entry.rootNote = static_cast<int>(item["rootNote"]);
            entry.license  = item["license"].toString();
            entry.source   = item["source"].toString();
            samples.add(entry);
        }
    }
}

juce::Array<SampleEntry> SampleLibrary::getByCategory(const juce::String& category) const
{
    juce::Array<SampleEntry> result;
    for (const auto& s : samples)
        if (s.category.equalsIgnoreCase(category))
            result.add(s);
    return result;
}

juce::Array<SampleEntry> SampleLibrary::getByPack(const juce::String& pack) const
{
    juce::Array<SampleEntry> result;
    for (const auto& s : samples)
        if (s.pack.equalsIgnoreCase(pack))
            result.add(s);
    return result;
}

juce::StringArray SampleLibrary::getCategories() const
{
    juce::StringArray cats;
    for (const auto& s : samples)
        if (!cats.contains(s.category))
            cats.add(s.category);
    return cats;
}

juce::StringArray SampleLibrary::getPacks() const
{
    juce::StringArray packs;
    for (const auto& s : samples)
        if (!packs.contains(s.pack))
            packs.add(s.pack);
    return packs;
}

juce::File SampleLibrary::resolveFilePath(const SampleEntry& entry) const
{
    return assetsRoot.getChildFile(entry.filePath);
}
