#include <JuceHeader.h>
#include "../Source/SampleLibrary.h"

//==============================================================================
class SampleLibraryTest : public juce::UnitTest
{
public:
    SampleLibraryTest() : juce::UnitTest("SampleLibrary", "DustCrate") {}

    void runTest() override
    {
        testLoadFromJSON();
        testGetByCategory();
        testGetByPack();
        testInvalidJSON();
        testMissingSchemaVersion();
    }

private:
    static juce::var buildValidPack(const juce::String& packName = "TestPack",
                                    int schemaVersion = 1)
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("packName",       packName);
        obj->setProperty("schema_version", schemaVersion);

        auto* samples = new juce::Array<juce::var>();

        auto makeEntry = [](const juce::String& name,
                             const juce::String& path,
                             const juce::String& category,
                             const juce::String& sub,
                             int rootNote = 60)
        {
            auto* e = new juce::DynamicObject();
            e->setProperty("name",        name);
            e->setProperty("filePath",    path);
            e->setProperty("category",    category);
            e->setProperty("subcategory", sub);
            e->setProperty("rootNote",    rootNote);
            e->setProperty("license",     "CC0");
            e->setProperty("source",      "test");
            return juce::var(e);
        };

        samples->add(makeEntry("Kick Deep",     "drums/kick.wav",   "drums",   "KICK", 36));
        samples->add(makeEntry("Vinyl Crackle", "noise/vinyl.wav",  "noise",   "VINYL"));
        samples->add(makeEntry("Rhodes C3",     "melodic/rh.wav",   "melodic", "KEYS"));
        obj->setProperty("samples", juce::var(samples));
        return juce::var(obj);
    }

    //--------------------------------------------------------------------------
    void testLoadFromJSON()
    {
        beginTest("loadPackFromBinaryData — valid pack");
        SampleLibrary lib;
        const juce::String json = juce::JSON::toString(buildValidPack());
        lib.loadPackFromBinaryData(json.toRawUTF8(), (int) json.getNumBytesAsUTF8());
        expectEquals(lib.getAllSamples().size(), 3);
    }

    void testGetByCategory()
    {
        beginTest("getByCategory — drums");
        SampleLibrary lib;
        const juce::String json = juce::JSON::toString(buildValidPack());
        lib.loadPackFromBinaryData(json.toRawUTF8(), (int) json.getNumBytesAsUTF8());

        const auto drums = lib.getByCategory("drums");
        expectEquals(drums.size(), 1);
        expectEquals(drums[0].name, juce::String("Kick Deep"));

        beginTest("getByCategory — noise");
        const auto noise = lib.getByCategory("noise");
        expectEquals(noise.size(), 1);
        expectEquals(noise[0].subcategory, juce::String("VINYL"));
    }

    void testGetByPack()
    {
        beginTest("getByPack");
        SampleLibrary lib;
        const juce::String json = juce::JSON::toString(buildValidPack("MyPack"));
        lib.loadPackFromBinaryData(json.toRawUTF8(), (int) json.getNumBytesAsUTF8());

        expectEquals(lib.getByPack("MyPack").size(), 3);
        expectEquals(lib.getByPack("Other").size(), 0);
    }

    void testInvalidJSON()
    {
        beginTest("loadPackFromBinaryData — invalid JSON is ignored");
        SampleLibrary lib;
        const juce::String bad = "{ this is not valid JSON }}}";
        lib.loadPackFromBinaryData(bad.toRawUTF8(), (int) bad.getNumBytesAsUTF8());
        expectEquals(lib.getAllSamples().size(), 0);
    }

    void testMissingSchemaVersion()
    {
        beginTest("loadPackFromBinaryData — missing schema_version is rejected");
        SampleLibrary lib;
        // Build a pack without schema_version — should be rejected by PresetValidator.
        auto* obj = new juce::DynamicObject();
        obj->setProperty("packName", "NoSchemaVersionPack");
        obj->setProperty("samples", juce::var(new juce::Array<juce::var>()));
        const juce::String json = juce::JSON::toString(juce::var(obj));
        lib.loadPackFromBinaryData(json.toRawUTF8(), (int) json.getNumBytesAsUTF8());
        expectEquals(lib.getAllSamples().size(), 0);
    }
};

static SampleLibraryTest sampleLibraryTest;
