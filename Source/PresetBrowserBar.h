#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"

//==============================================================================
// PresetBrowserBar
//
// A compact header-bar component (sits inside the masthead) that exposes:
//   - A ComboBox listing all saved presets
//   - SAVE / SAVE AS / DELETE buttons
//   - A New Preset dialog (TextEditor overlay)
//
// Owns a PresetManager instance (shared ref to processor’s APVTS).
//==============================================================================
class PresetBrowserBar : public juce::Component,
                         public juce::ComboBox::Listener
{
public:
    explicit PresetBrowserBar(juce::AudioProcessorValueTreeState& apvts);
    ~PresetBrowserBar() override;

    void paint    (juce::Graphics&)     override;
    void resized  ()                    override;
    void comboBoxChanged (juce::ComboBox*) override;

    // Called after an external state change to refresh the list
    void refreshList();

    std::function<void()> onPresetLoaded;

private:
    PresetManager presetManager;

    juce::ComboBox  presetCombo;
    juce::TextButton saveBtn  { "SAVE" };
    juce::TextButton saveAsBtn{ "AS"   };
    juce::TextButton deleteBtn{ "DEL"  };

    void doSave();
    void doSaveAs();
    void doDelete();
    void rebuildCombo();
    void styleButton(juce::TextButton&, bool accent = false);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserBar)
};
