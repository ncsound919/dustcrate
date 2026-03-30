#include "PresetManager.h"

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& ap)
    : apvts(ap)
{
    getPresetsFolder().createDirectory();
    refreshPresetList();
}

juce::File PresetManager::getPresetsFolder() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("Overlay365")
               .getChildFile("DustCrate")
               .getChildFile("Presets");
}

juce::File PresetManager::getPresetFile(const juce::String& name) const
{
    return getPresetsFolder().getChildFile(name + ".xml");
}

bool PresetManager::savePreset(const juce::String& name, const juce::String& category)
{
    if (name.isEmpty()) return false;

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml == nullptr) return false;

    // Attach metadata
    auto* meta = xml->createNewChildElement("PresetMeta");
    meta->setAttribute("name",     name);
    meta->setAttribute("category", category);
    meta->setAttribute("author",   juce::SystemStats::getFullUserName());
    meta->setAttribute("created",  juce::Time::getCurrentTime().toString(true, true));

    if (! xml->writeTo(getPresetFile(name))) return false;

    refreshPresetList();
    currentPresetName = name;
    return true;
}

bool PresetManager::loadPreset(const juce::String& name)
{
    const juce::File f = getPresetFile(name);
    if (! f.existsAsFile()) return false;

    std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(f));
    if (xml == nullptr) return false;

    // Strip metadata child before restoring (APVTS doesn't know about it)
    xml->removeChildElement(xml->getChildByName("PresetMeta"), true);

    const auto tree = juce::ValueTree::fromXml(*xml);
    if (tree.isValid() && tree.hasType(apvts.state.getType()))
    {
        apvts.replaceState(tree);
        currentPresetName = name;
        return true;
    }
    return false;
}

bool PresetManager::deletePreset(const juce::String& name)
{
    const bool ok = getPresetFile(name).deleteFile();
    if (ok) refreshPresetList();
    return ok;
}

void PresetManager::refreshPresetList()
{
    presetNames.clear();
    for (const auto& f : getPresetsFolder().findChildFiles(
             juce::File::findFiles, false, "*.xml"))
        presetNames.add(f.getFileNameWithoutExtension());
    presetNames.sort(true);
}
