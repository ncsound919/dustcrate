#include "PluginEditor.h"

//==============================================================================
// SampleBrowserList
//==============================================================================

SampleBrowserList::SampleBrowserList(DustCrateAudioProcessor& p)
    : processor(p)
{
    addAndMakeVisible(listBox);
    listBox.setRowHeight(24);
    listBox.setColour(juce::ListBox::backgroundColourId,
                      juce::Colour(0xff151515));
    listBox.setColour(juce::ListBox::outlineColourId,
                      juce::Colour(0xff303030));
}

void SampleBrowserList::setEntries(const juce::Array<SampleEntry>& newEntries)
{
    entries = newEntries;
    listBox.updateContent();
    listBox.repaint();
}

int SampleBrowserList::getNumRows()
{
    return entries.size();
}

void SampleBrowserList::paintListBoxItem(int rowNumber, juce::Graphics& g,
                                         int width, int height,
                                         bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colour(0xff333333));

    g.setColour(juce::Colour(0xffc6b18b)); // warm dusty tone
    g.setFont(13.0f);

    if (juce::isPositiveAndBelow(rowNumber, entries.size()))
    {
        auto text = juce::String("\u25b6  ") + entries[rowNumber].name;
        g.drawText(text, 6, 0, width - 8, height,
                   juce::Justification::centredLeft);
    }
}

void SampleBrowserList::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    if (juce::isPositiveAndBelow(row, entries.size()) && onSampleSelected)
        onSampleSelected(entries[row]);
}

void SampleBrowserList::listBoxItemDoubleClicked(int row,
                                                 const juce::MouseEvent&)
{
    if (juce::isPositiveAndBelow(row, entries.size()) && onSampleTriggered)
        onSampleTriggered(entries[row]);
}

void SampleBrowserList::resized()
{
    listBox.setBounds(getLocalBounds());
}

//==============================================================================
// DustCrateAudioProcessorEditor
//==============================================================================

DustCrateAudioProcessorEditor::DustCrateAudioProcessorEditor(
    DustCrateAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      mainList(p),
      noiseList(p)
{
    setSize(620, 520);
    setResizable(true, true);

    // Filters & search
    addAndMakeVisible(categoryFilter);
    addAndMakeVisible(packFilter);
    addAndMakeVisible(searchBox);

    categoryFilter.addItem("All Categories", 1);
    packFilter.addItem("All Packs", 1);
    searchBox.setTextToShowWhenEmpty("Search crates...",
                                     juce::Colours::grey);

    auto& lib = audioProcessor.getSampleLibrary();
    int id = 2;
    for (auto& cat : lib.getCategories())
        categoryFilter.addItem(cat, id++);
    for (auto& pack : lib.getPacks())
        packFilter.addItem(pack, id++);

    categoryFilter.onChange = [this] { refreshBrowsers(); };
    packFilter.onChange     = [this] { refreshBrowsers(); };
    searchBox.onTextChange  = [this] { refreshBrowsers(); };

    // Browsers
    addAndMakeVisible(mainList);
    addAndMakeVisible(noiseList);

    mainLabel.setJustificationType(juce::Justification::centredLeft);
    mainLabel.setColour(juce::Label::textColourId,
                        juce::Colour(0xffe0b46a));
    addAndMakeVisible(mainLabel);

    noiseLabel.setJustificationType(juce::Justification::centredLeft);
    noiseLabel.setColour(juce::Label::textColourId,
                         juce::Colour(0xffc6b18b));
    addAndMakeVisible(noiseLabel);

    mainList.onSampleTriggered = [this](const SampleEntry& e)
    {
        auto file = audioProcessor.getSampleLibrary().resolveFilePath(e);
        audioProcessor.triggerSample(file.getFullPathName(), e.rootNote, 1.0f);
    };

    noiseList.onSampleTriggered = [this](const SampleEntry& e)
    {
        auto file = audioProcessor.getSampleLibrary().resolveFilePath(e);
        audioProcessor.triggerSample(file.getFullPathName(), e.rootNote, 0.8f);
    };

    // ADSR + filter + pitch controls
    setupSlider(attackSlider,  attackLabel,  "A");
    setupSlider(decaySlider,   decayLabel,   "D");
    setupSlider(sustainSlider, sustainLabel, "S");
    setupSlider(releaseSlider, releaseLabel, "R");
    setupSlider(filterCutoffSlider, cutoffLabel, "Cutoff");
    setupSlider(filterResSlider,    resLabel,    "Res");
    setupSlider(pitchSlider,        pitchLabel,  "Pitch");

    // Noise + character macros
    setupSlider(noiseLevelSlider, noiseLabelKnob, "Noise");
    setupSlider(driftSlider,      driftLabel,     "Drift");
    setupSlider(vhsSlider,        vhsLabel,       "VHS");
    setupSlider(cassetteSlider,   cassetteLabel,  "Cass");

    addAndMakeVisible(filterTypeCombo);
    filterTypeCombo.addItem("Lowpass",  1);
    filterTypeCombo.addItem("Highpass", 2);

    // Attach to APVTS
    auto& apvts = audioProcessor.apvts;
    attackAttach   = std::make_unique<SliderAttachment>(apvts, "attack",        attackSlider);
    decayAttach    = std::make_unique<SliderAttachment>(apvts, "decay",         decaySlider);
    sustainAttach  = std::make_unique<SliderAttachment>(apvts, "sustain",       sustainSlider);
    releaseAttach  = std::make_unique<SliderAttachment>(apvts, "release",       releaseSlider);
    cutoffAttach   = std::make_unique<SliderAttachment>(apvts, "filterCutoff",   filterCutoffSlider);
    resAttach      = std::make_unique<SliderAttachment>(apvts, "filterRes",      filterResSlider);
    pitchAttach    = std::make_unique<SliderAttachment>(apvts, "pitchSemitones", pitchSlider);

    noiseLevelAttach = std::make_unique<SliderAttachment>(apvts, "noiseLevel",    noiseLevelSlider);
    driftAttach      = std::make_unique<SliderAttachment>(apvts, "driftAmount",   driftSlider);
    vhsAttach        = std::make_unique<SliderAttachment>(apvts, "vhsAmount",     vhsSlider);
    cassetteAttach   = std::make_unique<SliderAttachment>(apvts, "cassetteAmount", cassetteSlider);

    filterTypeAttach = std::make_unique<ComboAttachment>(apvts, "filterType", filterTypeCombo);

    refreshBrowsers();
}

DustCrateAudioProcessorEditor::~DustCrateAudioProcessorEditor() {}

void DustCrateAudioProcessorEditor::setupSlider(juce::Slider& s,
                                                juce::Label& l,
                                                const juce::String& text)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.setColour(juce::Slider::rotarySliderFillColourId,
                juce::Colour(0xffe0b46a));
    addAndMakeVisible(s);

    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(10.0f);
    l.setColour(juce::Label::textColourId,
                juce::Colour(0xffc6b18b));
    addAndMakeVisible(l);
}

void DustCrateAudioProcessorEditor::refreshBrowsers()
{
    auto& lib = audioProcessor.getSampleLibrary();
    auto all  = lib.getAllSamples();

    juce::String catSel  = categoryFilter.getText();
    juce::String packSel = packFilter.getText();
    juce::String search  = searchBox.getText().toLowerCase();

    juce::Array<SampleEntry> mainEntries;
    juce::Array<SampleEntry> noiseEntries;

    for (const auto& e : all)
    {
        bool catMatch  = catSel.isEmpty()  || catSel  == "All Categories" || e.category.equalsIgnoreCase(catSel);
        bool packMatch = packSel.isEmpty() || packSel == "All Packs"       || e.pack.equalsIgnoreCase(packSel);
        bool searchMatch = search.isEmpty() || e.name.toLowerCase().contains(search);

        if (! catMatch || ! packMatch || ! searchMatch)
            continue;

        // Treat explicit "noise" category as vinyl/dust/noise candidates
        if (e.category.equalsIgnoreCase("noise"))
            noiseEntries.add(e);
        else
            mainEntries.add(e);
    }

    mainList.setEntries(mainEntries);
    noiseList.setEntries(noiseEntries);
}

void DustCrateAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff101010));

    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff1b1b1b));
    g.fillRoundedRectangle(bounds.reduced(4.0f), 6.0f);

    g.setColour(juce::Colour(0xffe0b46a));
    g.setFont(juce::Font("Courier New", 18.0f, juce::Font::bold));
    g.drawText("DUSTCRATE", 12, 8, 200, 22, juce::Justification::centredLeft);
    g.setFont(10.0f);
    g.setColour(juce::Colour(0xff8a7a5a));
    g.drawText("digital crate for MPC Sample", 12, 26, 260, 14,
               juce::Justification::centredLeft);
}

void DustCrateAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // Top bar for filters/search
    auto top = area.removeFromTop(32);
    categoryFilter.setBounds(top.removeFromLeft(160).reduced(2));
    packFilter.setBounds(top.removeFromLeft(160).reduced(2));
    searchBox.setBounds(top.reduced(2));

    area.removeFromTop(6);

    // Browsers occupy ~55% of remaining height
    int browserHeight = int(area.getHeight() * 0.55f);
    auto browserArea = area.removeFromTop(browserHeight);

    auto mainTitleRow  = browserArea.removeFromTop(18);
    mainLabel.setBounds(mainTitleRow);

    auto mainArea = browserArea.removeFromTop(browserArea.getHeight() / 2);
    mainList.setBounds(mainArea.reduced(0, 2));

    auto noiseTitleRow = browserArea.removeFromTop(18);
    noiseLabel.setBounds(noiseTitleRow);
    noiseList.setBounds(browserArea.reduced(0, 2));

    area.removeFromTop(8);

    // Bottom control strip – 4 ADSR, 2 filter, 1 pitch, 4 macros = 11 slots
    auto controls = area;
    int numSlots = 11;
    int slotWidth = controls.getWidth() / numSlots;

    auto placeKnob = [&](juce::Slider& s, juce::Label& l)
    {
        auto col = controls.removeFromLeft(slotWidth);
        auto labelArea = col.removeFromBottom(14);
        l.setBounds(labelArea);
        s.setBounds(col.reduced(4));
    };

    placeKnob(attackSlider,       attackLabel);
    placeKnob(decaySlider,        decayLabel);
    placeKnob(sustainSlider,      sustainLabel);
    placeKnob(releaseSlider,      releaseLabel);
    placeKnob(filterCutoffSlider, cutoffLabel);
    placeKnob(filterResSlider,    resLabel);
    placeKnob(pitchSlider,        pitchLabel);
    placeKnob(noiseLevelSlider,   noiseLabelKnob);
    placeKnob(driftSlider,        driftLabel);
    placeKnob(vhsSlider,          vhsLabel);
    placeKnob(cassetteSlider,     cassetteLabel);

    // Filter type combo sits over the last two knobs
    filterTypeCombo.setBounds(controls.reduced(4));
}
