#include "PluginEditor.h"

// ──────────────────────────────────────────────
// SampleBrowserList
// ──────────────────────────────────────────────

SampleBrowserList::SampleBrowserList(DustCrateAudioProcessor& p)
    : processor(p)
{
    addAndMakeVisible(listBox);
    listBox.setModel(this);
    listBox.setRowHeight(28);
    listBox.setColour(juce::ListBox::backgroundColourId,
                     juce::Colour(0xff1a1a1a));
}

void SampleBrowserList::setEntries(const juce::Array<SampleEntry>& newEntries)
{
    entries = newEntries;
    listBox.updateContent();
    listBox.repaint();
}

int SampleBrowserList::getNumRows() { return entries.size(); }

void SampleBrowserList::paintListBoxItem(int row, juce::Graphics& g,
                                          int width, int height, bool selected)
{
    if (selected)
        g.fillAll(juce::Colour(0xff3a3a3a));

    g.setColour(juce::Colour(0xffb0a090)); // warm dusty beige
    g.setFont(13.0f);
    if (row < entries.size())
        g.drawText(juce::String("\u25b6 ") + entries[row].name,
                   6, 0, width - 6, height,
                   juce::Justification::centredLeft);
}

void SampleBrowserList::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    if (row < entries.size() && onSampleSelected)
        onSampleSelected(entries[row]);
}

void SampleBrowserList::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    if (row < entries.size() && onSampleTriggered)
        onSampleTriggered(entries[row]);
}

void SampleBrowserList::resized()
{
    listBox.setBounds(getLocalBounds());
}

// ──────────────────────────────────────────────
// Editor
// ──────────────────────────────────────────────

DustCrateAudioProcessorEditor::DustCrateAudioProcessorEditor(
    DustCrateAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), sampleList(p)
{
    setSize(520, 480);
    setResizable(true, true);

    // Category + Pack filters
    addAndMakeVisible(categoryFilter);
    addAndMakeVisible(packFilter);
    addAndMakeVisible(searchBox);
    categoryFilter.addItem("All Categories", 1);
    packFilter.addItem("All Packs", 1);
    searchBox.setTextToShowWhenEmpty("Search...",
                                     juce::Colours::grey);

    auto& lib = audioProcessor.getSampleLibrary();
    int id = 2;
    for (auto& cat : lib.getCategories())
        categoryFilter.addItem(cat, id++);
    for (auto& pack : lib.getPacks())
        packFilter.addItem(pack, id++);

    categoryFilter.onChange = [this] { refreshBrowser(); };
    packFilter.onChange     = [this] { refreshBrowser(); };
    searchBox.onTextChange  = [this] { refreshBrowser(); };

    addAndMakeVisible(sampleList);
    sampleList.onSampleTriggered = [this](const SampleEntry& e) {
        auto file = audioProcessor.getSampleLibrary().resolveFilePath(e);
        audioProcessor.triggerSample(file.getFullPathName(), e.rootNote, 1.0f);
    };

    // ADSR sliders
    setupSlider(attackSlider,  attackLabel,  "A");
    setupSlider(decaySlider,   decayLabel,   "D");
    setupSlider(sustainSlider, sustainLabel, "S");
    setupSlider(releaseSlider, releaseLabel, "R");
    setupSlider(filterCutoffSlider, cutoffLabel, "Cutoff");
    setupSlider(filterResSlider,    resLabel,    "Res");
    setupSlider(pitchSlider,        pitchLabel,  "Pitch");

    addAndMakeVisible(filterTypeCombo);
    filterTypeCombo.addItem("Lowpass",  1);
    filterTypeCombo.addItem("Highpass", 2);

    // APVTS attachments
    auto& apvts = audioProcessor.apvts;
    attackAttach  = std::make_unique<SliderAttachment>(apvts, "attack",  attackSlider);
    decayAttach   = std::make_unique<SliderAttachment>(apvts, "decay",   decaySlider);
    sustainAttach = std::make_unique<SliderAttachment>(apvts, "sustain", sustainSlider);
    releaseAttach = std::make_unique<SliderAttachment>(apvts, "release", releaseSlider);
    cutoffAttach  = std::make_unique<SliderAttachment>(apvts, "filterCutoff", filterCutoffSlider);
    resAttach     = std::make_unique<SliderAttachment>(apvts, "filterRes",    filterResSlider);
    pitchAttach   = std::make_unique<SliderAttachment>(apvts, "pitchSemitones", pitchSlider);
    filterTypeAttach = std::make_unique<ComboAttachment>(apvts, "filterType", filterTypeCombo);

    refreshBrowser();
}

DustCrateAudioProcessorEditor::~DustCrateAudioProcessorEditor() {}

void DustCrateAudioProcessorEditor::setupSlider(juce::Slider& s,
                                                  juce::Label& l,
                                                  const juce::String& text)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.setColour(juce::Slider::rotarySliderFillColourId,
                juce::Colour(0xffb0803a));
    addAndMakeVisible(s);
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(10.0f);
    l.setColour(juce::Label::textColourId, juce::Colour(0xffb0a090));
    addAndMakeVisible(l);
}

void DustCrateAudioProcessorEditor::refreshBrowser()
{
    auto& lib = audioProcessor.getSampleLibrary();
    auto all  = lib.getAllSamples();

    juce::String catSel  = categoryFilter.getText();
    juce::String packSel = packFilter.getText();
    juce::String search  = searchBox.getText().toLowerCase();

    juce::Array<SampleEntry> filtered;
    for (const auto& e : all)
    {
        bool catMatch  = catSel.isEmpty()  || catSel  == "All Categories" || e.category.equalsIgnoreCase(catSel);
        bool packMatch = packSel.isEmpty() || packSel == "All Packs"       || e.pack.equalsIgnoreCase(packSel);
        bool searchMatch = search.isEmpty() || e.name.toLowerCase().contains(search);
        if (catMatch && packMatch && searchMatch)
            filtered.add(e);
    }
    sampleList.setEntries(filtered);
}

void DustCrateAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121212));
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

    // Title
    g.setColour(juce::Colour(0xffb0803a)); // warm amber
    g.setFont(juce::Font("Courier New", 16.0f, juce::Font::bold));
    g.drawText("DUSTCRATE", 10, 8, 200, 24, juce::Justification::centredLeft);
    g.setFont(9.0f);
    g.setColour(juce::Colour(0xff606060));
    g.drawText("digital crate digger", 10, 26, 200, 14,
               juce::Justification::centredLeft);
}

void DustCrateAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(8);

    // Top bar
    auto topBar = area.removeFromTop(32);
    categoryFilter.setBounds(topBar.removeFromLeft(140).reduced(2));
    packFilter    .setBounds(topBar.removeFromLeft(140).reduced(2));
    searchBox     .setBounds(topBar.reduced(2));

    // Browser list (60% of remaining height)
    int browserH = static_cast<int>(area.getHeight() * 0.58f);
    sampleList.setBounds(area.removeFromTop(browserH));

    area.removeFromTop(6); // spacer

    // Control strip — 8 knobs in a row
    auto controlStrip = area;
    int knobW = controlStrip.getWidth() / 8;
    int knobH = controlStrip.getHeight() - 16;

    auto placeKnob = [&](juce::Slider& s, juce::Label& l)
    {
        auto col = controlStrip.removeFromLeft(knobW);
        l.setBounds(col.removeFromBottom(14));
        s.setBounds(col);
    };

    placeKnob(attackSlider,       attackLabel);
    placeKnob(decaySlider,        decayLabel);
    placeKnob(sustainSlider,      sustainLabel);
    placeKnob(releaseSlider,      releaseLabel);
    placeKnob(filterCutoffSlider, cutoffLabel);
    placeKnob(filterResSlider,    resLabel);
    placeKnob(pitchSlider,        pitchLabel);

    // Filter type combo in last slot
    filterTypeCombo.setBounds(controlStrip.reduced(4));
}
