#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SampleBrowserList : public juce::Component,
                          public juce::ListBoxModel
{
public:
    SampleBrowserList(DustCrateAudioProcessor& p);

    void setEntries(const juce::Array<SampleEntry>& newEntries);
    int  getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g,
                          int width, int height, bool selected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;

    std::function<void(const SampleEntry&)> onSampleSelected;
    std::function<void(const SampleEntry&)> onSampleTriggered;

private:
    DustCrateAudioProcessor& processor;
    juce::Array<SampleEntry> entries;
    juce::ListBox listBox;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleBrowserList)
};

class DustCrateAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DustCrateAudioProcessorEditor(DustCrateAudioProcessor&);
    ~DustCrateAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DustCrateAudioProcessor& audioProcessor;

    // Browser
    juce::ComboBox categoryFilter;
    juce::ComboBox packFilter;
    juce::TextEditor searchBox;
    SampleBrowserList sampleList;

    // Controls
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Slider filterCutoffSlider, filterResSlider, pitchSlider;
    juce::ComboBox filterTypeCombo;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    juce::Label cutoffLabel, resLabel, pitchLabel;

    // APVTS attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAttachment> attackAttach, decayAttach,
                                      sustainAttach, releaseAttach,
                                      cutoffAttach, resAttach, pitchAttach;
    std::unique_ptr<ComboAttachment>  filterTypeAttach;

    void refreshBrowser();
    void setupSlider(juce::Slider& s, juce::Label& l,
                     const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DustCrateAudioProcessorEditor)
};
