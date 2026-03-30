#include "MpcKitPanel.h"

//==============================================================================
juce::String MpcKitPanel::padLabel (int i)
{
    // Row letters D->A (bottom to top on MPC), cols 1-4
    static const char* rows[] = { "D","C","B","A" };
    int row = i / 4;
    int col = (i % 4) + 1;
    return juce::String (rows[row]) + juce::String (col).paddedLeft ('0', 2);
}

MpcKitPanel::MpcKitPanel()
{
    setName ("MPC Kit");
}

void MpcKitPanel::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    g.fillAll (juce::Colour (0xff141517));

    // Grid
    for (int i = 0; i < kNumPads; ++i)
        paintPad (g, i, padBounds (i));
}

void MpcKitPanel::paintPad (juce::Graphics& g, int index, juce::Rectangle<int> b)
{
    const bool occupied = pads[index].occupied;
    const bool selected = (index == selectedPad);
    const bool hovered  = (index == hoveredPad);
    const bool dragOver = (index == dragOverPad);

    // Background gradient
    juce::Colour base = occupied ? juce::Colour (0xff2a2010) : juce::Colour (0xff1a1c1e);
    if (selected) base = juce::Colour (0xff3a2a10);
    if (hovered)  base = base.brighter (0.08f);
    if (dragOver) base = juce::Colour (0xff1a3a1a);

    juce::ColourGradient grad (base.brighter(0.04f), (float)b.getX(), (float)b.getY(),
                               base.darker(0.1f),   (float)b.getRight(), (float)b.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (b.toFloat().reduced (1.5f), 4.0f);

    // Border
    juce::Colour border = selected ? juce::Colour (0xffc8921a)
                                   : (occupied ? juce::Colour (0xff6b4a0a)
                                               : juce::Colour (0xff2a2c2f));
    if (dragOver) border = juce::Colour (0xff44cc44);
    g.setColour (border);
    g.drawRoundedRectangle (b.toFloat().reduced (1.5f), 4.0f, selected ? 1.5f : 0.8f);

    // Amber left accent bar when occupied
    if (occupied)
    {
        g.setColour (juce::Colour (0xffc8921a));
        g.fillRect (b.getX() + 2, b.getY() + 4, 3, b.getHeight() - 8);
    }

    // Pad label (A01 etc.)
    g.setFont (juce::Font ("Courier New", 9.0f, juce::Font::bold));
    g.setColour (selected ? juce::Colour (0xffc8921a) : juce::Colour (0xff6a6258));
    g.drawText (padLabel (index), b.removeFromTop (14).withTrimmedLeft (8),
                juce::Justification::centredLeft, true);

    // Sample name
    if (occupied)
    {
        g.setFont (juce::Font ("Courier New", 8.5f, juce::Font::plain));
        g.setColour (juce::Colour (0xffe4dcc8));
        auto nameArea = b.reduced (6, 0);
        g.drawText (pads[index].label.isEmpty()
                    ? juce::File (pads[index].filePath).getFileNameWithoutExtension()
                    : pads[index].label,
                    nameArea, juce::Justification::centredLeft, true);
    }
    else
    {
        // Empty pad hint
        g.setFont (juce::Font (10.0f));
        g.setColour (juce::Colour (0xff3a3c3e));
        g.drawText ("DROP", b, juce::Justification::centred, false);
    }

    // Note name indicator
    if (occupied)
    {
        auto noteArea = juce::Rectangle<int>(b.getRight() - 28, b.getY() - 14, 26, 12);
        g.setFont (juce::Font ("Courier New", 8.0f, juce::Font::plain));
        g.setColour (juce::Colour (0xff6b4a0a));
        g.drawText (juce::MidiMessage::getMidiNoteName (pads[index].rootNote, true, true, 4),
                    noteArea.translated(0, 14), juce::Justification::centredRight, false);
    }
}

void MpcKitPanel::resized() { repaint(); }

juce::Rectangle<int> MpcKitPanel::padBounds (int index) const
{
    auto area = getLocalBounds().reduced (4);
    int cols = 4, rows = 4;
    int padW = area.getWidth()  / cols;
    int padH = area.getHeight() / rows;
    int col  = index % cols;
    int row  = index / cols;
    return { area.getX() + col * padW, area.getY() + row * padH, padW - 2, padH - 2 };
}

int MpcKitPanel::padAtPoint (juce::Point<int> p) const
{
    for (int i = 0; i < kNumPads; ++i)
        if (padBounds (i).contains (p))
            return i;
    return -1;
}

//==============================================================================
void MpcKitPanel::mouseDown (const juce::MouseEvent& e)
{
    int pad = padAtPoint (e.getPosition());
    if (pad >= 0)
    {
        selectedPad = pad;
        repaint();
        if (onPadSelected) onPadSelected (pad);
    }
}

void MpcKitPanel::mouseDrag (const juce::MouseEvent&) {}
void MpcKitPanel::mouseUp   (const juce::MouseEvent&) {}

void MpcKitPanel::mouseDoubleClick (const juce::MouseEvent& e)
{
    int pad = padAtPoint (e.getPosition());
    if (pad >= 0 && pads[pad].occupied)
        if (onPadAudition) onPadAudition (pad, pads[pad]);
}

//==============================================================================
bool MpcKitPanel::isInterestedInDragSource (const SourceDetails& details)
{
    return details.description.toString().startsWith ("sample:");
}

void MpcKitPanel::itemDragEnter (const SourceDetails& details)
{
    dragOverPad = padAtPoint (details.localPosition.toInt());
    repaint();
}

void MpcKitPanel::itemDragExit (const SourceDetails&)
{
    dragOverPad = -1;
    repaint();
}

void MpcKitPanel::itemDropped (const SourceDetails& details)
{
    int pad = padAtPoint (details.localPosition.toInt());
    if (pad >= 0)
    {
        juce::String desc = details.description.toString();
        juce::String path = desc.fromFirstOccurrenceOf ("sample:", false, false);
        assignSample (pad, path);
    }
    dragOverPad = -1;
    repaint();
}

//==============================================================================
void MpcKitPanel::assignSample (int padIndex, const juce::String& filePath,
                                const juce::String& label)
{
    if (padIndex < 0 || padIndex >= kNumPads) return;
    auto& p    = pads[padIndex];
    p.filePath = filePath;
    p.label    = label;
    p.occupied = filePath.isNotEmpty();
    // Map pad index to MIDI note: A01=36, A02=37... D04=51
    p.rootNote = 36 + padIndex;
    repaint();
    if (onPadAssigned) onPadAssigned (padIndex, p);
}

void MpcKitPanel::clearPad (int padIndex)
{
    if (padIndex < 0 || padIndex >= kNumPads) return;
    pads[padIndex] = MpcPadSlot{};
    repaint();
}

void MpcKitPanel::clearAll()
{
    for (auto& p : pads) p = MpcPadSlot{};
    repaint();
}

//==============================================================================
juce::ValueTree MpcKitPanel::toValueTree() const
{
    juce::ValueTree tree ("MpcKit");
    for (int i = 0; i < kNumPads; ++i)
    {
        juce::ValueTree pad ("Pad");
        pad.setProperty ("index",    i,                    nullptr);
        pad.setProperty ("file",     pads[i].filePath,     nullptr);
        pad.setProperty ("label",    pads[i].label,        nullptr);
        pad.setProperty ("note",     pads[i].rootNote,     nullptr);
        pad.setProperty ("attack",   pads[i].attack,       nullptr);
        pad.setProperty ("decay",    pads[i].decay,        nullptr);
        pad.setProperty ("sustain",  pads[i].sustain,      nullptr);
        pad.setProperty ("release",  pads[i].release_,     nullptr);
        pad.setProperty ("pitch",    pads[i].pitch,        nullptr);
        pad.setProperty ("filterCut",pads[i].filterCut,    nullptr);
        tree.appendChild (pad, nullptr);
    }
    return tree;
}

void MpcKitPanel::fromValueTree (const juce::ValueTree& tree)
{
    for (auto child : tree)
    {
        int i = child.getProperty ("index", -1);
        if (i < 0 || i >= kNumPads) continue;
        pads[i].filePath  = child.getProperty ("file").toString();
        pads[i].label     = child.getProperty ("label").toString();
        pads[i].rootNote  = (int)  child.getProperty ("note",      36);
        pads[i].attack    = (float)child.getProperty ("attack",    0.001f);
        pads[i].decay     = (float)child.getProperty ("decay",     0.1f);
        pads[i].sustain   = (float)child.getProperty ("sustain",   1.0f);
        pads[i].release_  = (float)child.getProperty ("release",   0.2f);
        pads[i].pitch     = (float)child.getProperty ("pitch",     0.0f);
        pads[i].filterCut = (float)child.getProperty ("filterCut", 20000.f);
        pads[i].occupied  = pads[i].filePath.isNotEmpty();
    }
    repaint();
}

void MpcKitPanel::showPadContextMenu (int padIndex)
{
    juce::PopupMenu m;
    m.addItem (1, "Clear pad");
    m.addItem (2, "Set root note...");
    m.addSeparator();
    m.addItem (3, "Copy pad");
    m.addItem (4, "Paste pad");
    m.showMenuAsync ({}, [this, padIndex](int r)
    {
        if (r == 1) clearPad (padIndex);
    });
}
