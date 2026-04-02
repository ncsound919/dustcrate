#include "MidiLearnManager.h"

MidiLearnManager::MidiLearnManager(juce::AudioProcessorValueTreeState& ap)
    : apvts(ap) {}

void MidiLearnManager::registerSlider(juce::Slider* slider, const juce::String& paramID)
{
    const juce::ScopedLock sl(mappingLock);
    for (auto& si : sliders)
        if (si.slider == slider) { si.paramID = paramID; return; }
    sliders.add({ slider, paramID, -1 });
}

void MidiLearnManager::unregisterSlider(juce::Slider* slider)
{
    const juce::ScopedLock sl(mappingLock);
    if (pendingLearnSlider.getComponent() == slider)
        pendingLearnSlider = nullptr;
    for (int i = sliders.size() - 1; i >= 0; --i)
        if (sliders[i].slider == slider)
            sliders.remove(i);
}

bool MidiLearnManager::processMidiBuffer(const juce::MidiBuffer& midi,
                                          juce::AudioProcessorValueTreeState& ap)
{
    juce::ignoreUnused(ap); // CC values are applied on the message thread via flushCcQueue()

    // Use a non-blocking try-lock: if the message thread is currently modifying
    // the mapping state (register/unregister/context-menu), skip this audio block
    // rather than spinning.  We lose at most one CC event per block — acceptable.
    const juce::ScopedTryLock stl(mappingLock);
    if (! stl.isLocked()) return false;

    bool updated = false;
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (! msg.isController()) continue;
        const int   cc = msg.getControllerNumber();
        const float v  = (float)msg.getControllerValue() / 127.0f;

        // Assign any pending per-slider learn
        if (auto* learnTarget = pendingLearnSlider.getComponent())
        {
            for (auto& si : sliders)
                if (si.slider == learnTarget) { si.learnedCC = cc; break; }
            pendingLearnSlider = nullptr;
        }

        // Check if any slider is mapped to this CC
        bool anyMapped = false;
        for (const auto& si : sliders)
            if (si.learnedCC == cc) { anyMapped = true; break; }

        if (anyMapped)
        {
            // Push to lock-free queue; flushCcQueue() applies on message thread.
            int s1, n1, s2, n2;
            ccFifo.prepareToWrite(1, s1, n1, s2, n2);
            if (n1 > 0) ccQueue[(size_t)s1] = { cc, v };
            else if (n2 > 0) ccQueue[(size_t)s2] = { cc, v };
            ccFifo.finishedWrite(n1 > 0 ? 1 : (n2 > 0 ? 1 : 0));
            updated = true;
        }
    }
    return updated;
}

void MidiLearnManager::flushCcQueue(juce::AudioProcessorValueTreeState& ap)
{
    // Called on the message thread — safe to call setValueNotifyingHost here
    int s1, n1, s2, n2;
    const int numReady = ccFifo.getNumReady();
    if (numReady == 0) return;
    ccFifo.prepareToRead(numReady, s1, n1, s2, n2);

    auto apply = [&](int start, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            const auto& ev = ccQueue[(size_t)(start + i)];
            for (const auto& si : sliders)
                if (si.learnedCC == ev.cc)
                    if (auto* param = ap.getParameter(si.paramID))
                        param->setValueNotifyingHost(ev.value);
        }
    };
    apply(s1, n1);
    apply(s2, n2);
    ccFifo.finishedRead(n1 + n2);
}

void MidiLearnManager::toggleLearnMode()
{
    const juce::ScopedLock sl(mappingLock);
    learnMode = !learnMode;
    if (! learnMode)
        pendingLearnSlider = nullptr; // cancel any pending per-slider learn
}

void MidiLearnManager::clearAll()
{
    {
        const juce::ScopedLock sl(mappingLock);
        for (auto& si : sliders)
            si.learnedCC = -1;
    }
    // Drain the CC queue to discard stale events (no lock needed — queue is lock-free)
    int s1, n1, s2, n2;
    ccFifo.prepareToRead(ccFifo.getNumReady(), s1, n1, s2, n2);
    ccFifo.finishedRead(n1 + n2);
}

void MidiLearnManager::showContextMenu(juce::Slider* slider)
{
    juce::PopupMenu menu;
    {
        const juce::ScopedLock sl(mappingLock);
        menu.addItem(1, "MIDI Learn");
        for (const auto& si : sliders)
            if (si.slider == slider && si.learnedCC >= 0)
            { menu.addItem(2, "Clear MIDI Mapping (CC " + juce::String(si.learnedCC) + ")"); break; }
    }

    // SafePointer guards against the slider being destroyed before the async callback
    juce::Component::SafePointer<juce::Slider> safeSlider(slider);
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(slider),
        [this, safeSlider](int result)
        {
            const juce::ScopedLock sl(mappingLock);
            auto* s = safeSlider.getComponent();
            if (s == nullptr) return;
            if (result == 1)
                pendingLearnSlider = s;
            else if (result == 2)
                for (auto& si : sliders)
                    if (si.slider == s) { si.learnedCC = -1; break; }
        });
}

void MidiLearnManager::saveToState(juce::ValueTree& extra) const
{
    const juce::ScopedLock sl(mappingLock);
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
    const juce::ScopedLock sl(mappingLock);
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
