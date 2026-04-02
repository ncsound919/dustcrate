#include "PresetValidator.h"

ValidationResult PresetValidator::validatePresetXml(
    const juce::XmlElement& xml,
    const juce::Identifier& apvtsStateType)
{
    // 1. Root element type must match the APVTS state identifier.
    if (xml.getTagName() != apvtsStateType.toString())
        return ValidationResult::fail("Preset root element '" + xml.getTagName()
            + "' does not match expected type '" + apvtsStateType.toString() + "'");

    // 2. Schema version must be present and within the supported range.
    if (! xml.hasAttribute("schemaVersion"))
        return ValidationResult::fail("Preset is missing 'schemaVersion' attribute");

    const int version = xml.getIntAttribute("schemaVersion", 0);
    if (version < 1)
        return ValidationResult::fail("Preset schemaVersion must be >= 1, got "
            + juce::String(version));

    if (version > kCurrentPresetSchemaVersion)
        return ValidationResult::fail("Preset schemaVersion " + juce::String(version)
            + " is newer than supported version " + juce::String(kCurrentPresetSchemaVersion));

    // 3. The XML must have at least one child element that is a parameter node
    //    (i.e. not solely PresetMeta).  This guards against empty/corrupt saves.
    bool hasParams = false;
    for (auto* child : xml.getChildIterator())
    {
        if (child != nullptr && child->getTagName() != "PresetMeta"
                             && child->getTagName() != "Extra")
        {
            hasParams = true;
            break;
        }
    }
    if (! hasParams)
        return ValidationResult::fail("Preset contains no parameter data");

    return ValidationResult::pass();
}

ValidationResult PresetValidator::validatePackJson(const juce::var& json)
{
    // 1. Top-level must be an object.
    if (! json.isObject())
        return ValidationResult::fail("Pack JSON root is not an object");

    // 2. packName must be a non-empty string.
    const juce::String packName = json["packName"].toString();
    if (packName.isEmpty())
        return ValidationResult::fail("Pack JSON missing or empty 'packName'");

    // 3. schema_version must be present and within the supported range.
    if (! json.hasProperty("schema_version"))
        return ValidationResult::fail("Pack JSON missing 'schema_version' field");

    const int version = (int) json["schema_version"];
    if (version < 1)
        return ValidationResult::fail("Pack JSON schema_version must be >= 1, got "
            + juce::String(version));

    if (version > kCurrentPackSchemaVersion)
        return ValidationResult::fail("Pack JSON schema_version " + juce::String(version)
            + " is newer than supported version " + juce::String(kCurrentPackSchemaVersion));

    // 4. samples must be an array (may be empty).
    if (! json["samples"].isArray())
        return ValidationResult::fail("Pack JSON missing or invalid 'samples' array");

    // 5. Each sample entry must have a non-empty name and filePath.
    const auto& arr = *json["samples"].getArray();
    for (int i = 0; i < arr.size(); ++i)
    {
        const auto& item = arr[i];
        if (! item.isObject())
            return ValidationResult::fail("samples[" + juce::String(i) + "] is not an object");

        if (item["name"].toString().isEmpty())
            return ValidationResult::fail("samples[" + juce::String(i) + "] missing 'name'");

        if (item["filePath"].toString().isEmpty())
            return ValidationResult::fail("samples[" + juce::String(i) + "] missing 'filePath'");
    }

    return ValidationResult::pass();
}
