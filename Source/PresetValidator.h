#pragma once
#include <JuceHeader.h>

//==============================================================================
/** Result of a validation pass, containing a success flag and an error message. */
struct ValidationResult
{
    bool    ok    { true };
    juce::String errorMessage;

    static ValidationResult pass() { return { true, {} }; }
    static ValidationResult fail(const juce::String& msg) { return { false, msg }; }
};

//==============================================================================
/**
 * @brief Validates preset XML files and sample-pack JSON data before use.
 *
 * PresetValidator is a stateless utility class (all methods are static) that
 * checks structural integrity and schema version compliance.  It is called by
 * PresetManager before accepting a preset file, and by SampleLibrary before
 * parsing a JSON pack.
 *
 * ### Preset XML schema (schema_version 1)
 * - Root element type must match the APVTS state type ("Parameters").
 * - Root must carry a @c schemaVersion attribute (integer ≥ 1).
 * - At least one APVTS parameter child must be present.
 *
 * ### Pack JSON schema (schema_version 1)
 * - Top-level object with @c packName (non-empty string).
 * - @c schema_version integer field present and ≥ 1.
 * - @c samples array present (may be empty but must exist).
 * - Each entry in @c samples must have non-empty @c name and @c filePath.
 */
class PresetValidator
{
public:
    /** Current schema version written by PresetManager::savePreset(). */
    static constexpr int kCurrentPresetSchemaVersion = 1;

    /** Current schema version expected in pack JSON files. */
    static constexpr int kCurrentPackSchemaVersion = 1;

    //--------------------------------------------------------------------------
    /**
     * @brief Validate a preset XML element before loading into APVTS.
     *
     * @param xml           The root XmlElement parsed from the preset file.
     * @param apvtsStateType The APVTS state type to validate against (e.g. "Parameters").
     * @return ValidationResult with @c ok == true on success.
     */
    [[nodiscard]] static ValidationResult validatePresetXml(
        const juce::XmlElement& xml,
        const juce::Identifier& apvtsStateType);

    //--------------------------------------------------------------------------
    /**
     * @brief Validate a parsed pack JSON var before ingesting entries.
     *
     * @param json  The top-level juce::var parsed from the pack JSON file.
     * @return ValidationResult with @c ok == true on success.
     */
    [[nodiscard]] static ValidationResult validatePackJson(const juce::var& json);

private:
    PresetValidator() = delete;
};
