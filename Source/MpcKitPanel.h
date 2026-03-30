#pragma once
#include <JuceHeader.h>
#include "SampleLibrary.h"

// Pad assignment for one of the 16 MPC-style pads
struct MpcPadSlot
{
    juce::String filePath;
    juce::String label;
    int          rootNote  { 36 };  // C2 default, matches MPC pad A01
    float        attack    { 0.001f };
    float        decay     { 0.1f };
    float        sustain   { 1.0f };
    float        release_  { 0.2f };
    float        pitch     { 0.0f };  // semitones
    float        filterCut { 20000.f };
    bool         occupied  { false };
};

//==============================================================================
// 16-pad grid UI — drag samples from the browser onto pads.
// Mirrors the MPC Sample pad layout (A01-D04, 4 columns x 4 rows).
// Double-click a pad to audition. Right-click for per-pad options.
class MpcKitPanel : public juce::Component,
                   public juce::DragAndDropTarget
{
public:
    explicit MpcKitPanel();
    ~MpcKitPanel() override = default;

    void paint   (juce::Graphics&) override;
    void resized () override;

    // Drag & drop from SampleBrowserList
    bool isInterestedInDragSource (const SourceDetails&) override;
    void itemDropped              (const SourceDetails&) override;
    void itemDragEnter            (const SourceDetails&) override;
    void itemDragExit             (const SourceDetails&) override;

    void mouseDown  (const juce::MouseEvent&) override;
    void mouseDrag  (const juce::MouseEvent&) override;
    void mouseUp    (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    // Assign a sample to a pad slot index 0-15
    void assignSample (int padIndex, const juce::String& filePath,
                       const juce::String& label = {});

    // Clear one or all pads
    void clearPad (int padIndex);
    void clearAll ();

    // Serialise / deserialise kit state (for save/load)
    juce::ValueTree toValueTree  () const;
    void            fromValueTree (const juce::ValueTree&);

    // Callbacks
    std::function<void(int padIndex, const MpcPadSlot&)> onPadAudition;
    std::function<void(int padIndex)>                    onPadSelected;
    std::function<void(int padIndex, const MpcPadSlot&)> onPadAssigned;

    const MpcPadSlot& getPad (int i) const { return pads[i]; }
    MpcPadSlot&       getPad (int i)       { return pads[i]; }

    static constexpr int kNumPads = 16;

    // MPC-style bank labels A01-D04
    static juce::String padLabel (int i);

private:
    std::array<MpcPadSlot, 16> pads;
    int selectedPad   { -1 };
    int hoveredPad    { -1 };
    int dragOverPad   { -1 };
    bool isDragging   { false };

    juce::Rectangle<int> padBounds (int index) const;
    int                  padAtPoint (juce::Point<int>) const;

    void showPadContextMenu (int padIndex);
    void paintPad (juce::Graphics&, int index, juce::Rectangle<int> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MpcKitPanel)
};
