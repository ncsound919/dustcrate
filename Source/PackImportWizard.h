#pragma once
#include <JuceHeader.h>
#include "SampleLibrary.h"

//==============================================================================
// PackImportWizard
//
// Makes the plugin window a drag-and-drop target for folders.
// When the user drops a folder (or multiple) onto any part of the UI:
//   1. Asks for a pack name (AlertWindow)
//   2. Calls SampleLibrary::scanUserFolder()
//   3. Persists the new entries as a JSON sidecar in the dragged folder
//      (so the pack is re-loadable on next launch)
//   4. Fires onPackImported so the editor can refresh its browser lists
//
// Attach this to the editor via FileDragAndDropTarget.
//==============================================================================
class PackImportWizard : public juce::FileDragAndDropTarget
{
public:
    explicit PackImportWizard(SampleLibrary& lib);

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped           (const juce::StringArray& files,
                                  int x, int y)                  override;

    // Called from File menu → "Import Pack..." — opens a folder chooser
    void launchImportDialog ();

    std::function<void(const juce::String& packName)> onPackImported;

private:
    SampleLibrary& library;

    // FIX: FileChooser must be a member (heap lifetime) for async callbacks
    std::unique_ptr<juce::FileChooser> fileChooser;

    void importFolder (const juce::File& folder, const juce::String& packName);
    void saveSidecarJSON (const juce::File& folder,
                          const juce::String& packName,
                          const juce::Array<SampleEntry>& entries);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PackImportWizard)
};
