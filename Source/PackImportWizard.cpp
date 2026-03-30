#include "PackImportWizard.h"

PackImportWizard::PackImportWizard(SampleLibrary& lib)
    : library(lib) {}

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
