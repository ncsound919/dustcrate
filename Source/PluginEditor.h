#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WaveformDisplay.h"
#include "PresetBrowserBar.h"
#include "SamplePreview.h"
#include "PackImportWizard.h"

//==============================================================================
class DustCrateLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DustCrateLookAndFeel();
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    void drawComboBox(juce::Graphics&,int,int,bool,int,int,int,int,juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void drawLabel(juce::Graphics&,juce::Label&) override;
    void drawPopupMenuBackground(juce::Graphics&,int,int) override;
    void drawPopupMenuItem(juce::Graphics&,const juce::Rectangle<int>&,
                           bool,bool,bool,bool,bool,const juce::String&,
                           const juce::String&,const juce::Drawable*,
                           const juce::Colour*) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    juce::Font getTextButtonFont(juce::TextButton&,int) override;

    static juce::Colour body()        { return juce::Colour(0xff141517); }
    static juce::Colour panel()       { return juce::Colour(0xff1e2022); }
    static juce::Colour panelBorder() { return juce::Colour(0xff2a2c2f); }
    static juce::Colour amber()       { return juce::Colour(0xffc8921a); }
    static juce::Colour amberDim()    { return juce::Colour(0xff6b4a0a); }
    static juce::Colour amberGlow()   { return juce::Colour(0x22c8921a); }
    static juce::Colour slate()       { return juce::Colour(0xff3a8fcc); }
    static juce::Colour slateDim()    { return juce::Colour(0xff1a4466); }
    static juce::Colour textPri()     { return juce::Colour(0xffe4dcc8); }
    static juce::Colour textSec()     { return juce::Colour(0xff6a6258); }
    static juce::Colour rowSel()      { return juce::Colour(0xff282a2c); }
    static juce::Colour knobTrack()   { return juce::Colour(0xff2c2e30); }
    static juce::Colour knobBody()    { return juce::Colour(0xff252729); }
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
class SampleBrowserList : public juce::Component, public juce::ListBoxModel
{
public:
    SampleBrowserList(DustCrateAudioProcessor&, DustCrateLookAndFeel&);
    void setEntries(const juce::Array<SampleEntry>&);
    int getNumRows() override;
    void paintListBoxItem(int,juce::Graphics&,int,int,bool) override;
    void listBoxItemClicked(int,const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked(int,const juce::MouseEvent&) override;
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
class LFOPanel : public juce::Component
{
public:
    LFOPanel();
    void resized() override;
    void paint(juce::Graphics&) override;
    juce::Slider    rateSlider, depthSlider;
    juce::ComboBox  shapeCombo, targetCombo;
    juce::Label     rateLabel,  depthLabel;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFOPanel)
};

//==============================================================================
class DustCrateAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                       public juce::FileDragAndDropTarget
{
public:
    explicit DustCrateAudioProcessorEditor(DustCrateAudioProcessor&);
    ~DustCrateAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag(const juce::StringArray& f) override { return packWizard.isInterestedInFileDrag(f); }
    void filesDropped(const juce::StringArray& f, int x, int y) override { packWizard.filesDropped(f, x, y); }

    // Exposed for KeyboardListener
    DustCrateAudioProcessor& audioProcessor;
    juce::String currentFilePath;
    int          currentRootNote { 60 };

private:
    DustCrateLookAndFeel laf;

    // Typeface
    juce::Typeface::Ptr velumStrokeTypeface;

    // ---- Top menu buttons ----
    juce::TextButton menuFileBtn     { "FILE" };
    juce::TextButton menuSettingsBtn { "SETTINGS" };

    // ---- Preset bar ----
    PresetBrowserBar presetBar { audioProcessor.apvts };

    // ---- Waveform ----
    WaveformDisplay waveform;

    // ---- Browsers ----
    SectionPanel soundsPanel { "SOUNDS" };
    SectionPanel noisePanel  { "VINYL / NOISE" };
    CategoryTagBar soundTagBar;
    CategoryTagBar noiseTagBar;
    juce::TextEditor searchBox;
    juce::ComboBox   packFilter;
    SampleBrowserList mainList;
    SampleBrowserList noiseList;

    // ---- Sample preview ----
    SamplePreview samplePreview;
    juce::Slider  previewTrimSlider;
    juce::Label   previewTrimLabel;

    // ---- Pack import ----
    PackImportWizard packWizard { audioProcessor.getSampleLibrary() };

    // ---- Envelope ----
    SectionPanel envelopePanel { "ENVELOPE" };
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label  attackLabel,  decayLabel,  sustainLabel,  releaseLabel;

    // ---- Filter / Pitch ----
    SectionPanel filterPanel { "FILTER / PITCH" };
    juce::Slider filterCutoffSlider, filterResSlider, pitchSlider;
    juce::Label  cutoffLabel, resLabel, pitchLabel;
    // LP / HP / BP toggle buttons (replace hidden combo)
    juce::TextButton filterLpBtn { "LP" };
    juce::TextButton filterHpBtn { "HP" };
    juce::TextButton filterBpBtn { "BP" };
    juce::ComboBox   filterTypeCombo;  // kept for APVTS attachment

    // ---- Character ----
    SectionPanel characterPanel { "CHARACTER", true };
    juce::Slider noiseLevelSlider, driftSlider, vhsSlider, cassetteSlider;
    juce::Label  noiseLabelKnob,   driftLabel,  vhsLabel,  cassetteLabel;
    LFOPanel     lfoPanel;

    // ---- On-screen keyboard ----
    juce::MidiKeyboardState     keyboardState;
    juce::MidiKeyboardComponent keyboard { keyboardState,
        juce::MidiKeyboardComponent::horizontalKeyboard };

    struct KeyboardListener : public juce::MidiKeyboardStateListener
    {
        DustCrateAudioProcessorEditor& ed;
        explicit KeyboardListener(DustCrateAudioProcessorEditor& e) : ed(e) {}
        void handleNoteOn(juce::MidiKeyboardState*, int, int note, float vel) override
        {
            if (!ed.currentFilePath.isEmpty())
                ed.audioProcessor.triggerSample(ed.currentFilePath, note, vel);
        }
        void handleNoteOff(juce::MidiKeyboardState*, int, int /*note*/, float) override
        {
            ed.audioProcessor.stopAllVoices();
        }
    };
    std::unique_ptr<KeyboardListener> keyboardListener;

    // ---- APVTS attachments ----
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAttachment>
        attackAttach, decayAttach, sustainAttach, releaseAttach,
        cutoffAttach, resAttach,   pitchAttach,
        noiseLevelAttach, driftAttach, vhsAttach, cassetteAttach,
        lfoRateAttach, lfoDepthAttach;
    std::unique_ptr<ComboAttachment>
        filterTypeAttach, lfoShapeAttach, lfoTargetAttach;

    // ---- Helpers ----
    void setupKnob    (juce::Slider&, juce::Label&, const juce::String&, bool slate = false);
    void mouseDown    (const juce::MouseEvent&) override;
    void styleCombo   (juce::ComboBox&);
    void updateFilterButtons();
    void refreshBrowsers();
    void refreshNoiseTags();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DustCrateAudioProcessorEditor)
};
