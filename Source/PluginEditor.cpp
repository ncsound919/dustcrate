#include "PluginEditor.h"

//==============================================================================
// DustCrateLookAndFeel
//==============================================================================

DustCrateLookAndFeel::DustCrateLookAndFeel()
{
    // Combo popups inherit body colours
    setColour(juce::PopupMenu::backgroundColourId,   panel());
    setColour(juce::PopupMenu::textColourId,         textPri());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, rowSel());
    setColour(juce::PopupMenu::highlightedTextColourId,       amber());

    setColour(juce::TextEditor::backgroundColourId,  panel());
    setColour(juce::TextEditor::outlineColourId,     panelBorder());
    setColour(juce::TextEditor::textColourId,        textPri());
    setColour(juce::TextEditor::focusedOutlineColourId, amber());
    setColour(juce::TextEditor::highlightColourId,   amberDim());
    setColour(juce::CaretComponent::caretColourId,   amber());
}

void DustCrateLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                             int x, int y, int w, int h,
                                             float pos,
                                             float startA, float endA,
                                             juce::Slider& slider)
{
    const bool isSlate = slider.getProperties()["slateAccent"];
    const juce::Colour accentCol = isSlate ? slate() : amber();
    const juce::Colour dimCol    = isSlate ? slateDim() : amberDim();

    const float cx = x + w * 0.5f;
    const float cy = y + h * 0.5f;
    const float r  = juce::jmin(w, h) * 0.38f;
    const float angle = startA + pos * (endA - startA);

    // Track arc
    juce::Path trackArc;
    trackArc.addCentredArc(cx, cy, r, r, 0.0f, startA, endA, true);
    g.setColour(knobTrack());
    g.strokePath(trackArc, juce::PathStrokeType(2.5f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc
    juce::Path valueArc;
    valueArc.addCentredArc(cx, cy, r, r, 0.0f, startA, angle, true);
    g.setColour(accentCol);
    g.strokePath(valueArc, juce::PathStrokeType(2.5f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knob body
    const float knobR = r * 0.68f;
    g.setColour(juce::Colour(0xff2c2e30));
    g.fillEllipse(cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f);

    // Subtle inner shadow ring
    g.setColour(juce::Colour(0xff161718));
    g.drawEllipse(cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f, 1.0f);

    // Pointer dot
    const float dotDist = knobR * 0.62f;
    const float dotX = cx + dotDist * std::sin(angle);
    const float dotY = cy - dotDist * std::cos(angle);
    g.setColour(accentCol);
    g.fillEllipse(dotX - 2.2f, dotY - 2.2f, 4.4f, 4.4f);
}

void DustCrateLookAndFeel::drawComboBox(juce::Graphics& g,
                                         int w, int h,
                                         bool /*down*/,
                                         int /*bx*/, int /*by*/,
                                         int /*bw*/, int /*bh*/,
                                         juce::ComboBox& /*box*/)
{
    g.setColour(panel());
    g.fillRoundedRectangle(0.0f, 0.0f, (float) w, (float) h, 3.0f);
    g.setColour(panelBorder());
    g.drawRoundedRectangle(0.5f, 0.5f, w - 1.0f, h - 1.0f, 3.0f, 1.0f);

    // Chevron
    const float arrowX = w - 14.0f;
    const float arrowY = h * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath(arrowX,       arrowY - 2.5f);
    arrow.lineTo(arrowX + 5.0f, arrowY + 2.5f);
    arrow.lineTo(arrowX + 10.0f, arrowY - 2.5f);
    g.setColour(textSec());
    g.strokePath(arrow, juce::PathStrokeType(1.2f, juce::PathStrokeType::mitered,
                                              juce::PathStrokeType::rounded));
}

juce::Font DustCrateLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font("Helvetica Neue", 11.5f, juce::Font::plain);
}

void DustCrateLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.findColour(juce::Label::backgroundColourId));
    g.fillRoundedRectangle(label.getLocalBounds().toFloat(), 2.0f);
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(label.getFont());
    g.drawText(label.getText(), label.getLocalBounds().reduced(2, 0),
               label.getJustificationType());
}

void DustCrateLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int w, int h)
{
    g.setColour(panel());
    g.fillRoundedRectangle(0.0f, 0.0f, (float) w, (float) h, 4.0f);
    g.setColour(panelBorder());
    g.drawRoundedRectangle(0.5f, 0.5f, w - 1.0f, h - 1.0f, 4.0f, 0.8f);
}

void DustCrateLookAndFeel::drawPopupMenuItem(juce::Graphics& g,
                                              const juce::Rectangle<int>& area,
                                              bool isSeparator, bool isActive,
                                              bool isHighlighted, bool /*isTicked*/,
                                              bool /*hasSubMenu*/,
                                              const juce::String& text,
                                              const juce::String& /*shortcut*/,
                                              const juce::Drawable* /*icon*/,
                                              const juce::Colour* /*textColour*/)
{
    if (isSeparator)
    {
        g.setColour(panelBorder());
        g.fillRect(area.getX() + 4, area.getCentreY(), area.getWidth() - 8, 1);
        return;
    }
    if (isHighlighted)
    {
        g.setColour(rowSel());
        g.fillRoundedRectangle(area.reduced(2, 1).toFloat(), 3.0f);
    }
    g.setColour(isActive ? (isHighlighted ? amber() : textPri()) : textSec());
    g.setFont(juce::Font("Helvetica Neue", 11.5f, juce::Font::plain));
    g.drawText(text, area.reduced(8, 0), juce::Justification::centredLeft);
}

//==============================================================================
// CategoryTagBar
//==============================================================================

CategoryTagBar::CategoryTagBar()
{
    setRepaintsOnMouseActivity(true);
}

void CategoryTagBar::setCategories(const juce::StringArray& cats)
{
    categories.clear();
    categories.add("All");
    for (auto& c : cats)
        categories.addIfNotAlreadyThere(c);
    selectedCategory = "All";
    repaint();
}

void CategoryTagBar::paint(juce::Graphics& g)
{
    g.fillAll(DustCrateLookAndFeel::body());
    const int h  = getHeight();
    const int ph = h - 4;
    int x = 2;

    for (const auto& cat : categories)
    {
        const bool sel = cat.equalsIgnoreCase(selectedCategory);
        const int tw = 8 + juce::Font("Helvetica Neue", 10.5f,
                             juce::Font::plain).getStringWidth(cat) + 8;

        juce::Rectangle<float> pill(x, 2.0f, tw, ph);

        if (sel)
        {
            g.setColour(DustCrateLookAndFeel::amber());
            g.fillRoundedRectangle(pill, ph * 0.5f);
            g.setColour(DustCrateLookAndFeel::body());
        }
        else
        {
            g.setColour(DustCrateLookAndFeel::panelBorder());
            g.drawRoundedRectangle(pill, ph * 0.5f, 1.0f);
            g.setColour(DustCrateLookAndFeel::textSec());
        }

        g.setFont(juce::Font("Helvetica Neue", 10.5f, juce::Font::plain));
        g.drawText(cat, (int) pill.getX(), (int) pill.getY(),
                   (int) pill.getWidth(), (int) pill.getHeight(),
                   juce::Justification::centred, false);

        x += tw + 4;
    }
}

void CategoryTagBar::mouseUp(const juce::MouseEvent& e)
{
    const int h  = getHeight();
    const int ph = h - 4;
    int x = 2;

    for (const auto& cat : categories)
    {
        const int tw = 8 + juce::Font("Helvetica Neue", 10.5f,
                             juce::Font::plain).getStringWidth(cat) + 8;
        const juce::Rectangle<int> pill(x, 2, tw, ph);

        if (pill.contains(e.getPosition()))
        {
            selectedCategory = cat;
            repaint();
            if (onCategorySelected)
                onCategorySelected(cat.equalsIgnoreCase("All") ? juce::String() : cat);
            return;
        }
        x += tw + 4;
    }
}

//==============================================================================
// SampleBrowserList
//==============================================================================

SampleBrowserList::SampleBrowserList(DustCrateAudioProcessor& p,
                                     DustCrateLookAndFeel& l)
    : processor(p), laf(l)
{
    addAndMakeVisible(listBox);
    listBox.setRowHeight(26);
    listBox.setColour(juce::ListBox::backgroundColourId,
                      DustCrateLookAndFeel::panel());
    listBox.setColour(juce::ListBox::outlineColourId,
                      juce::Colours::transparentBlack);
    listBox.setLookAndFeel(&laf);
}

void SampleBrowserList::setEntries(const juce::Array<SampleEntry>& e)
{
    entries = e;
    listBox.updateContent();
    listBox.repaint();
}

int SampleBrowserList::getNumRows() { return entries.size(); }

void SampleBrowserList::paintListBoxItem(int row, juce::Graphics& g,
                                          int w, int h, bool sel)
{
    if (sel)
    {
        g.setColour(DustCrateLookAndFeel::rowSel());
        g.fillRect(0, 0, w, h);
        // Amber left accent bar
        g.setColour(DustCrateLookAndFeel::amber());
        g.fillRect(0, 2, 2, h - 4);
    }

    if (! juce::isPositiveAndBelow(row, entries.size()))
        return;

    const auto& entry = entries[row];

    // Category tag badge (right side)
    const juce::String badge = entry.category.toUpperCase().substring(0, 4);
    const int badgeW = 32;
    const int badgeX = w - badgeW - 6;
    g.setColour(sel ? DustCrateLookAndFeel::amberDim()
                    : DustCrateLookAndFeel::panelBorder());
    g.fillRoundedRectangle((float) badgeX, 5.0f, (float) badgeW, (float)(h - 10), 2.0f);
    g.setColour(sel ? DustCrateLookAndFeel::amber()
                    : DustCrateLookAndFeel::textSec());
    g.setFont(juce::Font("Helvetica Neue", 8.5f, juce::Font::plain));
    g.drawText(badge, badgeX, 0, badgeW, h, juce::Justification::centred);

    // Name
    g.setColour(sel ? DustCrateLookAndFeel::textPri()
                    : DustCrateLookAndFeel::textPri().withAlpha(0.75f));
    g.setFont(juce::Font("Helvetica Neue", 12.0f, juce::Font::plain));
    g.drawText(entry.name, 10, 0, badgeX - 14, h,
               juce::Justification::centredLeft);
}

void SampleBrowserList::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    selectedRow = row;
    if (juce::isPositiveAndBelow(row, entries.size()) && onSampleSelected)
        onSampleSelected(entries[row]);
}

void SampleBrowserList::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    selectedRow = row;
    if (juce::isPositiveAndBelow(row, entries.size()) && onSampleTriggered)
        onSampleTriggered(entries[row]);
}

void SampleBrowserList::resized()
{
    listBox.setBounds(getLocalBounds());
}

//==============================================================================
// SectionPanel
//==============================================================================

SectionPanel::SectionPanel(const juce::String& t, bool slate)
    : title(t), slateAccent(slate) {}

void SectionPanel::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // Panel body
    g.setColour(DustCrateLookAndFeel::panel());
    g.fillRoundedRectangle(b, 5.0f);

    // Border
    g.setColour(DustCrateLookAndFeel::panelBorder());
    g.drawRoundedRectangle(b.reduced(0.5f), 5.0f, 0.8f);

    // Header accent bar (top edge)
    const juce::Colour accent = slateAccent ? DustCrateLookAndFeel::slate()
                                            : DustCrateLookAndFeel::amber();
    juce::Path topBar;
    topBar.addRoundedRectangle(b.getX(), b.getY(), b.getWidth(), 3.0f,
                               3.0f, 3.0f, true, true, false, false);
    g.setColour(accent);
    g.fillPath(topBar);

    // Header label
    g.setColour(slateAccent ? DustCrateLookAndFeel::slate()
                            : DustCrateLookAndFeel::amber());
    g.setFont(juce::Font("Helvetica Neue", 9.5f, juce::Font::bold));
    g.drawText(title,
               (int) b.getX() + 8, (int) b.getY() + 3,
               (int) b.getWidth() - 16, kHeaderH - 3,
               juce::Justification::centredLeft);
}

void SectionPanel::addChildComponent(juce::Component& c)
{
    juce::Component::addChildComponent(c);
}

void SectionPanel::addChildAndMakeVisible(juce::Component& c)
{
    juce::Component::addAndMakeVisible(c);
}

//==============================================================================
// DustCrateAudioProcessorEditor
//==============================================================================

DustCrateAudioProcessorEditor::DustCrateAudioProcessorEditor(
    DustCrateAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      mainList(p, laf),
      noiseList(p, laf)
{
    setLookAndFeel(&laf);
    setSize(660, 540);
    setResizable(true, true);

    //=== Section panels ====================================================
    addAndMakeVisible(soundsPanel);
    addAndMakeVisible(noisePanel);
    addAndMakeVisible(envelopePanel);
    addAndMakeVisible(filterPanel);
    addAndMakeVisible(characterPanel);

    //=== Sounds browser setup ==============================================
    soundsPanel.addChildAndMakeVisible(soundTagBar);
    soundsPanel.addChildAndMakeVisible(searchBox);
    soundsPanel.addChildAndMakeVisible(packFilter);
    soundsPanel.addChildAndMakeVisible(mainList);

    searchBox.setTextToShowWhenEmpty("Search", DustCrateLookAndFeel::textSec());
    searchBox.onTextChange = [this] { refreshBrowsers(); };
    styleCombo(packFilter);
    packFilter.addItem("All Packs", 1);

    auto& lib = audioProcessor.getSampleLibrary();
    soundTagBar.setCategories(lib.getCategories());
    soundTagBar.onCategorySelected = [this](const juce::String&) { refreshBrowsers(); };

    int pid = 2;
    for (auto& pk : lib.getPacks())
        packFilter.addItem(pk, pid++);
    packFilter.setSelectedId(1);
    packFilter.onChange = [this] { refreshBrowsers(); };

    mainList.onSampleTriggered = [this](const SampleEntry& e)
    {
        auto file = audioProcessor.getSampleLibrary().resolveFilePath(e);
        audioProcessor.triggerSample(file.getFullPathName(), e.rootNote, 1.0f);
    };

    //=== Noise browser setup ===============================================
    noisePanel.addChildAndMakeVisible(noiseList);

    noiseList.onSampleTriggered = [this](const SampleEntry& e)
    {
        auto file = audioProcessor.getSampleLibrary().resolveFilePath(e);
        audioProcessor.triggerSample(file.getFullPathName(), e.rootNote, 0.75f);
    };

    //=== Envelope section ==================================================
    setupKnob(attackSlider,  attackLabel,  "ATK");
    setupKnob(decaySlider,   decayLabel,   "DCY");
    setupKnob(sustainSlider, sustainLabel, "SUS");
    setupKnob(releaseSlider, releaseLabel, "REL");
    envelopePanel.addChildAndMakeVisible(attackSlider);
    envelopePanel.addChildAndMakeVisible(attackLabel);
    envelopePanel.addChildAndMakeVisible(decaySlider);
    envelopePanel.addChildAndMakeVisible(decayLabel);
    envelopePanel.addChildAndMakeVisible(sustainSlider);
    envelopePanel.addChildAndMakeVisible(sustainLabel);
    envelopePanel.addChildAndMakeVisible(releaseSlider);
    envelopePanel.addChildAndMakeVisible(releaseLabel);

    //=== Filter + Pitch section ============================================
    setupKnob(filterCutoffSlider, cutoffLabel, "CUTOFF");
    setupKnob(filterResSlider,    resLabel,    "RES");
    setupKnob(pitchSlider,        pitchLabel,  "PITCH");
    styleCombo(filterTypeCombo);
    filterTypeCombo.addItem("Lowpass",  1);
    filterTypeCombo.addItem("Highpass", 2);
    filterTypeCombo.setSelectedId(1);
    filterPanel.addChildAndMakeVisible(filterCutoffSlider);
    filterPanel.addChildAndMakeVisible(cutoffLabel);
    filterPanel.addChildAndMakeVisible(filterResSlider);
    filterPanel.addChildAndMakeVisible(resLabel);
    filterPanel.addChildAndMakeVisible(pitchSlider);
    filterPanel.addChildAndMakeVisible(pitchLabel);
    filterPanel.addChildAndMakeVisible(filterTypeCombo);

    //=== Character section =================================================
    setupKnob(noiseLevelSlider, noiseLabelKnob, "NOISE", true);
    setupKnob(driftSlider,      driftLabel,     "DRIFT", true);
    setupKnob(vhsSlider,        vhsLabel,       "VHS",   true);
    setupKnob(cassetteSlider,   cassetteLabel,  "CASS",  true);
    characterPanel.addChildAndMakeVisible(noiseLevelSlider);
    characterPanel.addChildAndMakeVisible(noiseLabelKnob);
    characterPanel.addChildAndMakeVisible(driftSlider);
    characterPanel.addChildAndMakeVisible(driftLabel);
    characterPanel.addChildAndMakeVisible(vhsSlider);
    characterPanel.addChildAndMakeVisible(vhsLabel);
    characterPanel.addChildAndMakeVisible(cassetteSlider);
    characterPanel.addChildAndMakeVisible(cassetteLabel);

    //=== APVTS attachments =================================================
    auto& apvts = audioProcessor.apvts;
    attackAttach     = std::make_unique<SliderAttachment>(apvts, "attack",         attackSlider);
    decayAttach      = std::make_unique<SliderAttachment>(apvts, "decay",          decaySlider);
    sustainAttach    = std::make_unique<SliderAttachment>(apvts, "sustain",        sustainSlider);
    releaseAttach    = std::make_unique<SliderAttachment>(apvts, "release",        releaseSlider);
    cutoffAttach     = std::make_unique<SliderAttachment>(apvts, "filterCutoff",   filterCutoffSlider);
    resAttach        = std::make_unique<SliderAttachment>(apvts, "filterRes",      filterResSlider);
    pitchAttach      = std::make_unique<SliderAttachment>(apvts, "pitchSemitones", pitchSlider);
    noiseLevelAttach = std::make_unique<SliderAttachment>(apvts, "noiseLevel",     noiseLevelSlider);
    driftAttach      = std::make_unique<SliderAttachment>(apvts, "driftAmount",    driftSlider);
    vhsAttach        = std::make_unique<SliderAttachment>(apvts, "vhsAmount",      vhsSlider);
    cassetteAttach   = std::make_unique<SliderAttachment>(apvts, "cassetteAmount", cassetteSlider);
    filterTypeAttach = std::make_unique<ComboAttachment> (apvts, "filterType",     filterTypeCombo);

    refreshBrowsers();
}

DustCrateAudioProcessorEditor::~DustCrateAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    noiseList.setLookAndFeel(nullptr);
    mainList.setLookAndFeel(nullptr);
}

void DustCrateAudioProcessorEditor::setupKnob(juce::Slider& s, juce::Label& l,
                                               const juce::String& text,
                                               bool slateAccent)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.getProperties().set("slateAccent", slateAccent);

    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font("Helvetica Neue", 9.0f, juce::Font::plain));
    l.setColour(juce::Label::textColourId,
                slateAccent ? DustCrateLookAndFeel::slate()
                            : DustCrateLookAndFeel::textSec());
    l.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
}

void DustCrateAudioProcessorEditor::styleCombo(juce::ComboBox& c)
{
    c.setColour(juce::ComboBox::backgroundColourId, DustCrateLookAndFeel::panel());
    c.setColour(juce::ComboBox::textColourId,       DustCrateLookAndFeel::textPri());
    c.setColour(juce::ComboBox::arrowColourId,      DustCrateLookAndFeel::textSec());
    c.setColour(juce::ComboBox::outlineColourId,    DustCrateLookAndFeel::panelBorder());
}

void DustCrateAudioProcessorEditor::refreshBrowsers()
{
    auto& lib  = audioProcessor.getSampleLibrary();
    auto all   = lib.getAllSamples();

    const juce::String cat    = soundTagBar.selectedCategory.equalsIgnoreCase("All")
                                    ? juce::String()
                                    : soundTagBar.selectedCategory;
    const juce::String pack   = packFilter.getText() == "All Packs"
                                    ? juce::String()
                                    : packFilter.getText();
    const juce::String search = searchBox.getText().toLowerCase();

    juce::Array<SampleEntry> mainEntries, noiseEntries;

    for (const auto& e : all)
    {
        const bool catMatch    = cat.isEmpty()    || e.category.equalsIgnoreCase(cat);
        const bool packMatch   = pack.isEmpty()   || e.pack.equalsIgnoreCase(pack);
        const bool searchMatch = search.isEmpty() || e.name.toLowerCase().contains(search);

        if (! catMatch || ! packMatch || ! searchMatch)
            continue;

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
    g.fillAll(DustCrateLookAndFeel::body());

    // Full-width masthead strip
    const int mastheadH = 44;
    g.setColour(juce::Colour(0xff161819));
    g.fillRect(0, 0, getWidth(), mastheadH);

    // Amber hairline under masthead
    g.setColour(DustCrateLookAndFeel::amber().withAlpha(0.35f));
    g.fillRect(0, mastheadH - 1, getWidth(), 1);

    // Wordmark
    g.setFont(juce::Font("Courier New", 20.0f, juce::Font::bold));
    g.setColour(DustCrateLookAndFeel::amber());
    g.drawText("DUSTCRATE", 14, 0, 200, mastheadH - 2,
               juce::Justification::centredLeft);

    // Sub-label
    g.setFont(juce::Font("Helvetica Neue", 9.5f, juce::Font::plain));
    g.setColour(DustCrateLookAndFeel::textSec());
    g.drawText("DIGITAL CRATE  \xb7  MPC SAMPLE COMPANION",
               14, 24, 320, 14, juce::Justification::centredLeft);

    // Version badge (top-right)
    g.setColour(DustCrateLookAndFeel::panelBorder());
    g.fillRoundedRectangle((float)(getWidth() - 48), 14.0f, 38.0f, 16.0f, 3.0f);
    g.setColour(DustCrateLookAndFeel::textSec());
    g.setFont(juce::Font("Helvetica Neue", 9.0f, juce::Font::plain));
    g.drawText("v0.1", getWidth() - 48, 14, 38, 16, juce::Justification::centred);
}

void DustCrateAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(44); // masthead
    area.reduce(8, 8);

    const int bottomH   = 100; // control strip height
    const int dividerH  = 6;

    //=== Browser row (top ~60%) ============================================
    auto browserRow = area.removeFromTop(
        area.getHeight() - bottomH - dividerH);
    area.removeFromTop(dividerH);

    // Sounds panel (left ~60%) and noise panel (right ~40%)
    const int soundsW = int(browserRow.getWidth() * 0.60f);
    auto soundsBounds = browserRow.removeFromLeft(soundsW).reduced(2);
    auto noiseBounds  = browserRow.reduced(2);

    soundsPanel.setBounds(soundsBounds);
    noisePanel.setBounds(noiseBounds);

    // Lay out sounds panel internals
    {
        auto inner = soundsBounds.reduced(6);
        inner.removeFromTop(SectionPanel::kHeaderH);

        auto filterRow = inner.removeFromTop(22);
        packFilter.setBounds(filterRow.removeFromRight(110).reduced(0, 1));
        filterRow.removeFromRight(4);
        searchBox.setBounds(filterRow.reduced(0, 1));
        inner.removeFromTop(4);

        soundTagBar.setBounds(inner.removeFromTop(22));
        inner.removeFromTop(4);
        mainList.setBounds(inner);
    }

    // Lay out noise panel internals
    {
        auto inner = noiseBounds.reduced(6);
        inner.removeFromTop(SectionPanel::kHeaderH);
        noiseList.setBounds(inner);
    }

    //=== Control strip (bottom) ============================================
    auto controls = area;
    const int numPanels  = 3;
    const int panelGap   = 6;
    const int totalGaps  = (numPanels - 1) * panelGap;
    const int panelW_env = int(controls.getWidth() * 0.30f);
    const int panelW_fil = int(controls.getWidth() * 0.28f);
    const int panelW_chr = controls.getWidth() - panelW_env - panelW_fil - totalGaps;

    auto envBounds = controls.removeFromLeft(panelW_env);
    controls.removeFromLeft(panelGap);
    auto filBounds = controls.removeFromLeft(panelW_fil);
    controls.removeFromLeft(panelGap);
    auto chrBounds = controls;

    envelopePanel.setBounds(envBounds);
    filterPanel.setBounds(filBounds);
    characterPanel.setBounds(chrBounds);

    //--- Place knobs inside panels -----------------------------------------
    auto placeKnobRow = [](juce::Rectangle<int> inner,
                           std::initializer_list<std::pair<juce::Slider*, juce::Label*>> items)
    {
        inner.removeFromTop(SectionPanel::kHeaderH + 2);
        const int n = (int) items.size();
        const int slotW = inner.getWidth() / n;
        for (auto& [s, l] : items)
        {
            auto col = inner.removeFromLeft(slotW);
            l->setBounds(col.removeFromBottom(14));
            s->setBounds(col.reduced(4));
        }
    };

    placeKnobRow(envBounds.reduced(4), {
        { &attackSlider,  &attackLabel  },
        { &decaySlider,   &decayLabel   },
        { &sustainSlider, &sustainLabel },
        { &releaseSlider, &releaseLabel },
    });

    {
        auto inner = filBounds.reduced(4);
        inner.removeFromTop(SectionPanel::kHeaderH + 2);
        const int n = 3;
        const int slotW = inner.getWidth() / n;

        for (auto [s, l] : std::initializer_list<std::pair<juce::Slider*, juce::Label*>>{
            { &filterCutoffSlider, &cutoffLabel },
            { &filterResSlider,    &resLabel    },
            { &pitchSlider,        &pitchLabel  } })
        {
            auto col = inner.removeFromLeft(slotW);
            l->setBounds(col.removeFromBottom(14));
            s->setBounds(col.reduced(4));
        }

        // Filter type combo under the knobs, centered
        filterTypeCombo.setBounds(
            filBounds.reduced(6).removeFromBottom(22));
    }

    placeKnobRow(chrBounds.reduced(4), {
        { &noiseLevelSlider, &noiseLabelKnob },
        { &driftSlider,      &driftLabel     },
        { &vhsSlider,        &vhsLabel       },
        { &cassetteSlider,   &cassetteLabel  },
    });
}
