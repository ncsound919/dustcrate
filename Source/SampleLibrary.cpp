#include "SampleLibrary.h"

SampleLibrary::SampleLibrary() {}

void SampleLibrary::loadPackFromJSON(const juce::File& f)
{
    // Guard: verify file exists before parsing
    if (! f.existsAsFile()) return;
    juce::var json;
    const auto result = juce::JSON::parse(f.loadFileAsString(), json);
    if (result.wasOk())
        parseJSON(json);
    // Silently ignore malformed JSON — don't crash or assert
}

void SampleLibrary::loadPackFromBinaryData(const char* data, int size)
{
    juce::var json;
    if (juce::JSON::parse(juce::String::fromUTF8(data, size), json).wasOk())
        parseJSON(json);
}

void SampleLibrary::parseJSON(const juce::var& json)
{
    // Guard: json must be an object with a "samples" array
    if (! json.isObject()) return;
    const juce::var& samplesVar = json["samples"];
    if (! samplesVar.isArray()) return;
    const juce::String packName = json["packName"].toString();
    const auto& arr = *samplesVar.getArray();
    for (const auto& item : arr)
    {
        if (! item.isObject()) continue;
        SampleEntry e;
        e.name        = item["name"].toString();
        e.filePath    = item["filePath"].toString();
        e.category    = item["category"].toString().toLowerCase();
        e.subcategory = item["subcategory"].toString().toUpperCase();
        e.pack        = packName;
        e.rootNote    = (int) item["rootNote"];
        e.license     = item["license"].toString();
        e.source      = item["source"].toString();
        if (e.name.isNotEmpty()) samples.add(e);
    }
}

//==============================================================================
juce::String SampleLibrary::inferSubcategory(const juce::String& filename,
                                              const juce::String& folder) const
{
    const juce::String fl = filename.toLowerCase();
    const juce::String fo = folder.toLowerCase();
    if (fl.contains("vinyl")   || fo.contains("vinyl"))   return "VINYL";
    if (fl.contains("dust")    || fo.contains("dust"))    return "DUST";
    if (fl.contains("crackle") || fo.contains("crackle")) return "CRACKLE";
    if (fl.contains("hiss")    || fo.contains("hiss"))    return "HUM";
    if (fl.contains("hum")     || fo.contains("hum"))     return "HUM";
    if (fl.contains("room")    || fo.contains("room"))    return "ROOM";
    if (fl.contains("noise")   || fo.contains("noise"))   return "NOISE";
    if (fl.contains("kick")    || fl.contains("bd"))      return "KICK";
    if (fl.contains("snare")   || fl.contains("sd"))      return "SNARE";
    if (fl.contains("hat")     || fl.contains("hh"))      return "HAT";
    if (fl.contains("bass"))                               return "BASS";
    if (fl.contains("rhodes")  || fl.contains("keys"))    return "KEYS";
    if (fl.contains("pad"))                                return "PAD";
    return "MISC";
}

void SampleLibrary::scanUserFolder(const juce::File& folder,
                                    const juce::String& packName)
{
    if (! folder.isDirectory()) return;
    const juce::String folderName = folder.getFileName();

    // Infer category from folder name
    const juce::String fl = folderName.toLowerCase();
    juce::String defaultCategory = "textures";
    if (fl.contains("drum") || fl.contains("beat") || fl.contains("perc"))
        defaultCategory = "drums";
    else if (fl.contains("bass") || fl.contains("keys") || fl.contains("melodic"))
        defaultCategory = "melodic";
    else if (fl.contains("vinyl") || fl.contains("dust") || fl.contains("noise")
          || fl.contains("hiss")  || fl.contains("hum")  || fl.contains("crackle")
          || fl.contains("room"))
        defaultCategory = "noise";

    const auto files = folder.findChildFiles(juce::File::findFiles, true,
                                              "*.wav;*.aiff;*.aif;*.flac");
    for (const auto& f : files)
    {
        SampleEntry e;
        e.name        = f.getFileNameWithoutExtension();
        e.filePath    = f.getFullPathName(); // absolute for user files
        e.category    = defaultCategory;
        e.subcategory = inferSubcategory(f.getFileNameWithoutExtension(), folderName);
        e.pack        = packName;
        e.rootNote    = 60;
        e.license     = "User";
        e.source      = f.getFullPathName();
        // Override: if file is in a clearly noise subfolder, always tag noise
        const juce::String pfl = f.getParentDirectory().getFileName().toLowerCase();
        if (pfl.contains("vinyl") || pfl.contains("dust") || pfl.contains("crackle")
         || pfl.contains("noise") || pfl.contains("hiss") || pfl.contains("hum")
         || pfl.contains("room"))
            e.category = "noise";
        samples.add(e);
    }
}

//==============================================================================
juce::Array<SampleEntry> SampleLibrary::getByCategory(const juce::String& cat) const
{
    juce::Array<SampleEntry> out;
    for (const auto& e : samples)
        if (e.category.equalsIgnoreCase(cat)) out.add(e);
    return out;
}

juce::Array<SampleEntry> SampleLibrary::getBySubcategory(const juce::String& sub) const
{
    juce::Array<SampleEntry> out;
    for (const auto& e : samples)
        if (e.subcategory.equalsIgnoreCase(sub)) out.add(e);
    return out;
}

juce::Array<SampleEntry> SampleLibrary::getByPack(const juce::String& pack) const
{
    juce::Array<SampleEntry> out;
    for (const auto& e : samples)
        if (e.pack.equalsIgnoreCase(pack)) out.add(e);
    return out;
}

juce::StringArray SampleLibrary::getCategories() const
{
    juce::StringArray cats;
    for (const auto& e : samples)
        cats.addIfNotAlreadyThere(e.category);
    return cats;
}

juce::StringArray SampleLibrary::getSubcategories(const juce::String& category) const
{
    juce::StringArray subs;
    for (const auto& e : samples)
        if (e.category.equalsIgnoreCase(category))
            subs.addIfNotAlreadyThere(e.subcategory);
    return subs;
}

juce::StringArray SampleLibrary::getPacks() const
{
    juce::StringArray packs;
    for (const auto& e : samples)
        packs.addIfNotAlreadyThere(e.pack);
    return packs;
}

juce::File SampleLibrary::resolveFilePath(const SampleEntry& e) const
{
    const juce::File direct(e.filePath);

    if (direct.isAbsolute())
    {
        // Security: if assetsRoot is set, reject absolute paths that escape it.
        // User-scanned entries always use absolute paths but are legitimate;
        // JSON-sourced entries should never require absolute paths outside assets.
        // We guard by checking child-of-root when assetsRoot is set.
        if (assetsRoot.isDirectory() && ! direct.isAChildOf(assetsRoot))
        {
            // Absolute path outside assetsRoot — only allow if the file is
            // a user-scanned entry (license == "User"). JSON packs must use
            // relative paths inside assetsRoot.
            if (e.license != "User")
                return juce::File(); // path traversal attempt from JSON pack
        }
        if (direct.existsAsFile()) return direct;
        return juce::File(); // absolute but not found — don't fall through to relative resolve
    }

    // Relative path — resolve under assetsRoot
    if (! assetsRoot.isDirectory()) return juce::File();
    const juce::File resolved = assetsRoot.getChildFile(e.filePath);

    // Security: verify the resolved path doesn't escape assetsRoot via ".." sequences.
    // juce::File::isAChildOf() performs a canonical path comparison.
    if (! resolved.isAChildOf(assetsRoot) && resolved != assetsRoot)
        return juce::File();
    return resolved;
}
