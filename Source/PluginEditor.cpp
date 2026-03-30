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
    setColour(juce::TextButton::buttonOnColourId,              rowSel());
    setColour(juce::TextButton::textColourOffId,               textSec());
    setColour(juce::TextButton::textColourOnId,                amber());
}

void DustCrateLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int w, int h, float pos, float startA, float endA, juce::Slider& slider)
{
    const bool isSlate = (bool)(int)slider.getProperties()["slateAccent"];
    const auto accent  = isSlate ? slate() : amber();
    const float cx = x + w * .5f, cy = y + h * .5f;
    const float r  = juce::jmin(w, h) * .38f;
    const float angle = startA + pos * (endA - startA);

    // Arc track
    juce::Path track; track.addCentredArc(cx,cy,r,r,0,startA,endA,true);
    g.setColour(knobTrack()); g.strokePath(track, juce::PathStrokeType(2.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));

    // Value arc
    juce::Path val; val.addCentredArc(cx,cy,r,r,0,startA,angle,true);
    g.setColour(accent); g.strokePath(val, juce::PathStrokeType(2.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));

    // Glowing cap at arc endpoint
    const float capX = cx + r * std::sin(angle);
    const float capY = cy - r * std::cos(angle);
    g.setColour(accent.withAlpha(0.25f)); g.fillEllipse(capX-4.5f, capY-4.5f, 9.0f, 9.0f);
    g.setColour(accent);                  g.fillEllipse(capX-2.5f, capY-2.5f, 5.0f, 5.0f);

    // Knob body with gradient (top-left lighter → bottom-right knobBody)
    const float kr = r * .68f;
    juce::ColourGradient bodyGrad(juce::Colour(0xff353739), cx-kr, cy-kr,
                                  knobBody(), cx+kr, cy+kr, false);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx-kr, cy-kr, kr*2, kr*2);

    // Dark ring around knob body
    g.setColour(juce::Colour(0xff101112)); g.drawEllipse(cx-kr, cy-kr, kr*2, kr*2, 1.0f);

    // Indicator dot
    const float dd = kr * .62f;
    g.setColour(accent);
    g.fillEllipse(cx+dd*std::sin(angle)-2.2f, cy-dd*std::cos(angle)-2.2f, 4.4f, 4.4f);

    // Specular highlight (top-left quadrant, white at 8% alpha)
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.fillEllipse(cx-kr*0.6f, cy-kr*0.7f, kr*0.55f, kr*0.4f);
}

void DustCrateLookAndFeel::drawComboBox(juce::Graphics& g,int w,int h,bool,int,int,int,int,juce::ComboBox&)
{
    g.setColour(panel());       g.fillRoundedRectangle(0,0,(float)w,(float)h,4);
    g.setColour(panelBorder()); g.drawRoundedRectangle(.5f,.5f,w-1.f,h-1.f,4,.8f);
    // Amber accent line on left edge
    g.setColour(amber().withAlpha(0.6f)); g.fillRoundedRectangle(0,0,2,(float)h,1);
    // Chevron dropdown arrow
    const float ax=w-14,ay=h*.5f;
    juce::Path arr; arr.startNewSubPath(ax,ay-2.5f); arr.lineTo(ax+5,ay+2.5f); arr.lineTo(ax+10,ay-2.5f);
    g.setColour(textSec()); g.strokePath(arr, juce::PathStrokeType(1.2f));
}
juce::Font DustCrateLookAndFeel::getComboBoxFont(juce::ComboBox&) { return juce::Font("Helvetica Neue",11.5f,juce::Font::plain); }
void DustCrateLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& l)
{
    g.setColour(l.findColour(juce::Label::backgroundColourId)); g.fillRoundedRectangle(l.getLocalBounds().toFloat(),2);
    g.setColour(l.findColour(juce::Label::textColourId)); g.setFont(l.getFont());
    g.drawText(l.getText(), l.getLocalBounds().reduced(2,0), l.getJustificationType());
}
void DustCrateLookAndFeel::drawPopupMenuBackground(juce::Graphics& g,int w,int h)
{ g.setColour(panel()); g.fillRoundedRectangle(0,0,(float)w,(float)h,4); g.setColour(panelBorder()); g.drawRoundedRectangle(.5f,.5f,w-1.f,h-1.f,4,.8f); }
void DustCrateLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
    bool sep, bool active, bool hi, bool, bool, const juce::String& text, const juce::String&,
    const juce::Drawable*, const juce::Colour*)
{
    if (sep) { g.setColour(panelBorder()); g.fillRect(area.getX()+4,area.getCentreY(),area.getWidth()-8,1); return; }
    if (hi)  {
        g.setColour(rowSel()); g.fillRoundedRectangle(area.reduced(2,1).toFloat(),3);
        g.setColour(amber()); g.fillRect(area.getX(), area.getY(), 2, area.getHeight());
    }
    g.setColour(active?(hi?amber():textPri()):textSec());
    g.setFont(juce::Font("Helvetica Neue",11.5f,juce::Font::plain));
    g.drawText(text,area.reduced(8,0),juce::Justification::centredLeft);
}

//==============================================================================
// CategoryTagBar
//==============================================================================
CategoryTagBar::CategoryTagBar() { setRepaintsOnMouseActivity(true); }
void CategoryTagBar::setCategories(const juce::StringArray& cats)
{ categories.clear(); categories.add("All"); for (auto& c:cats) categories.addIfNotAlreadyThere(c); selectedCategory="All"; repaint(); }
void CategoryTagBar::paint(juce::Graphics& g)
{
    g.fillAll(DustCrateLookAndFeel::body());
    const int ph=getHeight()-4; int x=2;
    for (const auto& cat:categories)
    {
        const bool sel=cat.equalsIgnoreCase(selectedCategory);
        const int tw=16+juce::Font("Helvetica Neue",10.5f,juce::Font::plain).getStringWidth(cat);
        juce::Rectangle<float> pill((float)x,2,(float)tw,(float)ph);
        if (sel)
        {
            // Outer amber glow
            g.setColour(DustCrateLookAndFeel::amber().withAlpha(0.2f));
            g.fillRoundedRectangle(pill.expanded(1.5f),(ph+3)*.5f);
            // Filled pill
            g.setColour(DustCrateLookAndFeel::amber());
            g.fillRoundedRectangle(pill,ph*.5f);
            g.setColour(DustCrateLookAndFeel::body());
            g.setFont(juce::Font("Helvetica Neue",10.5f,juce::Font::bold));
        }
        else
        {
            g.setColour(DustCrateLookAndFeel::panel().withAlpha(0.5f));
            g.fillRoundedRectangle(pill,ph*.5f);
            g.setColour(DustCrateLookAndFeel::panelBorder());
            g.drawRoundedRectangle(pill,ph*.5f,1);
            g.setColour(DustCrateLookAndFeel::textSec());
            g.setFont(juce::Font("Helvetica Neue",10.5f,juce::Font::plain));
        }
        g.drawText(cat,(int)pill.getX(),(int)pill.getY(),(int)pill.getWidth(),(int)pill.getHeight(),juce::Justification::centred);
        x+=tw+4;
    }
}
void CategoryTagBar::mouseUp(const juce::MouseEvent& e)
{
    const int ph=getHeight()-4; int x=2;
    for (const auto& cat:categories)
    {
        const int tw=16+juce::Font("Helvetica Neue",10.5f,juce::Font::plain).getStringWidth(cat);
        if(juce::Rectangle<int>(x,2,tw,ph).contains(e.getPosition()))
        { selectedCategory=cat; repaint(); if(onCategorySelected) onCategorySelected(cat.equalsIgnoreCase("All")?juce::String():cat); return; }
        x+=tw+4;
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
    if (sel)
    {
        g.setColour(DustCrateLookAndFeel::rowSel());    g.fillRect(0,0,w,h);
        g.setColour(DustCrateLookAndFeel::amberGlow()); g.fillRect(0,0,w,h);
        g.setColour(DustCrateLookAndFeel::amber());     g.fillRect(0,0,3,h);
    }
    // Row separator
    g.setColour(DustCrateLookAndFeel::panelBorder().withAlpha(0.4f));
    g.fillRect(0, h-1, w, 1);

    if (!juce::isPositiveAndBelow(row,entries.size())) return;
    const auto& e=entries[row];

    // Play triangle icon (5×6 px, vertically centred at x=10)
    const float triAlpha = sel ? 1.0f : 0.35f;
    g.setColour(DustCrateLookAndFeel::amber().withAlpha(triAlpha));
    juce::Path tri;
    const float ty = h * 0.5f;
    tri.addTriangle(10.0f, ty-3.0f, 10.0f, ty+3.0f, 16.0f, ty);
    g.fillPath(tri);

    // Category badge pill (right side)
    const juce::String badge=e.subcategory.isEmpty()?e.category.toUpperCase().substring(0,4):e.subcategory.substring(0,4);
    const int bw=32,bx=w-bw-6;
    g.setColour(DustCrateLookAndFeel::panelBorder());
    g.fillRoundedRectangle((float)bx,5,(float)bw,(float)(h-10),2);
    g.setColour(DustCrateLookAndFeel::textSec());
    g.setFont(juce::Font("Helvetica Neue",8.5f,juce::Font::plain));
    g.drawText(badge,bx,0,bw,h,juce::Justification::centred);

    // Sample name — monospace, starting at x=24
    g.setColour(DustCrateLookAndFeel::textPri());
    g.setFont(juce::Font("Courier New",12,juce::Font::plain));
    g.drawText(e.name,24,0,bx-14,h,juce::Justification::centredLeft);
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
    auto b=getLocalBounds().toFloat();
    const auto accent=slateAccent?DustCrateLookAndFeel::slate():DustCrateLookAndFeel::amber();

    // Panel background and border
    g.setColour(DustCrateLookAndFeel::panel()); g.fillRoundedRectangle(b,6);
    g.setColour(DustCrateLookAndFeel::panelBorder()); g.drawRoundedRectangle(b.reduced(.5f),6,.8f);

    // Header strip: gradient from accent tint to transparent, left-to-right
    const auto gradStart = slateAccent ? DustCrateLookAndFeel::slateDim().withAlpha(0.18f)
                                       : DustCrateLookAndFeel::amber().withAlpha(0.18f);
    juce::ColourGradient hdrGrad(gradStart, b.getX(), b.getY(),
                                 juce::Colours::transparentBlack, b.getRight(), b.getY(), false);
    g.setGradientFill(hdrGrad);
    juce::Path hdrPath;
    hdrPath.addRoundedRectangle(b.getX(), b.getY(), b.getWidth(), (float)kHeaderH, 6, 6, true, true, false, false);
    g.fillPath(hdrPath);

    // 2px accent line at top edge (top corners only)
    juce::Path topLine;
    topLine.addRoundedRectangle(b.getX(), b.getY(), b.getWidth(), 2.0f, 2, 2, true, true, false, false);
    g.setColour(accent.withAlpha(0.8f)); g.fillPath(topLine);

    // Section title with text shadow
    g.setFont(juce::Font("Helvetica Neue",9.5f,juce::Font::bold));
    g.setColour(juce::Colour(0x44000000));
    g.drawText(title,(int)b.getX()+9,(int)b.getY()+4,(int)b.getWidth()-16,kHeaderH-3,juce::Justification::centredLeft);
    g.setColour(accent);
    g.drawText(title,(int)b.getX()+8,(int)b.getY()+3,(int)b.getWidth()-16,kHeaderH-3,juce::Justification::centredLeft);
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

    rateLabel.setText("RATE",juce::dontSendNotification);
    rateLabel.setJustificationType(juce::Justification::centred);
    rateLabel.setFont(juce::Font("Helvetica Neue",9,juce::Font::plain));
    rateLabel.setColour(juce::Label::textColourId,DustCrateLookAndFeel::slate());
    rateLabel.setColour(juce::Label::backgroundColourId,juce::Colours::transparentBlack);

    depthLabel.setText("DEPTH",juce::dontSendNotification);
    depthLabel.setJustificationType(juce::Justification::centred);
    depthLabel.setFont(juce::Font("Helvetica Neue",9,juce::Font::plain));
    depthLabel.setColour(juce::Label::textColourId,DustCrateLookAndFeel::slate());
    depthLabel.setColour(juce::Label::backgroundColourId,juce::Colours::transparentBlack);

    for (auto* cb:{&shapeCombo,&targetCombo})
    {
        cb->setColour(juce::ComboBox::backgroundColourId, DustCrateLookAndFeel::panel());
        cb->setColour(juce::ComboBox::textColourId,       DustCrateLookAndFeel::slate());
        cb->setColour(juce::ComboBox::arrowColourId,      DustCrateLookAndFeel::slateDim());
        cb->setColour(juce::ComboBox::outlineColourId,    DustCrateLookAndFeel::panelBorder());
    }
    shapeCombo.addItem("Sine",1); shapeCombo.addItem("Triangle",2);
    shapeCombo.addItem("Saw",3);  shapeCombo.addItem("Square",4);
    shapeCombo.setSelectedId(1);
    targetCombo.addItem("None",1); targetCombo.addItem("Noise",2);
    targetCombo.addItem("Drift",3); targetCombo.addItem("VHS",4);
    targetCombo.addItem("Cassette",5); targetCombo.setSelectedId(1);
}
void LFOPanel::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // Base fill
    g.setColour(DustCrateLookAndFeel::slateDim().withAlpha(0.3f));
    g.fillRoundedRectangle(b, 5);

    // Gradient overlay: slate tint top-left → transparent bottom-right
    juce::ColourGradient overlay(DustCrateLookAndFeel::slate().withAlpha(0.08f), b.getX(), b.getY(),
                                 juce::Colours::transparentBlack, b.getRight(), b.getBottom(), false);
    g.setGradientFill(overlay);
    g.fillRoundedRectangle(b, 5);

    // Border
    g.setColour(DustCrateLookAndFeel::slate().withAlpha(0.35f));
    g.drawRoundedRectangle(b.reduced(0.5f), 5, 0.7f);

    // "LFO" label
    g.setColour(DustCrateLookAndFeel::slate());
    g.setFont(juce::Font("Helvetica Neue",8.5f,juce::Font::bold));
    g.drawText("LFO",4,2,30,12,juce::Justification::centredLeft);
}
void LFOPanel::resized()
{
    auto area=getLocalBounds().reduced(4);
    area.removeFromTop(14);
    const int kw=area.getHeight()/2;
    auto combos=area.removeFromRight(area.getWidth()-kw*2-4);
    auto lc=area.removeFromLeft(kw); auto dc=area;
    rateLabel.setBounds(lc.removeFromBottom(12)); rateSlider.setBounds(lc.reduced(2));
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
    setSize(720, 600);
    setResizable(true,true);

    addAndMakeVisible(presetBar);
    presetBar.onPresetLoaded = [this] { refreshBrowsers(); };

    addAndMakeVisible(waveform);
    waveform.waveColour       = DustCrateLookAndFeel::amber();
    waveform.backgroundColour = juce::Colour(0xff161819);
    waveform.gridColour       = juce::Colour(0xff252729);

    // FIX: use SafePointer on waveform so the lambda is safe if processor
    // outlives the editor (host can theoretically call processBlock after
    // the editor window is closed).
    juce::Component::SafePointer<WaveformDisplay> safeWave(&waveform);
    audioProcessor.onAudioBlock = [safeWave](const float* d, int n)
    {
        if (auto* w = safeWave.getComponent())
            w->pushSamples(d, n);
    };

    addAndMakeVisible(soundsPanel); addAndMakeVisible(noisePanel);
    soundsPanel.addChildAndMakeVisible(soundTagBar);
    soundsPanel.addChildAndMakeVisible(searchBox);
    soundsPanel.addChildAndMakeVisible(packFilter);
    soundsPanel.addChildAndMakeVisible(mainList);
    noisePanel.addChildAndMakeVisible(noiseTagBar);
    noisePanel.addChildAndMakeVisible(noiseList);

    searchBox.setTextToShowWhenEmpty("Search", DustCrateLookAndFeel::textSec());
    searchBox.onTextChange = [this] { refreshBrowsers(); };
    styleCombo(packFilter);
    packFilter.addItem("All Packs",1); packFilter.setSelectedId(1);
    packFilter.onChange = [this] { refreshBrowsers(); };

    auto& lib = audioProcessor.getSampleLibrary();
    soundTagBar.setCategories(lib.getCategories());
    soundTagBar.onCategorySelected = [this](const juce::String&) { refreshBrowsers(); };
    refreshNoiseTags();
    noiseTagBar.onCategorySelected = [this](const juce::String&) { refreshBrowsers(); };

    int pid=2;
    for (auto& pk:lib.getPacks()) packFilter.addItem(pk,pid++);

    samplePreview.initialise();
    setupKnob(previewTrimSlider, previewTrimLabel, "PRV", false);
    previewTrimSlider.setRange(0.0, 1.0); previewTrimSlider.setValue(0.75);
    soundsPanel.addChildAndMakeVisible(previewTrimSlider);
    soundsPanel.addChildAndMakeVisible(previewTrimLabel);

    mainList.onSampleSelected = [this](const SampleEntry& e)
    {
        const juce::File f = audioProcessor.getSampleLibrary().resolveFilePath(e);
        if (f.existsAsFile()) // FIX: guard resolve before previewing
            samplePreview.previewFile(f, (float)previewTrimSlider.getValue());
    };
    mainList.onSampleTriggered = [this](const SampleEntry& e)
    {
        const juce::File f = audioProcessor.getSampleLibrary().resolveFilePath(e);
        if (f.existsAsFile())
            audioProcessor.triggerSample(f.getFullPathName(), e.rootNote, 1.0f);
    };
    noiseList.onSampleSelected = [this](const SampleEntry& e)
    {
        const juce::File f = audioProcessor.getSampleLibrary().resolveFilePath(e);
        if (f.existsAsFile())
            samplePreview.previewFile(f, (float)previewTrimSlider.getValue());
    };
    noiseList.onSampleTriggered = [this](const SampleEntry& e)
    {
        const juce::File f = audioProcessor.getSampleLibrary().resolveFilePath(e);
        if (f.existsAsFile())
            audioProcessor.triggerSample(f.getFullPathName(), e.rootNote, 0.75f);
    };

    packWizard.onPackImported = [this](const juce::String& packName)
    {
        styleCombo(packFilter);
        packFilter.addItem(packName, packFilter.getNumItems()+2);
        refreshBrowsers();
        refreshNoiseTags();
    };

    addAndMakeVisible(envelopePanel);
    setupKnob(attackSlider,attackLabel,"ATK");    envelopePanel.addChildAndMakeVisible(attackSlider);  envelopePanel.addChildAndMakeVisible(attackLabel);
    setupKnob(decaySlider,decayLabel,"DCY");      envelopePanel.addChildAndMakeVisible(decaySlider);   envelopePanel.addChildAndMakeVisible(decayLabel);
    setupKnob(sustainSlider,sustainLabel,"SUS");  envelopePanel.addChildAndMakeVisible(sustainSlider); envelopePanel.addChildAndMakeVisible(sustainLabel);
    setupKnob(releaseSlider,releaseLabel,"REL");  envelopePanel.addChildAndMakeVisible(releaseSlider); envelopePanel.addChildAndMakeVisible(releaseLabel);

    addAndMakeVisible(filterPanel);
    setupKnob(filterCutoffSlider,cutoffLabel,"CUTOFF"); filterPanel.addChildAndMakeVisible(filterCutoffSlider); filterPanel.addChildAndMakeVisible(cutoffLabel);
    setupKnob(filterResSlider,resLabel,"RES");           filterPanel.addChildAndMakeVisible(filterResSlider);    filterPanel.addChildAndMakeVisible(resLabel);
    setupKnob(pitchSlider,pitchLabel,"PITCH");           filterPanel.addChildAndMakeVisible(pitchSlider);        filterPanel.addChildAndMakeVisible(pitchLabel);
    styleCombo(filterTypeCombo);
    filterTypeCombo.addItem("Lowpass",1); filterTypeCombo.addItem("Highpass",2); filterTypeCombo.addItem("Bandpass",3);
    filterTypeCombo.setSelectedId(1);
    filterPanel.addChildAndMakeVisible(filterTypeCombo);

    addAndMakeVisible(characterPanel);
    setupKnob(noiseLevelSlider,noiseLabelKnob,"NOISE",true); characterPanel.addChildAndMakeVisible(noiseLevelSlider); characterPanel.addChildAndMakeVisible(noiseLabelKnob);
    setupKnob(driftSlider,driftLabel,"DRIFT",true);          characterPanel.addChildAndMakeVisible(driftSlider);      characterPanel.addChildAndMakeVisible(driftLabel);
    setupKnob(vhsSlider,vhsLabel,"VHS",true);                characterPanel.addChildAndMakeVisible(vhsSlider);        characterPanel.addChildAndMakeVisible(vhsLabel);
    setupKnob(cassetteSlider,cassetteLabel,"CASS",true);     characterPanel.addChildAndMakeVisible(cassetteSlider);   characterPanel.addChildAndMakeVisible(cassetteLabel);
    characterPanel.addChildAndMakeVisible(lfoPanel);

    auto& ap = audioProcessor.apvts;
    attackAttach      = std::make_unique<SliderAttachment>(ap,"attack",attackSlider);
    decayAttach       = std::make_unique<SliderAttachment>(ap,"decay",decaySlider);
    sustainAttach     = std::make_unique<SliderAttachment>(ap,"sustain",sustainSlider);
    releaseAttach     = std::make_unique<SliderAttachment>(ap,"release",releaseSlider);
    cutoffAttach      = std::make_unique<SliderAttachment>(ap,"filterCutoff",filterCutoffSlider);
    resAttach         = std::make_unique<SliderAttachment>(ap,"filterRes",filterResSlider);
    pitchAttach       = std::make_unique<SliderAttachment>(ap,"pitchSemitones",pitchSlider);
    noiseLevelAttach  = std::make_unique<SliderAttachment>(ap,"noiseLevel",noiseLevelSlider);
    driftAttach       = std::make_unique<SliderAttachment>(ap,"driftAmount",driftSlider);
    vhsAttach         = std::make_unique<SliderAttachment>(ap,"vhsAmount",vhsSlider);
    cassetteAttach    = std::make_unique<SliderAttachment>(ap,"cassetteAmount",cassetteSlider);
    filterTypeAttach  = std::make_unique<ComboAttachment> (ap,"filterType",filterTypeCombo);
    lfoRateAttach     = std::make_unique<SliderAttachment>(ap,"lfoRate",lfoPanel.rateSlider);
    lfoDepthAttach    = std::make_unique<SliderAttachment>(ap,"lfoDepth",lfoPanel.depthSlider);
    lfoShapeAttach    = std::make_unique<ComboAttachment> (ap,"lfoShape",lfoPanel.shapeCombo);
    lfoTargetAttach   = std::make_unique<ComboAttachment> (ap,"lfoTarget",lfoPanel.targetCombo);

    auto& ml = audioProcessor.midiLearn;
    ml.registerSlider(&attackSlider,"attack");             ml.registerSlider(&decaySlider,"decay");
    ml.registerSlider(&sustainSlider,"sustain");           ml.registerSlider(&releaseSlider,"release");
    ml.registerSlider(&filterCutoffSlider,"filterCutoff"); ml.registerSlider(&filterResSlider,"filterRes");
    ml.registerSlider(&pitchSlider,"pitchSemitones");      ml.registerSlider(&noiseLevelSlider,"noiseLevel");
    ml.registerSlider(&driftSlider,"driftAmount");         ml.registerSlider(&vhsSlider,"vhsAmount");
    ml.registerSlider(&cassetteSlider,"cassetteAmount");   ml.registerSlider(&lfoPanel.rateSlider,"lfoRate");
    ml.registerSlider(&lfoPanel.depthSlider,"lfoDepth");

    refreshBrowsers();
}

DustCrateAudioProcessorEditor::~DustCrateAudioProcessorEditor()
{
    // FIX: null the audio callback FIRST, before any member destruction,
    // so the audio thread can’t fire into a half-destroyed editor.
    audioProcessor.onAudioBlock = nullptr;

    // FIX: unregister all sliders from MidiLearnManager before they are destroyed,
    // preventing the manager (owned by the processor, outlives the editor) from
    // holding dangling Slider* pointers.
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
    s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    s.getProperties().set("slateAccent",slate);
    // FIX: MIDI learn context menu must be on mouseDown, not onValueChange.
    // onValueChange fires on every automated parameter move, not right-click.
    s.addMouseListener(this, false);
    l.setText(text,juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font("Helvetica Neue",9,juce::Font::plain));
    l.setColour(juce::Label::textColourId, slate?DustCrateLookAndFeel::slate():DustCrateLookAndFeel::textSec());
    l.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
}

void DustCrateAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    // FIX: right-click MIDI learn — fires on mouseDown so it’s not confused
    // with parameter changes. Only triggers on registered sliders.
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

void DustCrateAudioProcessorEditor::refreshNoiseTags()
{
    auto& lib = audioProcessor.getSampleLibrary();
    juce::StringArray subs = lib.getSubcategories("noise");
    noiseTagBar.setCategories(subs);
}

void DustCrateAudioProcessorEditor::refreshBrowsers()
{
    auto& lib     = audioProcessor.getSampleLibrary();
    const juce::String cat    = soundTagBar.selectedCategory.equalsIgnoreCase("All") ? juce::String() : soundTagBar.selectedCategory;
    const juce::String nSub   = noiseTagBar.selectedCategory.equalsIgnoreCase("All") ? juce::String() : noiseTagBar.selectedCategory;
    const juce::String pack   = packFilter.getText() == "All Packs" ? juce::String() : packFilter.getText();
    const juce::String search = searchBox.getText().toLowerCase();

    juce::Array<SampleEntry> me, ne;
    for (const auto& e : lib.getAllSamples())
    {
        if (!pack.isEmpty()   && !e.pack.equalsIgnoreCase(pack))       continue;
        if (!search.isEmpty() && !e.name.toLowerCase().contains(search)) continue;
        if (e.category.equalsIgnoreCase("noise"))
        {
            if (!nSub.isEmpty() && !e.subcategory.equalsIgnoreCase(nSub)) continue;
            ne.add(e);
        }
        else
        {
            if (!cat.isEmpty() && !e.category.equalsIgnoreCase(cat)) continue;
            me.add(e);
        }
    }
    mainList.setEntries(me);
    noiseList.setEntries(ne);
}

void DustCrateAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds();
    const int W = bounds.getWidth(), H = bounds.getHeight();
    const int mh = 44;

    // ── Background ──────────────────────────────────────────────────────────
    g.fillAll(DustCrateLookAndFeel::body());

    // Noise/grain texture: 1×1 dot every 3px
    g.setColour(DustCrateLookAndFeel::textSec().withAlpha(0.04f));
    for (int py = 0; py < H; py += 3)
        for (int px = 0; px < W; px += 3)
            g.fillRect(px, py, 1, 1);

    // Vignette: radial gradient, center transparent → edges dark
    juce::ColourGradient vignette(juce::Colours::transparentBlack, (float)W * 0.5f, (float)H * 0.5f,
                                  juce::Colour(0x33000000), 0.0f, 0.0f, true);
    g.setGradientFill(vignette);
    g.fillRect(bounds);

    // ── Header ───────────────────────────────────────────────────────────────
    // Radial gradient: centre slightly brighter, fading to body() at edges
    juce::ColourGradient hdrGrad(juce::Colour(0xff1e2022), (float)W * 0.5f, (float)mh * 0.5f,
                                 DustCrateLookAndFeel::body(), 0.0f, 0.0f, true);
    g.setGradientFill(hdrGrad);
    g.fillRect(0, 0, W, mh);

    // 1px amber line at bottom of header (alpha 0.5)
    g.setColour(DustCrateLookAndFeel::amber().withAlpha(0.5f));
    g.fillRect(0, mh-1, W, 1);

    // Title "DUSTCRATE" — Courier New bold (or velumStroke if available)
    juce::Font titleFont;
    if (velumStrokeTypeface != nullptr)
        titleFont = juce::Font(velumStrokeTypeface).withHeight(22).boldened();
    else
        titleFont = juce::Font("Courier New", 22, juce::Font::bold);
    g.setFont(titleFont);
    g.setColour(DustCrateLookAndFeel::amber());
    g.drawText("DUSTCRATE", 14, 0, 220, mh-2, juce::Justification::centredLeft);

    // Subtitle
    g.setFont(juce::Font("Helvetica Neue", 9, juce::Font::plain));
    g.setColour(DustCrateLookAndFeel::textSec());
    g.drawText("DIGITAL CRATE  \xb7  MPC SAMPLE COMPANION", 14, 26, 340, 14, juce::Justification::centredLeft);

    // ── Status bar at bottom ─────────────────────────────────────────────────
    const int statusH = 16;
    g.setColour(juce::Colour(0xff111213));
    g.fillRect(0, H - statusH, W, statusH);
    g.setFont(juce::Font("Helvetica Neue", 9, juce::Font::plain));
    g.setColour(juce::Colour(0xff3aaa55));
    g.drawText("\xe2\x97\x8f READY", 8, H - statusH, 60, statusH, juce::Justification::centredLeft);
    g.setColour(DustCrateLookAndFeel::textSec());
    g.drawText("DustCrate v0.1 beta", 0, H - statusH, W - 8, statusH, juce::Justification::centredRight);
}

void DustCrateAudioProcessorEditor::resized()
{
    auto area=getLocalBounds();
    area.removeFromTop(44);
    presetBar.setBounds(area.removeFromTop(32));
    area.reduce(8,8);
    waveform.setBounds(area.removeFromTop(48));
    area.removeFromTop(6);

    const int bottomH=120, divH=6;
    auto browserRow = area.removeFromTop(area.getHeight()-bottomH-divH);
    area.removeFromTop(divH);

    const int sw=int(browserRow.getWidth()*.62f);
    auto sb=browserRow.removeFromLeft(sw).reduced(2);
    auto nb=browserRow.reduced(2);
    soundsPanel.setBounds(sb); noisePanel.setBounds(nb);

    {
        auto in=sb.reduced(6); in.removeFromTop(SectionPanel::kHeaderH);
        auto fr=in.removeFromTop(22);
        previewTrimSlider.setBounds(fr.removeFromRight(24).reduced(0,2));
        previewTrimLabel.setBounds(fr.removeFromRight(24));
        packFilter.setBounds(fr.removeFromRight(100).reduced(0,1)); fr.removeFromRight(4);
        searchBox.setBounds(fr.reduced(0,1));
        in.removeFromTop(4);
        soundTagBar.setBounds(in.removeFromTop(22));
        in.removeFromTop(4);
        mainList.setBounds(in);
    }
    {
        auto in=nb.reduced(6); in.removeFromTop(SectionPanel::kHeaderH);
        noiseTagBar.setBounds(in.removeFromTop(22));
        in.removeFromTop(4);
        noiseList.setBounds(in);
    }

    auto controls=area;
    const int gap=6;
    const int ew=int(controls.getWidth()*.28f);
    const int fw=int(controls.getWidth()*.26f);
    const int cw=controls.getWidth()-ew-fw-2*gap;
    auto eb=controls.removeFromLeft(ew); controls.removeFromLeft(gap);
    auto fb=controls.removeFromLeft(fw); controls.removeFromLeft(gap);
    auto cb=controls;
    envelopePanel.setBounds(eb); filterPanel.setBounds(fb); characterPanel.setBounds(cb);

    auto placeKnobs=[](juce::Rectangle<int> pb,
        std::initializer_list<std::pair<juce::Slider*,juce::Label*>> items)
    {
        auto inner=pb.reduced(4); inner.removeFromTop(SectionPanel::kHeaderH+2);
        const int n=(int)items.size(), sw2=inner.getWidth()/n;
        for (auto&[s,l]:items){ auto col=inner.removeFromLeft(sw2); l->setBounds(col.removeFromBottom(14)); s->setBounds(col.reduced(4)); }
    };
    placeKnobs(eb,{{&attackSlider,&attackLabel},{&decaySlider,&decayLabel},{&sustainSlider,&sustainLabel},{&releaseSlider,&releaseLabel}});

    {
        auto in=fb.reduced(4); in.removeFromTop(SectionPanel::kHeaderH+2);
        const int slotW=in.getWidth()/3;
        for (auto[s,l]:std::initializer_list<std::pair<juce::Slider*,juce::Label*>>{{&filterCutoffSlider,&cutoffLabel},{&filterResSlider,&resLabel},{&pitchSlider,&pitchLabel}})
        { auto col=in.removeFromLeft(slotW); l->setBounds(col.removeFromBottom(14)); s->setBounds(col.reduced(4)); }
        filterTypeCombo.setBounds(fb.reduced(6).removeFromBottom(22));
    }

    {
        auto in=cb.reduced(4); in.removeFromTop(SectionPanel::kHeaderH+2);
        const int lfoH=72;
        lfoPanel.setBounds(in.removeFromBottom(lfoH));
        in.removeFromBottom(4);
        const int slotW=in.getWidth()/4;
        for (auto[s,l]:std::initializer_list<std::pair<juce::Slider*,juce::Label*>>{
            {&noiseLevelSlider,&noiseLabelKnob},{&driftSlider,&driftLabel},
            {&vhsSlider,&vhsLabel},{&cassetteSlider,&cassetteLabel}})
        { auto col=in.removeFromLeft(slotW); l->setBounds(col.removeFromBottom(14)); s->setBounds(col.reduced(4)); }
    }
}
