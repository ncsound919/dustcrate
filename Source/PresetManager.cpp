#include "PresetManager.h"
#include "PresetValidator.h"

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
    // FIX: sanitise name to avoid path traversal (strip slashes, dots)
    const juce::String safe = name.replaceCharacters("\\/:..", "_____");
    return getPresetsFolder().getChildFile(safe + ".xml");
}

bool PresetManager::savePreset(const juce::String& name, const juce::String& category)
{
    if (name.isEmpty()) return false;
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml == nullptr) return false;

    // Stamp the current schema version on every saved preset.
    xml->setAttribute("schemaVersion", kCurrentSchemaVersion);

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

    // Remove PresetMeta before validation so the validator only sees param data.
    if (auto* meta = xml->getChildByName("PresetMeta"))
        xml->removeChildElement(meta, true);

    // Migrate schema if the preset was saved by an older version.
    const int fileVersion = xml->getIntAttribute("schemaVersion", 0);
    if (fileVersion > 0 && fileVersion < kCurrentSchemaVersion)
        migratePreset(*xml, fileVersion);

    // Validate structure and schema version.
    if (! xml->hasAttribute("schemaVersion"))
        xml->setAttribute("schemaVersion", 1);

    const auto validation = PresetValidator::validatePresetXml(*xml, apvts.state.getType());
    if (! validation.ok)
    {
        DBG("PresetManager: validation failed for '" + name + "' — " + validation.errorMessage);
        return false;
    }

    // Strip the schemaVersion attribute before restoring so the APVTS
    // ValueTree is not polluted with our bookkeeping attribute.
    xml->removeAttribute("schemaVersion");

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
    if (ok) { if (currentPresetName == name) currentPresetName.clear(); refreshPresetList(); }
    return ok;
}

void PresetManager::refreshPresetList()
{
    presetNames.clear();
    for (const auto& f : getPresetsFolder().findChildFiles(juce::File::findFiles, false, "*.xml"))
        presetNames.add(f.getFileNameWithoutExtension());
    presetNames.sort(true);
}

void PresetManager::migratePreset(juce::XmlElement& xml, int fromVersion)
{
    // Migration stubs — add cases here for each future schema bump.
    // Example (hypothetical v1 → v2 migration):
    //   if (fromVersion < 2) { /* rename attribute "oldKey" → "newKey" */ }
    juce::ignoreUnused(xml, fromVersion);
}
