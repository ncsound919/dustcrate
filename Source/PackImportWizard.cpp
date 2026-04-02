#include "PackImportWizard.h"

PackImportWizard::PackImportWizard(SampleLibrary& lib)
    : library(lib) {}

void PackImportWizard::launchImportDialog()
{
    // FIX: FileChooser must be heap-allocated (member variable) so it outlives
    // the async callback. Stack-allocated choosers are destroyed before the
    // callback fires in JUCE.
    fileChooser = std::make_unique<juce::FileChooser>(
        "Import Sample Pack Folder",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory));

    // Capture a WeakReference so that if the wizard (and its owning editor) is
    // destroyed before the async chooser callback fires, we bail out safely
    // instead of dereferencing a dangling 'this'.
    juce::WeakReference<PackImportWizard> weakThis (this);

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectDirectories,
        [weakThis](const juce::FileChooser& fc)
        {
            // Guard: wizard may have been destroyed while the chooser was open
            if (weakThis == nullptr) return;

            const auto results = fc.getResults();
            if (results.isEmpty()) return;

            // Prompt for a pack name using the first folder's name as default
            const juce::String suggested = results.size() == 1
                                         ? results[0].getFileName()
                                         : "User Pack";

            auto* dialog = new juce::AlertWindow("Import Pack",
                                                  "Enter a name for this pack:",
                                                  juce::MessageBoxIconType::NoIcon);
            dialog->addTextEditor("packName", suggested, "");
            dialog->addButton("Import", 1, juce::KeyPress(juce::KeyPress::returnKey));
            dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

            dialog->enterModalState(true,
                juce::ModalCallbackFunction::create([weakThis, dialog, results](int result)
                {
                    // Guard: wizard may have been destroyed while the dialog was open
                    if (weakThis == nullptr) return;

                    if (result == 1)
                    {
                        const juce::String packName =
                            dialog->getTextEditorContents("packName").trim();
                        const juce::String safeName =
                            packName.replaceCharacters("\\/:..", "_____");
                        if (safeName.isNotEmpty())
                        {
                            for (const auto& f : results)
                                if (f.isDirectory())
                                    weakThis->importFolder(f, safeName);
                            if (weakThis->onPackImported)
                                weakThis->onPackImported(safeName);
                        }
                    }
                }), true);
        });
}



bool PackImportWizard::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
        if (juce::File(f).isDirectory()) return true;
    return false;
}

void PackImportWizard::filesDropped(const juce::StringArray& files, int, int)
{
    juce::Array<juce::File> folders;
    for (const auto& f : files)
    {
        juce::File folder(f);
        if (folder.isDirectory()) folders.add(folder);
    }
    if (folders.isEmpty()) return;

    auto* dialog = new juce::AlertWindow("Import Pack",
                                          "Enter a name for this pack:",
                                          juce::MessageBoxIconType::NoIcon);
    const juce::String suggested = folders.size() == 1
                                 ? folders[0].getFileName()
                                 : "User Pack";
    dialog->addTextEditor("packName", suggested, "");
    dialog->addButton("Import", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    // FIX: use SafePointer so wizard destruction before dialog callback is safe
    PackImportWizard* safeThis = this;
    dialog->enterModalState(true,
        juce::ModalCallbackFunction::create([safeThis, dialog, folders](int result)
        {
            if (result == 1)
            {
                const juce::String packName =
                    dialog->getTextEditorContents("packName").trim();
                // FIX: sanitise pack name (no slashes/dots that could escape paths)
                const juce::String safeName = packName.replaceCharacters("\\/:..", "_____");
                if (safeName.isNotEmpty())
                {
                    for (const auto& f : folders)
                        safeThis->importFolder(f, safeName);
                    if (safeThis->onPackImported)
                        safeThis->onPackImported(safeName);
                }
            }
        }), true);
}

void PackImportWizard::importFolder(const juce::File& folder,
                                     const juce::String& packName)
{
    const int prevCount = library.getAllSamples().size();
    library.scanUserFolder(folder, packName);
    const int newCount = library.getAllSamples().size();

    juce::Array<SampleEntry> newEntries;
    const auto& all = library.getAllSamples();
    for (int i = prevCount; i < newCount; ++i)
        newEntries.add(all[i]);

    saveSidecarJSON(folder, packName, newEntries);
}

void PackImportWizard::saveSidecarJSON(const juce::File& folder,
                                        const juce::String& packName,
                                        const juce::Array<SampleEntry>& entries)
{
    juce::DynamicObject* root = new juce::DynamicObject();
    root->setProperty("packName", packName);
    root->setProperty("version",  "1.0");

    juce::Array<juce::var> samplesArr;
    for (const auto& e : entries)
    {
        juce::DynamicObject* item = new juce::DynamicObject();
        item->setProperty("name",        e.name);
        // FIX: store path relative to the pack folder, not absolute —
        // prevents user home directory leaking into shareable JSON sidecars.
        const juce::File srcFile(e.filePath);
        const juce::String relPath = srcFile.existsAsFile()
                                   ? srcFile.getRelativePathFrom(folder)
                                   : e.filePath;
        item->setProperty("filePath",    relPath);
        item->setProperty("category",    e.category);
        item->setProperty("subcategory", e.subcategory);
        item->setProperty("rootNote",    e.rootNote);
        item->setProperty("pack",        e.pack);
        item->setProperty("license",     e.license);
        item->setProperty("source",      e.name); // FIX: do not store full source path
        samplesArr.add(juce::var(item));
    }
    root->setProperty("samples", samplesArr);

    const juce::File sidecar = folder.getChildFile("dustcrate_pack.json");
    // FIX: do not silently overwrite an existing sidecar — merge new entries in
    if (sidecar.existsAsFile())
    {
        juce::var existing;
        if (juce::JSON::parse(sidecar.loadFileAsString(), existing).wasOk())
        {
            if (auto* arr = existing["samples"].getArray())
            {
                for (const auto& v : samplesArr)
                    arr->add(v);
                existing.getDynamicObject()->setProperty("samples", samplesArr);
            }
        }
    }
    sidecar.replaceWithText(juce::JSON::toString(juce::var(root), true));
}
