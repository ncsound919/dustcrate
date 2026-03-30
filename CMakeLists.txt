cmake_minimum_required(VERSION 3.22)
project(DustCrate VERSION 0.1.0)

# --- JUCE ---
# Add JUCE as a submodule: git submodule add https://github.com/juce-framework/JUCE.git JUCE
add_subdirectory(JUCE)

juce_add_plugin(DustCrate
    COMPANY_NAME "Overlay365"
    PLUGIN_MANUFACTURER_CODE Ov65
    PLUGIN_CODE DsCr
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "DustCrate"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(DustCrate
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/SampleLibrary.cpp
        Source/SampleVoice.cpp
)

target_compile_definitions(DustCrate
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(DustCrate
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Bundle assets
juce_add_binary_data(DustCrateAssets
    SOURCES
        assets/samples/.gitkeep
        packs/factory_pack.json
)
target_link_libraries(DustCrate PRIVATE DustCrateAssets)
