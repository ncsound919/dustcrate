#include "MidiLearnManager.h"

MidiLearnManager::MidiLearnManager(juce::AudioProcessorValueTreeState& ap)
    : apvts(ap) {}

void MidiLearnManager::registerSlider(juce::Slider* slider, const juce::String& paramID)
{
    for (auto& si : sliders)
        if (si.slider == slider) { si.paramID = paramID; return; }
    sliders.add({ slider, paramID, -1 });
}

void MidiLearnManager::unregisterSlider(juce::Slider* slider)
{
    if (pendingLearnSlider.getComponent() == slider)
        pendingLearnSlider = nullptr;
    for (int i = sliders.size() - 1; i >= 0; --i)
        if (sliders[i].slider == slider)
            sliders.remove(i);
}

bool MidiLearnManager::processMidiBuffer(const juce::MidiBuffer& midi,
                                          juce::AudioProcessorValueTreeState& ap)
{
    bool updated = false;
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (! msg.isController()) continue;
        const int   cc = msg.getControllerNumber();
        const float v  = (float)msg.getControllerValue() / 127.0f;

        // FIX: use SafePointer so a destroyed slider never crashes here
        if (auto* learnTarget = pendingLearnSlider.getComponent())
        {
            for (auto& si : sliders)
                if (si.slider == learnTarget) { si.learnedCC = cc; break; }
            pendingLearnSlider = nullptr;
        }

        for (auto& si : sliders)
            if (si.learnedCC == cc)
                if (auto* param = ap.getParameter(si.paramID))
                { param->setValueNotifyingHost(v); updated = true; }
    }
    return updated;
}

void MidiLearnManager::showContextMenu(juce::Slider* slider)
{
    juce::PopupMenu menu;
    menu.addItem(1, "MIDI Learn");
    for (const auto& si : sliders)
        if (si.slider == slider && si.learnedCC >= 0)
        { menu.addItem(2, "Clear MIDI Mapping (CC " + juce::String(si.learnedCC) + ")"); break; }

    // FIX: capture SafePointer, not raw this — lambda may outlive the editor
    juce::Component::SafePointer<juce::Slider> safeSlider(slider);
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(slider),
        [this, safeSlider](int result)
        {
            if (safeSlider.getComponent() == nullptr) return;
            if (result == 1)
                pendingLearnSlider = safeSlider.getComponent();
            else if (result == 2)
                for (auto& si : sliders)
                    if (si.slider == safeSlider.getComponent()) { si.learnedCC = -1; break; }
        });
}

void MidiLearnManager::saveToState(juce::ValueTree& extra) const
{
    extra.removeAllChildren(nullptr);
    juce::ValueTree ccMap("MidiLearnMap");
    for (const auto& si : sliders)
        if (si.learnedCC >= 0)
        {
            juce::ValueTree entry("Mapping");
            entry.setProperty("param", si.paramID,   nullptr);
            entry.setProperty("cc",    si.learnedCC, nullptr);
            ccMap.addChild(entry, -1, nullptr);
        }
    extra.addChild(ccMap, -1, nullptr);
}

void MidiLearnManager::loadFromState(const juce::ValueTree& extra)
{
    const auto ccMap = extra.getChildWithName("MidiLearnMap");
    if (! ccMap.isValid()) return;
    for (int i = 0; i < ccMap.getNumChildren(); ++i)
    {
        const auto entry   = ccMap.getChild(i);
        const auto paramID = entry["param"].toString();
        const int  cc      = (int)entry["cc"];
        for (auto& si : sliders)
            if (si.paramID == paramID) { si.learnedCC = cc; break; }
    }
}
