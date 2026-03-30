#include "MidiLearnManager.h"

MidiLearnManager::MidiLearnManager(juce::AudioProcessorValueTreeState& ap)
    : apvts(ap) {}

void MidiLearnManager::registerSlider(juce::Slider* slider, const juce::String& paramID)
{
    for (auto& si : sliders)
        if (si.slider == slider) { si.paramID = paramID; return; }
    sliders.add({ slider, paramID, -1 });
}

bool MidiLearnManager::processMidiBuffer(const juce::MidiBuffer& midi,
                                          juce::AudioProcessorValueTreeState& ap)
{
    bool updated = false;
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (! msg.isController()) continue;
        const int cc  = msg.getControllerNumber();
        const float v = (float) msg.getControllerValue() / 127.0f;

        // If someone is waiting to learn, assign and clear
        if (pendingLearnSlider != nullptr)
        {
            for (auto& si : sliders)
                if (si.slider == pendingLearnSlider)
                    si.learnedCC = cc;
            pendingLearnSlider = nullptr;
        }

        // Drive any mapped parameters
        for (auto& si : sliders)
        {
            if (si.learnedCC == cc)
            {
                if (auto* param = ap.getParameter(si.paramID))
                { param->setValueNotifyingHost(v); updated = true; }
            }
        }
    }
    return updated;
}

void MidiLearnManager::showContextMenu(juce::Slider* slider)
{
    juce::PopupMenu menu;
    menu.addItem(1, "MIDI Learn");

    // Check if already has a mapping
    for (const auto& si : sliders)
    {
        if (si.slider == slider && si.learnedCC >= 0)
        {
            menu.addItem(2, "Clear MIDI Mapping (CC " + juce::String(si.learnedCC) + ")");
            break;
        }
    }

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(slider),
        [this, slider](int result)
        {
            if (result == 1)
                pendingLearnSlider = slider;
            else if (result == 2)
                for (auto& si : sliders)
                    if (si.slider == slider) si.learnedCC = -1;
        });
}

void MidiLearnManager::saveToState(juce::ValueTree& extra) const
{
    juce::ValueTree ccMap("MidiLearnMap");
    for (const auto& si : sliders)
        if (si.learnedCC >= 0)
        {
            juce::ValueTree entry("Mapping");
            entry.setProperty("param", si.paramID,  nullptr);
            entry.setProperty("cc",    si.learnedCC, nullptr);
            ccMap.addChild(entry, -1, nullptr);
        }
    extra.removeAllChildren(nullptr);
    extra.addChild(ccMap, -1, nullptr);
}

void MidiLearnManager::loadFromState(const juce::ValueTree& extra)
{
    const auto ccMap = extra.getChildWithName("MidiLearnMap");
    if (! ccMap.isValid()) return;
    for (int i = 0; i < ccMap.getNumChildren(); ++i)
    {
        const auto entry = ccMap.getChild(i);
        const juce::String paramID = entry["param"];
        const int cc = (int) entry["cc"];
        for (auto& si : sliders)
            if (si.paramID == paramID) si.learnedCC = cc;
    }
}
