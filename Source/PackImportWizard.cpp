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

    // Ask for pack name
    auto* dialog = new juce::AlertWindow("Import Pack",
                                          "Enter a name for this pack:",
                                          juce::MessageBoxIconType::NoIcon);
    const juce::String suggested = folders.size() == 1
                                 ? folders[0].getFileName()
                                 : "User Pack";
    dialog->addTextEditor("packName", suggested, "");
    dialog->addButton("Import", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    dialog->enterModalState(true,
        juce::ModalCallbackFunction::create([this, dialog, folders](int result)
        {
            if (result == 1)
            {
                const juce::String packName =
                    dialog->getTextEditorContents("packName").trim();
                if (packName.isNotEmpty())
                {
                    for (const auto& f : folders)
                        importFolder(f, packName);
                    if (onPackImported)
                        onPackImported(packName);
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

    // Collect newly added entries
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
        item->setProperty("filePath",    e.filePath);
        item->setProperty("category",    e.category);
        item->setProperty("subcategory", e.subcategory);
        item->setProperty("rootNote",    e.rootNote);
        item->setProperty("pack",        e.pack);
        item->setProperty("license",     e.license);
        item->setProperty("source",      e.source);
        samplesArr.add(juce::var(item));
    }
    root->setProperty("samples", samplesArr);

    const juce::File sidecar = folder.getChildFile("dustcrate_pack.json");
    sidecar.replaceWithText(juce::JSON::toString(juce::var(root), true));
}
