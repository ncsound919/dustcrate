#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// DustCrateLookAndFeel
// Custom LookAndFeel that gives DustCrate the MPC-cousin hardware aesthetic:
//  – Charcoal/graphite body panels
//  – Amber accent for active/selected states
//  – Slate-blue accent for the CHARACTER strip
//  – Helvetica-style sans-serif labels, monospaced header
//  – Knobs: flat, matte, no chrome (machined aluminum feel)
//==============================================================================
class DustCrateLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DustCrateLookAndFeel();

    // Rotary knobs
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override;

    // ComboBox – flush, no chrome border
    void drawComboBox(juce::Graphics& g,
                      int width, int height,
                      bool isButtonDown,
                      int buttonX, int buttonY,
                      int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    juce::Font getComboBoxFont(juce::ComboBox&) override;

    // ListBox rows – handled externally via paintListBoxItem
    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    // PopupMenu
    void drawPopupMenuBackground(juce::Graphics& g, int w, int h) override;
    void drawPopupMenuItem(juce::Graphics& g,
                           const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon,
                           const juce::Colour* textColour) override;

    // Colour tokens
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
    static juce::Colour rowHover()    { return juce::Colour(0xff252729); }
    static juce::Colour knobTrack()   { return juce::Colour(0xff303234); }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DustCrateLookAndFeel)
};

//==============================================================================
// CategoryTagBar – horizontal scroll of pill-shaped category tags
//==============================================================================
class CategoryTagBar : public juce::Component
{
public:
    CategoryTagBar();
    void setCategories(const juce::StringArray& cats);
    void paint(juce::Graphics& g) override;
    void mouseUp(const juce::MouseEvent& e) override;

    std::function<void(const juce::String&)> onCategorySelected;
    juce::String selectedCategory;

private:
    juce::StringArray categories;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CategoryTagBar)
};

//==============================================================================
// SampleBrowserList
//==============================================================================
class SampleBrowserList : public juce::Component,
                          public juce::ListBoxModel
{
public:
    SampleBrowserList(DustCrateAudioProcessor&, DustCrateLookAndFeel&);

    void setEntries(const juce::Array<SampleEntry>& newEntries);

    int  getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics&,
                          int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;
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
// SectionPanel – a rounded-rect panel with a header label and hairline top bar
//==============================================================================
class SectionPanel : public juce::Component
{
public:
    explicit SectionPanel(const juce::String& title,
                          bool useSlateAccent = false);
    void paint(juce::Graphics& g) override;
    void addChildComponent(juce::Component& c);
    void addChildAndMakeVisible(juce::Component& c);

    static constexpr int kHeaderH = 20;

private:
    juce::String title;
    bool slateAccent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionPanel)
};

//==============================================================================
// DustCrateAudioProcessorEditor
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

    //=== Browser section ===================================================
    SectionPanel soundsPanel  { "SOUNDS" };
    SectionPanel noisePanel   { "VINYL / NOISE", false };

    CategoryTagBar   soundTagBar;
    juce::TextEditor searchBox;
    juce::ComboBox   packFilter;

    SampleBrowserList mainList;
    SampleBrowserList noiseList;

    //=== Envelope section ==================================================
    SectionPanel envelopePanel { "ENVELOPE" };
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label  attackLabel, decayLabel, sustainLabel, releaseLabel;

    //=== Filter + Pitch section ============================================
    SectionPanel filterPanel { "FILTER  /  PITCH" };
    juce::Slider filterCutoffSlider, filterResSlider, pitchSlider;
    juce::Label  cutoffLabel, resLabel, pitchLabel;
    juce::ComboBox filterTypeCombo;

    //=== Character section (slate accent) ==================================
    SectionPanel characterPanel { "CHARACTER", true };
    juce::Slider noiseLevelSlider, driftSlider, vhsSlider, cassetteSlider;
    juce::Label  noiseLabelKnob, driftLabel, vhsLabel, cassetteLabel;

    //=== APVTS attachments =================================================
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> attackAttach, decayAttach,
                                      sustainAttach, releaseAttach,
                                      cutoffAttach, resAttach, pitchAttach,
                                      noiseLevelAttach, driftAttach,
                                      vhsAttach, cassetteAttach;
    std::unique_ptr<ComboAttachment>  filterTypeAttach;

    //=== Helpers ===========================================================
    void setupKnob(juce::Slider&, juce::Label&,
                   const juce::String& text, bool slateAccent = false);
    void refreshBrowsers();
    void styleCombo(juce::ComboBox&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DustCrateAudioProcessorEditor)
};
