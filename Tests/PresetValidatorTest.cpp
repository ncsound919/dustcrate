#include <JuceHeader.h>
#include "../Source/PresetValidator.h"

//==============================================================================
class PresetValidatorTest : public juce::UnitTest
{
public:
    PresetValidatorTest() : juce::UnitTest("PresetValidator", "DustCrate") {}

    void runTest() override
    {
        testPresetXmlValidation();
        testPackJsonValidation();
    }

private:
    //--------------------------------------------------------------------------
    void testPresetXmlValidation()
    {
        beginTest("validatePresetXml — valid preset");
        {
            juce::XmlElement xml("Parameters");
            xml.setAttribute("schemaVersion", 1);
            xml.createNewChildElement("PARAM")->setAttribute("id", "attack");
            const auto r = PresetValidator::validatePresetXml(xml, "Parameters");
            expect(r.ok, r.errorMessage);
        }

        beginTest("validatePresetXml — wrong root tag");
        {
            juce::XmlElement xml("WrongType");
            xml.setAttribute("schemaVersion", 1);
            xml.createNewChildElement("PARAM")->setAttribute("id", "attack");
            const auto r = PresetValidator::validatePresetXml(xml, "Parameters");
            expect(! r.ok);
        }

        beginTest("validatePresetXml — missing schemaVersion");
        {
            juce::XmlElement xml("Parameters");
            xml.createNewChildElement("PARAM")->setAttribute("id", "attack");
            const auto r = PresetValidator::validatePresetXml(xml, "Parameters");
            expect(! r.ok);
        }

        beginTest("validatePresetXml — schemaVersion too new");
        {
            juce::XmlElement xml("Parameters");
            xml.setAttribute("schemaVersion", 9999);
            xml.createNewChildElement("PARAM")->setAttribute("id", "attack");
            const auto r = PresetValidator::validatePresetXml(xml, "Parameters");
            expect(! r.ok);
        }

        beginTest("validatePresetXml — no parameter children (empty preset)");
        {
            juce::XmlElement xml("Parameters");
            xml.setAttribute("schemaVersion", 1);
            // Only a PresetMeta child — no real params.
            xml.createNewChildElement("PresetMeta")->setAttribute("name", "Test");
            const auto r = PresetValidator::validatePresetXml(xml, "Parameters");
            expect(! r.ok);
        }
    }

    //--------------------------------------------------------------------------
    void testPackJsonValidation()
    {
        beginTest("validatePackJson — valid pack");
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("packName", "Test Pack");
            obj->setProperty("schema_version", 1);
            auto* samples = new juce::Array<juce::var>();
            auto* entry = new juce::DynamicObject();
            entry->setProperty("name",     "Kick");
            entry->setProperty("filePath", "drums/kick.wav");
            samples->add(juce::var(entry));
            obj->setProperty("samples", juce::var(samples));
            const auto r = PresetValidator::validatePackJson(juce::var(obj));
            expect(r.ok, r.errorMessage);
        }

        beginTest("validatePackJson — missing packName");
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("schema_version", 1);
            obj->setProperty("samples", juce::var(new juce::Array<juce::var>()));
            const auto r = PresetValidator::validatePackJson(juce::var(obj));
            expect(! r.ok);
        }

        beginTest("validatePackJson — missing schema_version");
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("packName", "Test Pack");
            obj->setProperty("samples", juce::var(new juce::Array<juce::var>()));
            const auto r = PresetValidator::validatePackJson(juce::var(obj));
            expect(! r.ok);
        }

        beginTest("validatePackJson — schema_version too new");
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("packName",       "Test Pack");
            obj->setProperty("schema_version", 9999);
            obj->setProperty("samples", juce::var(new juce::Array<juce::var>()));
            const auto r = PresetValidator::validatePackJson(juce::var(obj));
            expect(! r.ok);
        }

        beginTest("validatePackJson — missing samples array");
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("packName",       "Test Pack");
            obj->setProperty("schema_version", 1);
            const auto r = PresetValidator::validatePackJson(juce::var(obj));
            expect(! r.ok);
        }

        beginTest("validatePackJson — sample entry missing filePath");
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("packName",       "Test Pack");
            obj->setProperty("schema_version", 1);
            auto* samples = new juce::Array<juce::var>();
            auto* entry = new juce::DynamicObject();
            entry->setProperty("name", "Kick");
            // No filePath
            samples->add(juce::var(entry));
            obj->setProperty("samples", juce::var(samples));
            const auto r = PresetValidator::validatePackJson(juce::var(obj));
            expect(! r.ok);
        }

        beginTest("validatePackJson — not an object");
        {
            const auto r = PresetValidator::validatePackJson(juce::var("not an object"));
            expect(! r.ok);
        }
    }
};

static PresetValidatorTest presetValidatorTest;
