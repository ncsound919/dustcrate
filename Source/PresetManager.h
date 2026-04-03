#pragma once
#include <JuceHeader.h>

//==============================================================================
/**
 * @brief Manages saving, loading and listing of DustCrate preset files.
 *
 * Presets are stored as XML files in the user's application-data directory:
 * - macOS:   ~/Library/Application Support/Overlay365/DustCrate/Presets/
 * - Windows: %APPDATA%\Overlay365\DustCrate\Presets\
 *
 * Each preset is the full APVTS state serialised to XML, with two extra
 * child elements added by this class:
 * - @c \<PresetMeta name="..." category="..." author="..." created="..." />
 * - A @c schemaVersion attribute on the root element for forward-compat
 *   migration.
 *
 * ### Schema versioning
 * The current schema version is @c kCurrentSchemaVersion.  loadPreset()
 * will call migratePreset() when it encounters an older version, allowing
 * forward-compatible upgrades without data loss.
 */
class PresetManager
{
public:
    /** Schema version written into every saved preset. */
    static constexpr int kCurrentSchemaVersion = 1;

    explicit PresetManager(juce::AudioProcessorValueTreeState& apvts);

    //--- Save / Load -------------------------------------------------------

    /** Save the current APVTS state as @p name.  Returns false on error. */
    [[nodiscard]] bool savePreset   (const juce::String& name,
                                     const juce::String& category = "User");

    /**
     * @brief Load a preset by name into the APVTS.
     *
     * The XML is validated (via PresetValidator) and migrated to the current
     * schema version before being applied.  Returns false if the file is
     * missing, invalid, or fails validation.
     */
    [[nodiscard]] bool loadPreset   (const juce::String& name);

    /** Delete a preset file.  Returns false if the file could not be removed. */
    [[nodiscard]] bool deletePreset (const juce::String& name);

    //--- Discovery ---------------------------------------------------------

    /** Re-scan the presets folder and refresh the in-memory name list. */
    void            refreshPresetList();

    /** Returns the sorted list of known preset names (without extension). */
    juce::StringArray getPresetNames()  const { return presetNames; }

    /** Returns the name of the last successfully loaded or saved preset. */
    juce::String    getCurrentPreset() const { return currentPresetName; }

    //--- Paths -------------------------------------------------------------

    /** Returns the folder where presets are stored (created on first access). */
    juce::File getPresetsFolder() const;

    /** Returns the full path of the XML file for @p name. */
    juce::File getPresetFile    (const juce::String& name) const;

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::StringArray presetNames;
    juce::String      currentPresetName;

    /**
     * @brief Apply schema migrations to bring @p xml up to
     *        @c kCurrentSchemaVersion.
     *
     * Each migration step is idempotent and non-destructive.  Currently a
     * no-op for v1 → v1, but the pattern is in place for future upgrades.
     */
    static void migratePreset (juce::XmlElement& xml, int fromVersion);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
