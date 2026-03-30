#include "PresetBrowserBar.h"

PresetBrowserBar::PresetBrowserBar(juce::AudioProcessorValueTreeState& ap)
    : presetManager(ap)
{
    addAndMakeVisible(presetCombo);
    presetCombo.addListener(this);
    presetCombo.setTextWhenNothingSelected("-- Init --");
    presetCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1c1e));
    presetCombo.setColour(juce::ComboBox::textColourId,       juce::Colour(0xffe8e0d0));
    presetCombo.setColour(juce::ComboBox::arrowColourId,      juce::Colour(0xfff0a020));
    presetCombo.setColour(juce::ComboBox::outlineColourId,    juce::Colour(0xff2e3032));

    addAndMakeVisible(saveBtn);
    addAndMakeVisible(saveAsBtn);
    addAndMakeVisible(deleteBtn);

    styleButton(saveBtn,   true);
    styleButton(saveAsBtn, false);
    styleButton(deleteBtn, false);

    saveBtn.onClick   = [this] { doSave();   };
    saveAsBtn.onClick = [this] { doSaveAs(); };
    deleteBtn.onClick = [this] { doDelete(); };

    rebuildCombo();
}

PresetBrowserBar::~PresetBrowserBar()
{
    presetCombo.removeListener(this);
}

void PresetBrowserBar::styleButton(juce::TextButton& b, bool accent)
{
    b.setColour(juce::TextButton::buttonColourId,   juce::Colour(0xff222426));
    b.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff2e3032));
    b.setColour(juce::TextButton::textColourOffId,
                accent ? juce::Colour(0xfff0a020) : juce::Colour(0xff7a7268));
    b.setColour(juce::TextButton::textColourOnId,   juce::Colour(0xfff0a020));
}

void PresetBrowserBar::rebuildCombo()
{
    presetCombo.removeListener(this);
    presetCombo.clear(juce::dontSendNotification);
    const auto names = presetManager.getPresetNames();
    int id = 1;
    for (const auto& n : names)
        presetCombo.addItem(n, id++);
    const juce::String cur = presetManager.getCurrentPreset();
    if (cur.isNotEmpty())
        presetCombo.setText(cur, juce::dontSendNotification);
    presetCombo.addListener(this);
}

void PresetBrowserBar::refreshList()
{
    presetManager.refreshPresetList();
    rebuildCombo();
}

void PresetBrowserBar::comboBoxChanged(juce::ComboBox* cb)
{
    if (cb == &presetCombo)
    {
        const juce::String name = presetCombo.getText();
        if (name.isNotEmpty() && presetManager.loadPreset(name))
            if (onPresetLoaded) onPresetLoaded();
    }
}

void PresetBrowserBar::doSave()
{
    const juce::String cur = presetManager.getCurrentPreset();
    if (cur.isEmpty()) { doSaveAs(); return; }
    presetManager.savePreset(cur);
    rebuildCombo();
}

void PresetBrowserBar::doSaveAs()
{
    auto* dialog = new juce::AlertWindow("Save Preset",
                                          "Enter a preset name:",
                                          juce::MessageBoxIconType::NoIcon);
    dialog->addTextEditor("name", presetManager.getCurrentPreset(), "");
    dialog->addButton("Save",   1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    dialog->enterModalState(true,
        juce::ModalCallbackFunction::create([this, dialog](int result)
        {
            if (result == 1)
            {
                const juce::String name = dialog->getTextEditorContents("name").trim();
                if (name.isNotEmpty())
                { presetManager.savePreset(name); rebuildCombo(); }
            }
        }), true);
}

void PresetBrowserBar::doDelete()
{
    const juce::String name = presetCombo.getText();
    if (name.isEmpty()) return;
    juce::AlertWindow::showOkCancelBox(
        juce::MessageBoxIconType::WarningIcon,
        "Delete Preset",
        "Delete \"" + name + "\"?",
        "Delete", "Cancel", nullptr,
        juce::ModalCallbackFunction::create([this, name](int result)
        {
            if (result == 1)
            { presetManager.deletePreset(name); rebuildCombo(); }
        }));
}

void PresetBrowserBar::paint(juce::Graphics& g)
{
    // Subtle separator line at bottom
    g.setColour(juce::Colour(0xff2e3032));
    g.drawHorizontalLine(getHeight()-1, 0.f, (float)getWidth());
}

void PresetBrowserBar::resized()
{
    auto area = getLocalBounds().reduced(4, 4);
    const int btnW = 36;
    deleteBtn.setBounds(area.removeFromRight(btnW));
    area.removeFromRight(2);
    saveAsBtn.setBounds(area.removeFromRight(btnW));
    area.removeFromRight(2);
    saveBtn.setBounds(area.removeFromRight(btnW + 8));
    area.removeFromRight(6);
    presetCombo.setBounds(area);
}
