#include "PluginEditor.h"

//==============================================================================
// DustCrateLookAndFeel
//==============================================================================
DustCrateLookAndFeel::DustCrateLookAndFeel()
{
    setColour(juce::PopupMenu::backgroundColourId,             panel());
    setColour(juce::PopupMenu::textColourId,                   textPri());
    setColour(juce::PopupMenu::highlightedBackgroundColourId,  rowSel());
    setColour(juce::PopupMenu::highlightedTextColourId,        amber());
    setColour(juce::TextEditor::backgroundColourId,            panel());
    setColour(juce::TextEditor::outlineColourId,               panelBorder());
    setColour(juce::TextEditor::textColourId,                  textPri());
    setColour(juce::TextEditor::focusedOutlineColourId,        amber());
    setColour(juce::TextEditor::highlightColourId,             amberDim());
    setColour(juce::CaretComponent::caretColourId,             amber());
    setColour(juce::Slider::thumbColourId,                     amber());
    setColour(juce::Slider::trackColourId,                     amberDim());
    setColour(juce::Slider::backgroundColourId,                knobTrack());
    setColour(juce::TextButton::buttonColourId,                panel());
    setColour(juce::TextButton::buttonOnColourId,              amberDim());
    setColour(juce::TextButton::textColourOffId,               textSec());
    setColour(juce::TextButton::textColourOnId,                amber());
    setColour(juce::MidiKeyboardComponent::whiteNoteColourId,          juce::Colour(0xffe8e2d0));
    setColour(juce::MidiKeyboardComponent::blackNoteColourId,          juce::Colour(0xff1a1c1e));
    setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId,   juce::Colour(0xff2a2c2f));
    setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId,     amber());
    setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,amber().withAlpha(0.4f));
    setColour(juce::MidiKeyboardComponent::textLabelColourId,          textSec());
    setColour(juce::MidiKeyboardComponent::upDownButtonArrowColourId,  textSec());
    setColour(juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, panel());
}

void DustCrateLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int w, int h, float pos, float startA, float endA, juce::Slider& slider)
{
    const bool isSlate = (bool)(int)slider.getProperties()["slateAccent"];
    const auto accent  = isSlate ? slate() : amber();
    const float cx = x + w * .5f, cy = y + h * .5f;
    const float r  = juce::jmin(w, h) * .38f;
    const float angle = startA + pos * (endA - startA);

    // Track arc
    juce::Path track;
    track.addCentredArc(cx,cy,r,r,0,startA,endA,true);
    g.setColour(knobTrack());
    g.strokePath(track, juce::PathStrokeType(2.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));

    // Value arc
    juce::Path val;
    val.addCentredArc(cx,cy,r,r,0,startA,angle,true);
    g.setColour(accent);
    g.strokePath(val, juce::PathStrokeType(2.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));

    // Arc endpoint glow
    const float ex = cx + r * std::sin(angle);
    const float ey = cy - r * std::cos(angle);
    g.setColour(accent.withAlpha(0.22f));
    g.fillEllipse(ex - 5.f, ey - 5.f, 10.f, 10.f);
    g.setColour(accent);
    g.fillEllipse(ex - 2.5f, ey - 2.5f, 5.f, 5.f);

    // Knob body — gradient for depth
    const float kr = r * .68f;
    juce::ColourGradient kg(juce::Colour(0xff353739), cx - kr * .4f, cy - kr * .4f,
                             knobBody(), cx + kr * .4f, cy + kr * .4f, true);
    g.setGradientFill(kg);
    g.fillEllipse(cx-kr, cy-kr, kr*2, kr*2);

    // Knob ring
    g.setColour(juce::Colour(0xff101112));
    g.drawEllipse(cx-kr, cy-kr, kr*2, kr*2, 1.0f);

    // Indicator dot
    const float dd = kr * .62f;
    g.setColour(accent);
    g.fillEllipse(cx + dd*std::sin(angle) - 2.2f,
                  cy - dd*std::cos(angle) - 2.2f, 4.4f, 4.4f);

    // Specular highlight
    g.setColour(juce::Colours::white.withAlpha(0.07f));
    g.fillEllipse(cx - kr*.55f, cy - kr*.65f, kr*.5f, kr*.35f);
}

void DustCrateLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    g.setColour(panel());       g.fillRoundedRectangle(0,0,(float)w,(float)h,4);
    g.setColour(panelBorder()); g.drawRoundedRectangle(.5f,.5f,w-1.f,h-1.f,4,.8f);
    // Left amber accent line
    g.setColour(amber().withAlpha(0.55f));
    g.fillRoundedRectangle(0,2,2,(float)(h-4),1);
    // Chevron
    const float ax=w-14, ay=h*.5f;
    juce::Path arr;
    arr.startNewSubPath(ax,ay-2.5f); arr.lineTo(ax+5,ay+2.5f); arr.lineTo(ax+10,ay-2.5f);
    g.setColour(textSec()); g.strokePath(arr, juce::PathStrokeType(1.2f));
}
juce::Font DustCrateLookAndFeel::getComboBoxFont(juce::ComboBox&)
{ return juce::Font("Helvetica Neue",11.5f,juce::Font::plain); }

void DustCrateLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& l)
{
    g.setColour(l.findColour(juce::Label::backgroundColourId));
    g.fillRoundedRectangle(l.getLocalBounds().toFloat(),2);
    g.setColour(l.findColour(juce::Label::textColourId));
    g.setFont(l.getFont());
    g.drawText(l.getText(), l.getLocalBounds().reduced(2,0), l.getJustificationType());
}

void DustCrateLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int w, int h)
{
    g.setColour(panel()); g.fillRoundedRectangle(0,0,(float)w,(float)h,4);
    g.setColour(panelBorder()); g.drawRoundedRectangle(.5f,.5f,w-1.f,h-1.f,4,.8f);
}

void DustCrateLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
    bool sep, bool active, bool hi, bool, bool, const juce::String& text,
    const juce::String&, const juce::Drawable*, const juce::Colour*)
{
    if (sep) { g.setColour(panelBorder()); g.fillRect(area.getX()+4,area.getCentreY(),area.getWidth()-8,1); return; }
    if (hi)
    {
        g.setColour(rowSel()); g.fillRoundedRectangle(area.reduced(2,1).toFloat(),3);
        g.setColour(amber()); g.fillRect(area.getX()+2, area.getY()+2, 2, area.getHeight()-4);
    }
    g.setColour(active ? (hi ? amber() : textPri()) : textSec());
    g.setFont(juce::Font("Helvetica Neue",11.5f,juce::Font::plain));
    g.drawText(text, area.reduced(10,0), juce::Justification::centredLeft);
}

void DustCrateLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& btn,
    const juce::Colour&, bool isHighlighted, bool isDown)
{
    auto b = btn.getLocalBounds().toFloat().reduced(0.5f);
    const bool on = btn.getToggleState();
    if (on || isDown)
    {
        g.setColour(amberDim()); g.fillRoundedRectangle(b, 3);
        g.setColour(amber());    g.drawRoundedRectangle(b, 3, 1.0f);
    }
    else
    {
        g.setColour(isHighlighted ? panel().brighter(0.1f) : panel());
        g.fillRoundedRectangle(b, 3);
        g.setColour(panelBorder()); g.drawRoundedRectangle(b, 3, 0.8f);
    }
}

juce::Font DustCrateLookAndFeel::getTextButtonFont(juce::TextButton&, int)
{ return juce::Font("Helvetica Neue", 9.5f, juce::Font::plain); }

//==============================================================================
// CategoryTagBar
//==============================================================================
CategoryTagBar::CategoryTagBar() { setRepaintsOnMouseActivity(true); }
void CategoryTagBar::setCategories(const juce::StringArray& cats)
{
    categories.clear(); categories.add("All");
    for (auto& c : cats) categories.addIfNotAlreadyThere(c);
    selectedCategory = "All"; repaint();
}
void CategoryTagBar::paint(juce::Graphics& g)
{
    g.fillAll(DustCrateLookAndFeel::body());
    const int ph = getHeight()-4; int x = 2;
    for (const auto& cat : categories)
    {
        const bool sel = cat.equalsIgnoreCase(selectedCategory);
        const int tw = 16 + (int)juce::Font("Helvetica Neue",10.5f,juce::Font::plain).getStringWidthFloat(cat);
        juce::Rectangle<float> pill((float)x, 2, (float)tw, (float)ph);
        if (sel)
        {
            // Outer glow
            g.setColour(DustCrateLookAndFeel::amber().withAlpha(0.2f));
            g.fillRoundedRectangle(pill.expanded(1.5f), ph*.5f+1);
            g.setColour(DustCrateLookAndFeel::amber());
            g.fillRoundedRectangle(pill, ph*.5f);
            g.setColour(DustCrateLookAndFeel::body());
        }
        else
        {
            g.setColour(DustCrateLookAndFeel::panel().withAlpha(0.5f));
            g.fillRoundedRectangle(pill, ph*.5f);
            g.setColour(DustCrateLookAndFeel::panelBorder());
            g.drawRoundedRectangle(pill, ph*.5f, 1);
            g.setColour(DustCrateLookAndFeel::textSec());
        }
        g.setFont(juce::Font("Helvetica Neue",10.5f, sel ? juce::Font::bold : juce::Font::plain));
        g.drawText(cat,(int)pill.getX(),(int)pill.getY(),(int)pill.getWidth(),(int)pill.getHeight(),juce::Justification::centred);
        x += tw + 4;
    }
}
void CategoryTagBar::mouseUp(const juce::MouseEvent& e)
{
    const int ph = getHeight()-4; int x = 2;
    for (const auto& cat : categories)
    {
        const int tw = 16 + (int)juce::Font("Helvetica Neue",10.5f,juce::Font::plain).getStringWidthFloat(cat);
        if (juce::Rectangle<int>(x,2,tw,ph).contains(e.getPosition()))
        { selectedCategory=cat; repaint(); if(onCategorySelected) onCategorySelected(cat.equalsIgnoreCase("All")?juce::String():cat); return; }
        x += tw + 4;
    }
}

//==============================================================================
// SampleBrowserList
//==============================================================================
SampleBrowserList::SampleBrowserList(DustCrateAudioProcessor& p, DustCrateLookAndFeel& l)
    : processor(p), laf(l)
{
    addAndMakeVisible(listBox);
    listBox.setRowHeight(26);
    listBox.setColour(juce::ListBox::backgroundColourId, DustCrateLookAndFeel::panel());
    listBox.setLookAndFeel(&laf);
}
void SampleBrowserList::setEntries(const juce::Array<SampleEntry>& e) { entries=e; listBox.updateContent(); listBox.repaint(); }
int  SampleBrowserList::getNumRows() { return entries.size(); }
void SampleBrowserList::paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool sel)
{
    // Row background
    if (sel)
    {
        g.setColour(DustCrateLookAndFeel::rowSel()); g.fillRect(0,0,w,h);
        g.setColour(DustCrateLookAndFeel::amberGlow()); g.fillRect(0,0,w,h);
        g.setColour(DustCrateLookAndFeel::amber());  g.fillRect(0,2,3,h-4);
    }
    // Separator
    g.setColour(DustCrateLookAndFeel::panelBorder().withAlpha(0.4f));
    g.fillRect(0, h-1, w, 1);

    if (!juce::isPositiveAndBelow(row, entries.size())) return;
    const auto& e = entries[row];

    // Play triangle
    const float ty = h * 0.5f;
    juce::Path tri;
    tri.startNewSubPath(10.f, ty - 5.f);
    tri.lineTo(10.f, ty + 5.f);
    tri.lineTo(16.f, ty);
    tri.closeSubPath();
    g.setColour(DustCrateLookAndFeel::amber().withAlpha(sel ? 1.0f : 0.35f));
    g.fillPath(tri);

    // Sample name
    const juce::String badge = e.subcategory.isEmpty() ? e.category.toUpperCase().substring(0,4)
                                                        : e.subcategory.substring(0,4);
    const int bw=32, bx=w-bw-6;
    g.setColour(sel ? DustCrateLookAndFeel::amberDim() : DustCrateLookAndFeel::panelBorder());
    g.fillRoundedRectangle((float)bx, 5, (float)bw, (float)(h-10), 2);
    g.setColour(sel ? DustCrateLookAndFeel::amber() : DustCrateLookAndFeel::textSec());
    g.setFont(juce::Font("Helvetica Neue", 8.5f, juce::Font::plain));
    g.drawText(badge, bx, 0, bw, h, juce::Justification::centred);

    g.setColour(sel ? DustCrateLookAndFeel::textPri() : DustCrateLookAndFeel::textPri().withAlpha(0.75f));
    g.setFont(juce::Font("Courier New", 11.5f, juce::Font::plain));
    g.drawText(e.name, 24, 0, bx-28, h, juce::Justification::centredLeft);
}
void SampleBrowserList::listBoxItemClicked(int row, const juce::MouseEvent&)
{ selectedRow=row; if(juce::isPositiveAndBelow(row,entries.size())&&onSampleSelected) onSampleSelected(entries[row]); }
void SampleBrowserList::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{ selectedRow=row; if(juce::isPositiveAndBelow(row,entries.size())&&onSampleTriggered) onSampleTriggered(entries[row]); }
void SampleBrowserList::resized() { listBox.setBounds(getLocalBounds()); }

//==============================================================================
// SectionPanel
//==============================================================================
SectionPanel::SectionPanel(const juce::String& t, bool s) : title(t), slateAccent(s) {}
void SectionPanel::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const auto accent = slateAccent ? DustCrateLookAndFeel::slate() : DustCrateLookAndFeel::amber();

    // Panel background
    g.setColour(DustCrateLookAndFeel::panel()); g.fillRoundedRectangle(b, 6);
    g.setColour(DustCrateLookAndFeel::panelBorder()); g.drawRoundedRectangle(b.reduced(.5f), 6, .8f);

    // Header gradient
    juce::ColourGradient hg(accent.withAlpha(0.18f), b.getX(), b.getY(),
                             juce::Colours::transparentBlack, b.getRight(), b.getY(), false);
    g.setGradientFill(hg);
    g.fillRoundedRectangle(b.getX(), b.getY(), b.getWidth(), (float)kHeaderH, 5);

    // Top accent line (2px)
    g.setColour(accent.withAlpha(0.8f));
    g.fillRoundedRectangle(b.getX()+1, b.getY(), b.getWidth()-2, 2, 1);

    // Title shadow then title
    g.setFont(juce::Font("Helvetica Neue", 9.5f, juce::Font::bold));
    g.setColour(juce::Colour(0x44000000));
    g.drawText(title, (int)b.getX()+9, (int)b.getY()+4, (int)b.getWidth()-16, kHeaderH-4, juce::Justification::centredLeft);
    g.setColour(accent);
    g.drawText(title, (int)b.getX()+8, (int)b.getY()+3, (int)b.getWidth()-16, kHeaderH-4, juce::Justification::centredLeft);
}
void SectionPanel::addChildAndMakeVisible(juce::Component& c) { addAndMakeVisible(c); }

//==============================================================================
// LFOPanel
//==============================================================================
LFOPanel::LFOPanel()
{
    addAndMakeVisible(rateSlider);  addAndMakeVisible(depthSlider);
    addAndMakeVisible(shapeCombo);  addAndMakeVisible(targetCombo);
    addAndMakeVisible(rateLabel);   addAndMakeVisible(depthLabel);

    rateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    rateSlider.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    rateSlider.getProperties().set("slateAccent",true);

    depthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    depthSlider.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    depthSlider.getProperties().set("slateAccent",true);

    auto makeLbl = [](juce::Label& l, const char* t)
    {
        l.setText(t, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setFont(juce::Font("Helvetica Neue",9,juce::Font::plain));
        l.setColour(juce::Label::textColourId,       DustCrateLookAndFeel::slate());
        l.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    };
    makeLbl(rateLabel,  "RATE");
    makeLbl(depthLabel, "DEPTH");

    for (auto* cb : {&shapeCombo, &targetCombo})
    {
        cb->setColour(juce::ComboBox::backgroundColourId, DustCrateLookAndFeel::panel());
        cb->setColour(juce::ComboBox::textColourId,       DustCrateLookAndFeel::slate());
        cb->setColour(juce::ComboBox::arrowColourId,      DustCrateLookAndFeel::slateDim());
        cb->setColour(juce::ComboBox::outlineColourId,    DustCrateLookAndFeel::panelBorder());
    }
    shapeCombo.addItem("Sine",1); shapeCombo.addItem("Triangle",2);
    shapeCombo.addItem("Saw",3);  shapeCombo.addItem("Square",4);
    shapeCombo.setSelectedId(1);
    targetCombo.addItem("None",1);     targetCombo.addItem("Noise",2);
    targetCombo.addItem("Drift",3);    targetCombo.addItem("VHS",4);
    targetCombo.addItem("Cassette",5); targetCombo.setSelectedId(1);
}
void LFOPanel::paint(juce::Graphics& g)
{
    juce::ColourGradient bg(DustCrateLookAndFeel::slateDim().withAlpha(0.3f), 0, 0,
                             juce::Colours::transparentBlack, getWidth(), getHeight(), false);
    g.setGradientFill(bg); g.fillRoundedRectangle(getLocalBounds().toFloat(), 5);
    g.setColour(DustCrateLookAndFeel::slate().withAlpha(0.35f));
    g.drawRoundedRectangle(getLocalBounds().reduced(1).toFloat(), 5, 0.6f);
    g.setColour(DustCrateLookAndFeel::slate());
    g.setFont(juce::Font("Helvetica Neue", 8.5f, juce::Font::bold));
    g.drawText("LFO", 4, 2, 30, 12, juce::Justification::centredLeft);
}
void LFOPanel::resized()
{
    auto area = getLocalBounds().reduced(4);
    area.removeFromTop(14);
    const int kw = area.getHeight()/2;
    auto combos = area.removeFromRight(area.getWidth()-kw*2-4);
    auto lc = area.removeFromLeft(kw); auto dc = area;
    rateLabel.setBounds(lc.removeFromBottom(12));  rateSlider.setBounds(lc.reduced(2));
    depthLabel.setBounds(dc.removeFromBottom(12)); depthSlider.setBounds(dc.reduced(2));
    combos.removeFromTop(4);
    shapeCombo.setBounds(combos.removeFromTop(combos.getHeight()/2).reduced(0,2));
    targetCombo.setBounds(combos.reduced(0,2));
}

//==============================================================================
// DustCrateAudioProcessorEditor
//==============================================================================
DustCrateAudioProcessorEditor::DustCrateAudioProcessorEditor(DustCrateAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), mainList(p,laf), noiseList(p,laf)
{
    setLookAndFeel(&laf);
    setSize(800, 660);
    setResizable(true, true);

    // ---- Load VelumStroke typeface from BinaryData if compiled in ----
#if defined(BinaryData_VelumStroke_ttf)
    velumStrokeTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::VelumStroke_ttf, BinaryData::VelumStroke_ttfSize);
#endif

    // ---- Menu buttons ----
    addAndMakeVisible(menuFileBtn);
    addAndMakeVisible(menuSettingsBtn);
    menuFileBtn.setLookAndFeel(&laf);
    menuSettingsBtn.setLookAndFeel(&laf);

    menuFileBtn.onClick = [this]
    {
        juce::PopupMenu m;
        m.addItem(1, "Load Preset...");
        m.addItem(2, "Save Preset...");
        m.addSeparator();
        m.addItem(3, "Import Pack...");
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&menuFileBtn),
            [this](int r)
            {
                if (r == 1)
                {
                    juce::FileChooser fc("Load Preset",
                        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                        "*.dcpreset;*.xml");
                    if (fc.browseForFileToOpen())
                    {
                        juce::XmlDocument doc(fc.getResult());
                        if (auto xml = doc.getDocumentElement())
                        {
                            auto tree = juce::ValueTree::fromXml(*xml);
                            if (tree.isValid()) audioProcessor.apvts.replaceState(tree);
                        }
                    }
                }
                else if (r == 2)
                {
                    juce::FileChooser fc("Save Preset",
                        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                        "*.dcpreset");
                    if (fc.browseForFileToSave(true))
                    {
                        auto f = fc.getResult().withFileExtension(".dcpreset");
                        if (auto xml = audioProcessor.apvts.copyState().createXml())
                            xml->writeTo(f);
                    }
                }
                else if (r == 3)
                {
                    packWizard.launchImportDialog();
                }
            });
    };

    menuSettingsBtn.onClick = [this]
    {
        juce::PopupMenu m;
        m.addItem(1, "MIDI Learn Mode");
        m.addItem(2, "Clear All MIDI Mappings");
        m.addSeparator();
        m.addItem(3, "About DustCrate...");
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&menuSettingsBtn),
            [this](int r)
            {
                if (r == 1) audioProcessor.midiLearn.toggleLearnMode();
                else if (r == 2) audioProcessor.midiLearn.clearAll();
                else if (r == 3)
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::MessageBoxIconType::InfoIcon,
                        "DustCrate v0.1",
                        "Digital crate digger for MPC workflow.\n"
                        "Browse vintage sounds, shape with ADSR/filter,\n"
                        "then sample the output into your MPC.\n\nMIT License.");
            });
    };

    // ---- Preset bar ----
    addAndMakeVisible(presetBar);
    presetBar.onPresetLoaded = [this] { refreshBrowsers(); };

    // ---- Waveform ----
    addAndMakeVisible(waveform);
    waveform.waveColour       = DustCrateLookAndFeel::amber();
    waveform.backgroundColour = juce::Colour(0xff141517);
    waveform.gridColour       = juce::Colour(0xff222426);
    juce::Component::SafePointer<WaveformDisplay> safeWave(&waveform);
    audioProcessor.onAudioBlock = [safeWave](const float* d, int n)
    { if (auto* w = safeWave.getComponent()) w->pushSamples(d, n); };

    // ---- Browser panels ----
    addAndMakeVisible(soundsPanel); addAndMakeVisible(noisePanel);
    soundsPanel.addChildAndMakeVisible(soundTagBar);
    soundsPanel.addChildAndMakeVisible(searchBox);
    soundsPanel.addChildAndMakeVisible(packFilter);
    soundsPanel.addChildAndMakeVisible(mainList);
    noisePanel.addChildAndMakeVisible(noiseTagBar);
    noisePanel.addChildAndMakeVisible(noiseList);

    searchBox.setTextToShowWhenEmpty("Search samples...", DustCrateLookAndFeel::textSec());
    searchBox.setWantsKeyboardFocus(true);
    searchBox.onTextChange = [this] { refreshBrowsers(); };
    searchBox.onReturnKey  = [this] { refreshBrowsers(); };
    searchBox.onEscapeKey  = [this] { searchBox.clear(); refreshBrowsers(); };

    styleCombo(packFilter);
    packFilter.addItem("All Packs",1); packFilter.setSelectedId(1);
    packFilter.onChange = [this] { refreshBrowsers(); };

    auto& lib = audioProcessor.getSampleLibrary();
    soundTagBar.setCategories(lib.getCategories());
    soundTagBar.onCategorySelected = [this](const juce::String&) { refreshBrowsers(); };
    refreshNoiseTags();
    noiseTagBar.onCategorySelected = [this](const juce::String&) { refreshBrowsers(); };

    int pid = 2;
    for (auto& pk : lib.getPacks()) packFilter.addItem(pk, pid++);

    samplePreview.initialise();
    setupKnob(previewTrimSlider, previewTrimLabel, "PRV", false);
    previewTrimSlider.setRange(0.0, 1.0); previewTrimSlider.setValue(0.75);
    soundsPanel.addChildAndMakeVisible(previewTrimSlider);
    soundsPanel.addChildAndMakeVisible(previewTrimLabel);

    // FIX: single-click now loads sample into synth AND sets currentFilePath
    mainList.onSampleSelected = [this](const SampleEntry& e)
    {
        const juce::File f = audioProcessor.getSampleLibrary().resolveFilePath(e);
        if (f.existsAsFile())
        {
            currentFilePath = f.getFullPathName();
            currentRootNote = e.rootNote;
            audioProcessor.selectSample(currentFilePath, currentRootNote);
            samplePreview.previewFile(f, (float)previewTrimSlider.getValue());
        }
    };
    mainList.onSampleTriggered = [this](const SampleEntry& e)
    {
        const juce::File f = audioProcessor.getSampleLibrary().resolveFilePath(e);
        if (f.existsAsFile())
        {
            currentFilePath = f.getFullPathName();
            currentRootNote = e.rootNote;
            audioProcessor.triggerSample(currentFilePath, currentRootNote, 1.0f);
        }
    };
    noiseList.onSampleSelected = [this](const SampleEntry& e)
    {
        const juce::File f = audioProcessor.getSampleLibrary().resolveFilePath(e);
        if (f.existsAsFile())
        {
            currentFilePath = f.getFullPathName();
            currentRootNote = e.rootNote;
            audioProcessor.selectSample(currentFilePath, currentRootNote);
            samplePreview.previewFile(f, (float)previewTrimSlider.getValue());
        }
    };
    noiseList.onSampleTriggered = [this](const SampleEntry& e)
    {
        const juce::File f = audioProcessor.getSampleLibrary().resolveFilePath(e);
        if (f.existsAsFile())
        {
            currentFilePath = f.getFullPathName();
            currentRootNote = e.rootNote;
            audioProcessor.triggerSample(currentFilePath, currentRootNote, 0.75f);
        }
    };

    packWizard.onPackImported = [this](const juce::String& packName)
    {
        styleCombo(packFilter);
        packFilter.addItem(packName, packFilter.getNumItems()+2);
        refreshBrowsers();
        refreshNoiseTags();
    };

    // ---- Envelope knobs ----
    addAndMakeVisible(envelopePanel);
    setupKnob(attackSlider,  attackLabel,  "ATK");   envelopePanel.addChildAndMakeVisible(attackSlider);  envelopePanel.addChildAndMakeVisible(attackLabel);
    setupKnob(decaySlider,   decayLabel,   "DCY");   envelopePanel.addChildAndMakeVisible(decaySlider);   envelopePanel.addChildAndMakeVisible(decayLabel);
    setupKnob(sustainSlider, sustainLabel, "SUS");   envelopePanel.addChildAndMakeVisible(sustainSlider); envelopePanel.addChildAndMakeVisible(sustainLabel);
    setupKnob(releaseSlider, releaseLabel, "REL");   envelopePanel.addChildAndMakeVisible(releaseSlider); envelopePanel.addChildAndMakeVisible(releaseLabel);

    // ---- Filter / Pitch ----
    addAndMakeVisible(filterPanel);
    setupKnob(filterCutoffSlider, cutoffLabel, "CUTOFF");
    filterCutoffSlider.setSkewFactorFromMidPoint(2000.0f);  // log-scale feel
    filterPanel.addChildAndMakeVisible(filterCutoffSlider); filterPanel.addChildAndMakeVisible(cutoffLabel);
    setupKnob(filterResSlider, resLabel, "RES");             filterPanel.addChildAndMakeVisible(filterResSlider);    filterPanel.addChildAndMakeVisible(resLabel);
    setupKnob(pitchSlider,     pitchLabel, "PITCH");         filterPanel.addChildAndMakeVisible(pitchSlider);        filterPanel.addChildAndMakeVisible(pitchLabel);

    // LP / HP / BP toggle buttons
    for (auto* btn : {&filterLpBtn, &filterHpBtn, &filterBpBtn})
    {
        btn->setClickingTogglesState(false);
        btn->setLookAndFeel(&laf);
        filterPanel.addChildAndMakeVisible(*btn);
    }
    filterLpBtn.onClick = [this] { audioProcessor.apvts.getParameterAsValue("filterType") = 0; updateFilterButtons(); };
    filterHpBtn.onClick = [this] { audioProcessor.apvts.getParameterAsValue("filterType") = 1; updateFilterButtons(); };
    filterBpBtn.onClick = [this] { audioProcessor.apvts.getParameterAsValue("filterType") = 2; updateFilterButtons(); };
    // Hidden combo kept for APVTS attachment
    styleCombo(filterTypeCombo);
    filterTypeCombo.addItem("Lowpass",1); filterTypeCombo.addItem("Highpass",2); filterTypeCombo.addItem("Bandpass",3);
    filterTypeCombo.setSelectedId(1);
    filterTypeCombo.setVisible(false);
    filterPanel.addChildAndMakeVisible(filterTypeCombo);
    updateFilterButtons();

    // ---- Character knobs ----
    addAndMakeVisible(characterPanel);
    setupKnob(noiseLevelSlider, noiseLabelKnob, "NOISE",   true); characterPanel.addChildAndMakeVisible(noiseLevelSlider); characterPanel.addChildAndMakeVisible(noiseLabelKnob);
    setupKnob(driftSlider,      driftLabel,     "DRIFT",   true); characterPanel.addChildAndMakeVisible(driftSlider);      characterPanel.addChildAndMakeVisible(driftLabel);
    setupKnob(vhsSlider,        vhsLabel,       "VHS",     true); characterPanel.addChildAndMakeVisible(vhsSlider);        characterPanel.addChildAndMakeVisible(vhsLabel);
    setupKnob(cassetteSlider,   cassetteLabel,  "CASS",    true); characterPanel.addChildAndMakeVisible(cassetteSlider);   characterPanel.addChildAndMakeVisible(cassetteLabel);
    characterPanel.addChildAndMakeVisible(lfoPanel);

    // ---- APVTS attachments ----
    auto& ap = audioProcessor.apvts;
    attackAttach      = std::make_unique<SliderAttachment>(ap,"attack",         attackSlider);
    decayAttach       = std::make_unique<SliderAttachment>(ap,"decay",          decaySlider);
    sustainAttach     = std::make_unique<SliderAttachment>(ap,"sustain",        sustainSlider);
    releaseAttach     = std::make_unique<SliderAttachment>(ap,"release",        releaseSlider);
    cutoffAttach      = std::make_unique<SliderAttachment>(ap,"filterCutoff",   filterCutoffSlider);
    resAttach         = std::make_unique<SliderAttachment>(ap,"filterRes",      filterResSlider);
    pitchAttach       = std::make_unique<SliderAttachment>(ap,"pitchSemitones", pitchSlider);
    noiseLevelAttach  = std::make_unique<SliderAttachment>(ap,"noiseLevel",     noiseLevelSlider);
    driftAttach       = std::make_unique<SliderAttachment>(ap,"driftAmount",    driftSlider);
    vhsAttach         = std::make_unique<SliderAttachment>(ap,"vhsAmount",      vhsSlider);
    cassetteAttach    = std::make_unique<SliderAttachment>(ap,"cassetteAmount", cassetteSlider);
    filterTypeAttach  = std::make_unique<ComboAttachment> (ap,"filterType",     filterTypeCombo);
    lfoRateAttach     = std::make_unique<SliderAttachment>(ap,"lfoRate",        lfoPanel.rateSlider);
    lfoDepthAttach    = std::make_unique<SliderAttachment>(ap,"lfoDepth",       lfoPanel.depthSlider);
    lfoShapeAttach    = std::make_unique<ComboAttachment> (ap,"lfoShape",       lfoPanel.shapeCombo);
    lfoTargetAttach   = std::make_unique<ComboAttachment> (ap,"lfoTarget",      lfoPanel.targetCombo);

    // ---- MIDI learn registration ----
    auto& ml = audioProcessor.midiLearn;
    ml.registerSlider(&attackSlider,       "attack");          ml.registerSlider(&decaySlider,        "decay");
    ml.registerSlider(&sustainSlider,      "sustain");         ml.registerSlider(&releaseSlider,      "release");
    ml.registerSlider(&filterCutoffSlider, "filterCutoff");    ml.registerSlider(&filterResSlider,    "filterRes");
    ml.registerSlider(&pitchSlider,        "pitchSemitones");  ml.registerSlider(&noiseLevelSlider,   "noiseLevel");
    ml.registerSlider(&driftSlider,        "driftAmount");     ml.registerSlider(&vhsSlider,          "vhsAmount");
    ml.registerSlider(&cassetteSlider,     "cassetteAmount");  ml.registerSlider(&lfoPanel.rateSlider,"lfoRate");
    ml.registerSlider(&lfoPanel.depthSlider, "lfoDepth");

    // ---- On-screen keyboard ----
    addAndMakeVisible(keyboard);
    keyboard.setAvailableRange(36, 84);
    keyboard.setLowestVisibleKey(48);
    keyboard.setWantsKeyboardFocus(false);
    keyboard.setScrollButtonsVisible(true);
    keyboardListener = std::make_unique<KeyboardListener>(*this);
    keyboardState.addListener(keyboardListener.get());

    refreshBrowsers();
}

DustCrateAudioProcessorEditor::~DustCrateAudioProcessorEditor()
{
    audioProcessor.onAudioBlock = nullptr;

    if (keyboardListener) keyboardState.removeListener(keyboardListener.get());

    auto& ml = audioProcessor.midiLearn;
    ml.unregisterSlider(&attackSlider);       ml.unregisterSlider(&decaySlider);
    ml.unregisterSlider(&sustainSlider);      ml.unregisterSlider(&releaseSlider);
    ml.unregisterSlider(&filterCutoffSlider); ml.unregisterSlider(&filterResSlider);
    ml.unregisterSlider(&pitchSlider);        ml.unregisterSlider(&noiseLevelSlider);
    ml.unregisterSlider(&driftSlider);        ml.unregisterSlider(&vhsSlider);
    ml.unregisterSlider(&cassetteSlider);     ml.unregisterSlider(&lfoPanel.rateSlider);
    ml.unregisterSlider(&lfoPanel.depthSlider);

    setLookAndFeel(nullptr);
}

void DustCrateAudioProcessorEditor::setupKnob(juce::Slider& s, juce::Label& l,
                                               const juce::String& text, bool slate)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.getProperties().set("slateAccent", slate);
    s.addMouseListener(this, false);
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font("Helvetica Neue", 9, juce::Font::plain));
    l.setColour(juce::Label::textColourId,       slate ? DustCrateLookAndFeel::slate() : DustCrateLookAndFeel::textSec());
    l.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
}

void DustCrateAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
        if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent))
            audioProcessor.midiLearn.showContextMenu(s);
}

void DustCrateAudioProcessorEditor::styleCombo(juce::ComboBox& c)
{
    c.setColour(juce::ComboBox::backgroundColourId, DustCrateLookAndFeel::panel());
    c.setColour(juce::ComboBox::textColourId,       DustCrateLookAndFeel::textPri());
    c.setColour(juce::ComboBox::arrowColourId,      DustCrateLookAndFeel::textSec());
    c.setColour(juce::ComboBox::outlineColourId,    DustCrateLookAndFeel::panelBorder());
}

void DustCrateAudioProcessorEditor::updateFilterButtons()
{
    const int ft = (int)*audioProcessor.apvts.getRawParameterValue("filterType");
    filterLpBtn.setToggleState(ft == 0, juce::dontSendNotification);
    filterHpBtn.setToggleState(ft == 1, juce::dontSendNotification);
    filterBpBtn.setToggleState(ft == 2, juce::dontSendNotification);
    filterLpBtn.repaint(); filterHpBtn.repaint(); filterBpBtn.repaint();
}

void DustCrateAudioProcessorEditor::refreshNoiseTags()
{
    auto& lib = audioProcessor.getSampleLibrary();
    noiseTagBar.setCategories(lib.getSubcategories("noise"));
}

void DustCrateAudioProcessorEditor::refreshBrowsers()
{
    if (!isShowing()) return;
    auto& lib           = audioProcessor.getSampleLibrary();
    const juce::String cat    = soundTagBar.selectedCategory.equalsIgnoreCase("All") ? juce::String() : soundTagBar.selectedCategory;
    const juce::String nSub   = noiseTagBar.selectedCategory.equalsIgnoreCase("All") ? juce::String() : noiseTagBar.selectedCategory;
    const juce::String pack   = packFilter.getText() == "All Packs" ? juce::String() : packFilter.getText();
    const juce::String search = searchBox.getText().toLowerCase();

    juce::Array<SampleEntry> me, ne;
    for (const auto& e : lib.getAllSamples())
    {
        if (!pack.isEmpty()   && !e.pack.equalsIgnoreCase(pack))           continue;
        if (!search.isEmpty() && !e.name.toLowerCase().contains(search))   continue;
        if (e.category.equalsIgnoreCase("noise"))
        { if (!nSub.isEmpty() && !e.subcategory.equalsIgnoreCase(nSub)) continue; ne.add(e); }
        else
        { if (!cat.isEmpty()  && !e.category.equalsIgnoreCase(cat))    continue; me.add(e); }
    }
    mainList.setEntries(me);
    noiseList.setEntries(ne);
}

void DustCrateAudioProcessorEditor::paint(juce::Graphics& g)
{
    // ---- Background ----
    g.fillAll(DustCrateLookAndFeel::body());

    // Grain texture
    g.setColour(DustCrateLookAndFeel::textSec().withAlpha(0.04f));
    for (int gy = 0; gy < getHeight(); gy += 3)
        for (int gx = 0; gx < getWidth(); gx += 3)
            g.fillRect(gx, gy, 1, 1);

    // Vignette
    juce::ColourGradient vign(juce::Colours::transparentBlack, getWidth()*.5f, getHeight()*.5f,
                               juce::Colour(0x44000000), 0, 0, true);
    g.setGradientFill(vign);
    g.fillRect(getLocalBounds());

    // ---- Header bar ----
    const int mh = 44;
    juce::ColourGradient hdr(juce::Colour(0xff1e2022), getWidth()*.5f, 0,
                              DustCrateLookAndFeel::body(), 0, 0, true);
    g.setGradientFill(hdr); g.fillRect(0, 0, getWidth(), mh);
    g.setColour(DustCrateLookAndFeel::amber().withAlpha(0.5f)); g.fillRect(0, mh-1, getWidth(), 1);

    // Title — use VelumStroke if loaded
    juce::Font titleFont = velumStrokeTypeface
        ? juce::Font(velumStrokeTypeface).withHeight(22.f)
        : juce::Font("Courier New", 22.f, juce::Font::bold);
    g.setFont(titleFont);
    g.setColour(DustCrateLookAndFeel::amber());
    g.drawText("DUSTCRATE", 14, 0, 220, mh - 2, juce::Justification::centredLeft);

    g.setFont(juce::Font("Helvetica Neue", 9.5f, juce::Font::plain));
    g.setColour(DustCrateLookAndFeel::textSec());
    g.drawText("DIGITAL CRATE  \xb7  MPC SAMPLE COMPANION", 14, 26, 340, 14, juce::Justification::centredLeft);

    // Version badge
    g.setColour(DustCrateLookAndFeel::panelBorder());
    g.fillRoundedRectangle((float)(getWidth()-206), 14, 38, 16, 3);
    g.setColour(DustCrateLookAndFeel::textSec());
    g.setFont(juce::Font("Helvetica Neue", 9, juce::Font::plain));
    g.drawText("v0.1", getWidth()-206, 14, 38, 16, juce::Justification::centred);

    // ---- Status bar ----
    const int sbH = 16;
    g.setColour(juce::Colour(0xff111213));
    g.fillRect(0, getHeight()-sbH, getWidth(), sbH);
    g.setColour(juce::Colour(0xff3aaa55));
    g.setFont(juce::Font("Helvetica Neue", 9, juce::Font::plain));
    g.drawText("\u25cf READY", 8, getHeight()-sbH, 60, sbH, juce::Justification::centredLeft);
    g.setColour(DustCrateLookAndFeel::textSec());
    g.drawText("DustCrate v0.1 beta", 0, getHeight()-sbH, getWidth()-8, sbH, juce::Justification::centredRight);
}

void DustCrateAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    const int statusBarH = 16;
    area.removeFromBottom(statusBarH);
    area.removeFromTop(44);
    presetBar.setBounds(area.removeFromTop(32));
    area.reduce(8, 8);
    waveform.setBounds(area.removeFromTop(48));
    area.removeFromTop(6);

    // Keyboard strip at the bottom
    keyboard.setBounds(area.removeFromBottom(70));
    area.removeFromBottom(4);

    const int bottomH = 160;
    auto browserRow = area.removeFromTop(area.getHeight() - bottomH - 6);
    area.removeFromTop(6);

    // Browser split: 62% sounds / 38% noise
    const int sw = int(browserRow.getWidth() * .62f);
    auto sb = browserRow.removeFromLeft(sw).reduced(2);
    auto nb = browserRow.reduced(2);
    soundsPanel.setBounds(sb); noisePanel.setBounds(nb);

    {
        auto in = sb.reduced(6); in.removeFromTop(SectionPanel::kHeaderH);
        auto fr = in.removeFromTop(22);
        previewTrimSlider.setBounds(fr.removeFromRight(24).reduced(0,2));
        previewTrimLabel .setBounds(fr.removeFromRight(24));
        packFilter       .setBounds(fr.removeFromRight(100).reduced(0,1));
        fr.removeFromRight(4);
        searchBox        .setBounds(fr.reduced(0,1));
        in.removeFromTop(4);
        soundTagBar      .setBounds(in.removeFromTop(22));
        in.removeFromTop(4);
        mainList         .setBounds(in);
    }
    {
        auto in = nb.reduced(6); in.removeFromTop(SectionPanel::kHeaderH);
        noiseTagBar.setBounds(in.removeFromTop(22));
        in.removeFromTop(4);
        noiseList.setBounds(in);
    }

    // ---- Controls row ----
    auto controls = area;
    const int gap = 6;
    const int ew  = int(controls.getWidth() * .28f);
    const int fw  = int(controls.getWidth() * .26f);
    const int cw  = controls.getWidth() - ew - fw - 2*gap;
    auto eb = controls.removeFromLeft(ew);  controls.removeFromLeft(gap);
    auto fb = controls.removeFromLeft(fw);  controls.removeFromLeft(gap);
    auto cb = controls;
    envelopePanel  .setBounds(eb);
    filterPanel    .setBounds(fb);
    characterPanel .setBounds(cb);

    // Envelope knobs
    auto placeKnobs = [](juce::Rectangle<int> pb,
        std::initializer_list<std::pair<juce::Slider*,juce::Label*>> items)
    {
        auto inner = pb.reduced(4); inner.removeFromTop(SectionPanel::kHeaderH + 2);
        const int n = (int)items.size(), slotW = inner.getWidth() / n;
        for (auto& [s, l] : items)
        {
            auto col = inner.removeFromLeft(slotW);
            col.removeFromTop(2);
            l->setBounds(col.removeFromBottom(16));
            s->setBounds(col.reduced(4));
        }
    };
    placeKnobs(eb, {{&attackSlider,&attackLabel},{&decaySlider,&decayLabel},
                    {&sustainSlider,&sustainLabel},{&releaseSlider,&releaseLabel}});

    // Filter panel: LP/HP/BP toggle strip + knobs
    {
        auto in = fb.reduced(4); in.removeFromTop(SectionPanel::kHeaderH + 2);
        // Toggle strip
        auto toggleRow = in.removeFromTop(22);
        in.removeFromTop(2);
        const int tbw = toggleRow.getWidth() / 3;
        filterLpBtn.setBounds(toggleRow.removeFromLeft(tbw).reduced(2,1));
        filterHpBtn.setBounds(toggleRow.removeFromLeft(tbw).reduced(2,1));
        filterBpBtn.setBounds(toggleRow.reduced(2,1));
        // Knobs
        const int slotW = in.getWidth() / 3;
        for (auto [s, l] : std::initializer_list<std::pair<juce::Slider*,juce::Label*>>{
            {&filterCutoffSlider,&cutoffLabel},{&filterResSlider,&resLabel},{&pitchSlider,&pitchLabel}})
        {
            auto col = in.removeFromLeft(slotW);
            col.removeFromTop(2);
            l->setBounds(col.removeFromBottom(16));
            s->setBounds(col.reduced(4));
        }
    }

    // Character panel
    {
        auto in = cb.reduced(4); in.removeFromTop(SectionPanel::kHeaderH + 2);
        const int lfoH = 72;
        lfoPanel.setBounds(in.removeFromBottom(lfoH));
        in.removeFromBottom(4);
        const int slotW = in.getWidth() / 4;
        for (auto [s, l] : std::initializer_list<std::pair<juce::Slider*,juce::Label*>>{
            {&noiseLevelSlider,&noiseLabelKnob},{&driftSlider,&driftLabel},
            {&vhsSlider,&vhsLabel},{&cassetteSlider,&cassetteLabel}})
        {
            auto col = in.removeFromLeft(slotW);
            col.removeFromTop(2);
            l->setBounds(col.removeFromBottom(16));
            s->setBounds(col.reduced(4));
        }
    }

    // Menu buttons in header
    menuSettingsBtn.setBounds(getWidth()-88,  10, 80, 24);
    menuFileBtn    .setBounds(getWidth()-170, 10, 76, 24);
}
