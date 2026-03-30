#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Simple list component used for both tone and noise browsers
class SampleBrowserList : public juce::Component,
                          public juce::ListBoxModel
{
public:
    SampleBrowserList(DustCrateAudioProcessor&);

    void setEntries(const juce::Array<SampleEntry>& newEntries);

    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics&, int width, int height,
                          bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;
    void resized() override;

    std::function<void(const SampleEntry&)> onSampleSelected;
    std::function<void(const SampleEntry&)> onSampleTriggered;

private:
    DustCrateAudioProcessor& processor;
    juce::ListBox listBox { "browser", this };
    juce::Array<SampleEntry> entries;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleBrowserList)
};

//==============================================================================
class DustCrateAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DustCrateAudioProcessorEditor(DustCrateAudioProcessor&);
    ~DustCrateAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DustCrateAudioProcessor& audioProcessor;

    // Top filters & search
    juce::ComboBox categoryFilter;
    juce::ComboBox packFilter;
    juce::TextEditor searchBox;

    // Browsers
    SampleBrowserList mainList;
    SampleBrowserList noiseList;
    juce::Label mainLabel { {}, "SOUNDS" };
    juce::Label noiseLabel { {}, "VINYL / NOISE" };

    // Controls
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Slider filterCutoffSlider, filterResSlider, pitchSlider;
    juce::Slider noiseLevelSlider, driftSlider, vhsSlider, cassetteSlider;

    juce::ComboBox filterTypeCombo;

    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    juce::Label cutoffLabel, resLabel, pitchLabel;
    juce::Label noiseLabelKnob, driftLabel, vhsLabel, cassetteLabel;

    // APVTS attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> attackAttach, decayAttach,
                                      sustainAttach, releaseAttach,
                                      cutoffAttach, resAttach, pitchAttach,
                                      noiseLevelAttach, driftAttach,
                                      vhsAttach, cassetteAttach;
    std::unique_ptr<ComboAttachment>  filterTypeAttach;

    void refreshBrowsers();
    void setupSlider(juce::Slider&, juce::Label&, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DustCrateAudioProcessorEditor)
};
