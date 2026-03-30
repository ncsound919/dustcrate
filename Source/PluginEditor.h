#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WaveformDisplay.h"

//==============================================================================
class DustCrateLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DustCrateLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;
    void drawComboBox(juce::Graphics&, int w, int h, bool down,
                      int bx, int by, int bw, int bh,
                      juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void drawLabel(juce::Graphics&, juce::Label&) override;
    void drawPopupMenuBackground(juce::Graphics&, int w, int h) override;
    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>&,
                           bool sep, bool active, bool highlighted, bool ticked,
                           bool sub, const juce::String& text,
                           const juce::String& shortcut,
                           const juce::Drawable*, const juce::Colour*) override;

    static juce::Colour body()        { return juce::Colour(0xff1a1c1e); }
    static juce::Colour panel()       { return juce::Colour(0xff222426); }
    static juce::Colour panelBorder() { return juce::Colour(0xff2e3032); }
    static juce::Colour amber()       { return juce::Colour(0xfff0a020); }
    static juce::Colour amberDim()    { return juce::Colour(0xff8a5a10); }
    static juce::Colour slate()       { return juce::Colour(0xff3a8fcc); }
    static juce::Colour slateDim()    { return juce::Colour(0xff1a4466); }
    static juce::Colour textPri()     { return juce::Colour(0xffe8e0d0); }
    static juce::Colour textSec()     { return juce::Colour(0xff7a7268); }
    static juce::Colour rowSel()      { return juce::Colour(0xff2a2c2e); }
    static juce::Colour knobTrack()   { return juce::Colour(0xff303234); }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DustCrateLookAndFeel)
};

//==============================================================================
class CategoryTagBar : public juce::Component
{
public:
    CategoryTagBar();
    void setCategories(const juce::StringArray& cats);
    void paint(juce::Graphics&) override;
    void mouseUp(const juce::MouseEvent&) override;
    std::function<void(const juce::String&)> onCategorySelected;
    juce::String selectedCategory { "All" };
private:
    juce::StringArray categories;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CategoryTagBar)
};

//==============================================================================
class SampleBrowserList : public juce::Component,
                          public juce::ListBoxModel
{
public:
    SampleBrowserList(DustCrateAudioProcessor&, DustCrateLookAndFeel&);
    void setEntries(const juce::Array<SampleEntry>&);
    int  getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics&,
                          int w, int h, bool sel) override;
    void listBoxItemClicked(int, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked(int, const juce::MouseEvent&) override;
    void resized() override;
    std::function<void(const SampleEntry&)> onSampleSelected;
    std::function<void(const SampleEntry&)> onSampleTriggered;
    int selectedRow { -1 };
private:
    DustCrateAudioProcessor& processor;
    DustCrateLookAndFeel& laf;
    juce::ListBox listBox { "browser", this };
    juce::Array<SampleEntry> entries;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleBrowserList)
};

//==============================================================================
class SectionPanel : public juce::Component
{
public:
    explicit SectionPanel(const juce::String& title, bool slateAccent = false);
    void paint(juce::Graphics&) override;
    void addChildAndMakeVisible(juce::Component& c);
    static constexpr int kHeaderH = 20;
private:
    juce::String title;
    bool slateAccent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionPanel)
};

//==============================================================================
class DustCrateAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit DustCrateAudioProcessorEditor(DustCrateAudioProcessor&);
    ~DustCrateAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DustCrateAudioProcessor& audioProcessor;
    DustCrateLookAndFeel laf;

    // Waveform
    WaveformDisplay waveform;

    // Browsers
    SectionPanel soundsPanel  { "SOUNDS" };
    SectionPanel noisePanel   { "VINYL / NOISE" };
    CategoryTagBar soundTagBar;
    juce::TextEditor searchBox;
    juce::ComboBox   packFilter;
    SampleBrowserList mainList;
    SampleBrowserList noiseList;

    // Envelope
    SectionPanel envelopePanel { "ENVELOPE" };
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label  attackLabel,  decayLabel,  sustainLabel,  releaseLabel;

    // Filter + Pitch
    SectionPanel filterPanel { "FILTER  /  PITCH" };
    juce::Slider filterCutoffSlider, filterResSlider, pitchSlider;
    juce::Label  cutoffLabel, resLabel, pitchLabel;
    juce::ComboBox filterTypeCombo;

    // Character
    SectionPanel characterPanel { "CHARACTER", true };
    juce::Slider noiseLevelSlider, driftSlider, vhsSlider, cassetteSlider;
    juce::Label  noiseLabelKnob, driftLabel, vhsLabel, cassetteLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAttachment> attackAttach, decayAttach, sustainAttach, releaseAttach,
                                      cutoffAttach, resAttach, pitchAttach,
                                      noiseLevelAttach, driftAttach, vhsAttach, cassetteAttach;
    std::unique_ptr<ComboAttachment>  filterTypeAttach;

    void setupKnob(juce::Slider&, juce::Label&, const juce::String&, bool slate = false);
    void styleCombo(juce::ComboBox&);
    void refreshBrowsers();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DustCrateAudioProcessorEditor)
};
