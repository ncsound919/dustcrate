#pragma once
#include <JuceHeader.h>

//==============================================================================
/** Metadata describing a single sample in the library. */
struct SampleEntry
{
    juce::String name;          ///< Display name shown in the browser.
    juce::String filePath;      ///< Relative (pack) or absolute (user) path to the audio file.
    juce::String category;      ///< drums | melodic | textures | vocal | noise
    juce::String subcategory;   ///< VINYL | DUST | ROOM | HUM | CRACKLE | KICK | etc.
    juce::String pack;          ///< Pack name this entry belongs to.
    int          rootNote { 60 }; ///< MIDI root note for chromatic pitch mapping.
    juce::String license;       ///< License string (e.g. "CC0", "User").
    juce::String source;        ///< Original source URL or path.
};

//==============================================================================
/**
 * @brief In-memory sample metadata catalogue with JSON pack loading and
 *        user-folder scanning.
 *
 * After construction, call loadPackFromBinaryData() or loadPackFromJSON()
 * to populate the library from a pack manifest.  User folders can be added
 * incrementally via scanUserFolder().
 *
 * ### Disk cache
 * scanUserFolder() writes a per-folder cache entry to
 * @c ~/.../DustCrate/.dustcrate_cache.json so subsequent launches skip the
 * filesystem walk when the folder has not changed.  Call
 * setCacheFile() before the first scanUserFolder() call to enable caching.
 */
class SampleLibrary
{
public:
    SampleLibrary();

    /** Load a pack from a JSON file on disk.  Validates schema before parsing. */
    void loadPackFromJSON        (const juce::File& jsonFile);

    /** Load a pack from in-memory binary data (e.g. BinaryData). */
    void loadPackFromBinaryData  (const char* data, int dataSize);

    /**
     * @brief Scan a folder recursively for audio files and add them to the library.
     *
     * WAV/AIFF/FLAC files are auto-categorised by folder/filename heuristics.
     * If a cache file has been set via setCacheFile(), results are written to
     * (and read from) disk to avoid re-scanning on subsequent launches.
     *
     * @param folder    The directory to scan.
     * @param packName  Pack label assigned to discovered entries.
     */
    void scanUserFolder (const juce::File& folder,
                         const juce::String& packName = "User");

    /**
     * @brief Set the path for the disk cache written by scanUserFolder().
     *
     * If not called (or called with an empty path), caching is silently
     * disabled and scanUserFolder() always performs a full filesystem scan.
     * Path writeability is validated at write-time (when scanUserFolder()
     * first tries to update the cache); a write failure is silently ignored
     * so a non-writeable path never prevents sample discovery.
     */
    void setCacheFile (const juce::File& cacheFile) { diskCacheFile = cacheFile; }

    const juce::Array<SampleEntry>& getAllSamples()  const { return samples; }
    juce::Array<SampleEntry>  getByCategory    (const juce::String&) const;
    juce::Array<SampleEntry>  getBySubcategory (const juce::String&) const;
    juce::Array<SampleEntry>  getByPack        (const juce::String&) const;
    juce::StringArray         getCategories    () const;
    juce::StringArray         getSubcategories (const juce::String& category) const;
    juce::StringArray         getPacks         () const;

    /** Resolve a SampleEntry's filePath to an absolute juce::File. */
    [[nodiscard]] juce::File resolveFilePath (const SampleEntry&) const;

    /** Set the root directory used to resolve relative pack paths. */
    void setAssetsRoot (const juce::File& root) { assetsRoot = root; }

private:
    juce::Array<SampleEntry> samples;
    juce::File               assetsRoot;
    juce::File               diskCacheFile;

    void parseJSON (const juce::var& json);
    juce::String inferSubcategory (const juce::String& filename,
                                   const juce::String& folderName) const;

    // ── Disk cache helpers ──────────────────────────────────────────────────
    /** Load cached entries for @p folderPath from the cache file.
     *  Returns true and populates @p out if a valid, up-to-date cache exists. */
    bool loadCacheForFolder (const juce::String& folderPath,
                             juce::int64           folderMtime,
                             juce::Array<SampleEntry>& out) const;

    /** Persist @p entries for @p folderPath (with @p folderMtime) to the cache. */
    void saveCacheForFolder (const juce::String& folderPath,
                             juce::int64           folderMtime,
                             const juce::Array<SampleEntry>& entries) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleLibrary)
};
