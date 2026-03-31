# DustCrate Code Review - Critical Fixes Required

## ✅ FIXED

### 1. PluginProcessor.h - producesMidi() returns false
**Issue**: MidiOutputPanel sends MIDI but `producesMidi()` returned `false`.
**Fix**: Changed to `return true;` (committed 146e4e9)

### 2. PluginProcessor.h - selectSample() was private
**Issue**: MPC panel callbacks called `audioProcessor.selectSample()` but it was in private section.
**Fix**: Moved `selectSample()` to public section (committed 146e4e9)

### 3. WaveformDisplay.h - loadFile() missing
**Issue**: `setupMpcKitCallbacks()` calls `waveform.loadFile()` but method not declared.
**Fix**: Added `void loadFile(const juce::File& file);` declaration (committed 1666bf3) and implementation (committed ec81661)

---

## ❌ REQUIRES MANUAL FIX (file too large for web editor)

### 4. PluginEditor.cpp - initMpcPanels() never called
**Issue**: Constructor sets up all MPC UI components via `initMpcPanels()` helper method, but **never calls it**. MPC tab strip, kit builder, slicer, MIDI out panel won't appear.

**Fix needed**:
Add this line at the **END** of `DustCrateAudioProcessorEditor::DustCrateAudioProcessorEditor()` constructor (after the keyboard listener setup, before final `refreshBrowsers()`):

```cpp
// ---- MPC Companion Panels ----
initMpcPanels();

refreshBrowsers();
```

**Location**: Line ~750 of PluginEditor.cpp, immediately after:
```cpp
keyboardState.addListener(keyboardListener.get());
```

---

### 5. PluginEditor.cpp - Callback conflict in setupMpcKitCallbacks()
**Issue**: `setupMpcKitCallbacks()` line 1240 overwrites `mainList.onSampleSelected` that was already set in constructor line 410. The constructor version calls `samplePreview.previewFile()` but the MPC callback doesn't, so audio preview breaks when MPC tabs are active.

**Fix needed**:
In `setupMpcKitCallbacks()`, change the lambda to ADD the new behavior instead of replacing:

```cpp
// OLD (line ~1240):
mainList.onSampleSelected = [this](const SampleLibrary::Entry& e) {
    // only kit/slicer logic, no preview
};

// NEW (merge both):
mainList.onSampleSelected = [this](const SampleEntry& e) {
    const juce::File f = audioProcessor.getSampleLibrary().resolveFilePath(e);
    if (f.existsAsFile()) {
        currentFilePath = f.getFullPathName();
        currentRootNote = e.rootNote;
        audioProcessor.selectSample(currentFilePath, currentRootNote);
        samplePreview.previewFile(f, (float)previewTrimSlider.getValue()); // FIX: preserve preview
        slicerPanel.loadFile(f);
        waveform.loadFile(f);
    }
};
```

**Also fix type name**: Change `SampleLibrary::Entry` → `SampleEntry` for consistency (lines ~1240, ~1249, ~1259, ~1269)

---

### 6. PluginEditor.cpp - resized() missing MPC tab layout
**Issue**: `resized()` method (line ~890) lays out browser panels (soundsPanel, noisePanel) but has **zero layout code for the MPC tabs**: mpcKitSection, mpcKitPanel, slicerSection, slicerPanel, midiOutSection, midiOutputPanel, and tab buttons (tabBrowserBtn, tabKitBtn, tabSlicerBtn, tabMidiBtn).

Result: When switching tabs, MPC panels overlap or have zero size → unusable UI.

**Fix needed**:
Add MPC tab layout in `resized()` method. The bottom row (currently only browser) needs conditional layout based on `activeTab`:

```cpp
void DustCrateAudioProcessorEditor::resized()
{
    // ... existing code through keyboard + controls row ...

    // ---- Bottom panel: browser OR MPC tabs ----
    auto bottomRow = browserRow; // reuse the same area

    // Tab strip buttons at the top of bottom panel
    auto tabStrip = bottomRow.removeFromTop(30);
    const int tbw = tabStrip.getWidth() / 4;
    tabBrowserBtn.setBounds(tabStrip.removeFromLeft(tbw).reduced(2));
    tabKitBtn.setBounds(tabStrip.removeFromLeft(tbw).reduced(2));
    tabSlicerBtn.setBounds(tabStrip.removeFromLeft(tbw).reduced(2));
    tabMidiBtn.setBounds(tabStrip.reduced(2));

    bottomRow.removeFromTop(4); // gap

    if (activeTab == 0) {
        // Browser tab (existing layout)
        // ... keep existing soundsPanel / noisePanel layout ...
    }
    else if (activeTab == 1) {
        // Kit Builder tab
        mpcKitSection.setBounds(bottomRow);
        auto inner = bottomRow.reduced(6);
        inner.removeFromTop(SectionPanel::kHeaderH);
        auto topRow = inner.removeFromTop(28);
        kitNameLabel.setBounds(topRow.removeFromLeft(70));
        kitNameEditor.setBounds(topRow.removeFromLeft(150));
        topRow.removeFromLeft(4);
        mpcExportBtn.setBounds(topRow.removeFromLeft(120));
        mpcClearBtn.setBounds(topRow);
        inner.removeFromTop(4);
        mpcKitPanel.setBounds(inner);
    }
    else if (activeTab == 2) {
        // Slicer tab
        slicerSection.setBounds(bottomRow);
        auto inner = bottomRow.reduced(6);
        inner.removeFromTop(SectionPanel::kHeaderH);
        auto topRow = inner.removeFromTop(28);
        sliceAutoBtn.setBounds(topRow.removeFromLeft(80).reduced(2));
        sliceEvenBtn.setBounds(topRow.removeFromLeft(80).reduced(2));
        sliceEvenCombo.setBounds(topRow.removeFromLeft(60).reduced(2));
        topRow.removeFromLeft(4);
        sliceClearBtn.setBounds(topRow.removeFromLeft(80).reduced(2));
        sliceExportBtn.setBounds(topRow.removeFromLeft(100).reduced(2));
        sliceCountLabel.setBounds(topRow);
        inner.removeFromTop(4);
        slicerPanel.setBounds(inner);
    }
    else if (activeTab == 3) {
        // MIDI Out tab
        midiOutSection.setBounds(bottomRow);
        auto inner = bottomRow.reduced(6);
        inner.removeFromTop(SectionPanel::kHeaderH);
        midiOutputPanel.setBounds(inner);
    }
}
```

---

## Summary

**Files successfully patched**:
- ✅ `Source/PluginProcessor.h` (producesMidi, selectSample visibility)
- ✅ `Source/WaveformDisplay.h` (loadFile declaration)
- ✅ `Source/WaveformDisplay.cpp` (loadFile implementation)

**Files requiring manual fix** (too large for GitHub web editor):
- ❌ `Source/PluginEditor.cpp` - 3 critical issues (see sections 4, 5, 6 above)

**Impact if not fixed**:
- MPC tab UI won't render (missing `initMpcPanels()` call)
- Sample preview breaks when MPC tabs active (callback conflict)
- MPC panels overlap/invisible (no `resized()` layout)

**Next steps**: Clone repo locally, apply the three PluginEditor.cpp fixes above, test build.
