#pragma once
#include <JuceHeader.h>

//==============================================================================
// PresetManager
//
// Saves and loads DustCrate presets as XML files in the user's app data dir:
//   macOS:   ~/Library/Application Support/Overlay365/DustCrate/Presets/
//   Windows: %APPDATA%\Overlay365\DustCrate\Presets\
//
// Each preset is the full APVTS state serialised to XML, with an added
// <PresetMeta name="..." category="..." author="..." /> child element.
//
// The class also maintains an in-memory list used to populate the preset
// ComboBox in the editor header.
//==============================================================================
class PresetManager
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& apvts);

    //--- Save / Load -------------------------------------------------------
    bool savePreset   (const juce::String& name, const juce::String& category = "User");
    bool loadPreset   (const juce::String& name);
    bool deletePreset (const juce::String& name);

    //--- Discovery ---------------------------------------------------------
    void            refreshPresetList();
    juce::StringArray getPresetNames()  const { return presetNames; }
    juce::String    getCurrentPreset() const { return currentPresetName; }

    //--- Paths -------------------------------------------------------------
    juce::File getPresetsFolder() const;
    juce::File getPresetFile    (const juce::String& name) const;

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::StringArray presetNames;
    juce::String      currentPresetName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
