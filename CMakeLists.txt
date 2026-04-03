cmake_minimum_required(VERSION 3.22)
project(DustCrate VERSION 0.1.0)

# ── AddressSanitizer build type ────────────────────────────────────────────────
# Usage: cmake -B build -DCMAKE_BUILD_TYPE=AddressSanitizer
if(CMAKE_BUILD_TYPE STREQUAL "AddressSanitizer")
    set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${ASAN_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    ${ASAN_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ASAN_FLAGS}")
    message(STATUS "AddressSanitizer enabled")
endif()

add_subdirectory(JUCE)

juce_add_plugin(DustCrate
    COMPANY_NAME "Overlay365"
    PLUGIN_MANUFACTURER_CODE Ov65
    PLUGIN_CODE DsCr
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "DustCrate"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
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
    Source/CharacterProcessor.cpp
    Source/WaveformDisplay.cpp
    Source/MpcKitPanel.cpp
    Source/MpcExportEngine.cpp
    Source/SlicerPanel.cpp
    Source/MidiOutputPanel.cpp
    Source/MidiLearnManager.cpp
    Source/PresetBrowserBar.cpp
    Source/PresetManager.cpp
    Source/PresetValidator.cpp
    Source/SamplePreview.cpp
    Source/PackImportWizard.cpp
    Source/MacroLFO.cpp
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

# Binary assets compiled into the plugin binary.
# NOTE: SedgwickAveDisplay-Regular.ttf is included for optional display use.
# NOTE: PNG images in assets/images/ are placeholder branding assets;
#       a designer can replace them without changing any source code.
juce_add_binary_data(DustCrateAssets
    SOURCES
    packs/factory_pack.json
    assets/fonts/SedgwickAveDisplay-Regular.ttf
    assets/images/dustcrate_logo.png
    assets/images/dustcrate_header_banner.png
    assets/images/section_browser_banner.png
    assets/images/section_keys_banner.png
    assets/images/section_character_banner.png
)

target_link_libraries(DustCrate PRIVATE DustCrateAssets)

# ── Unit tests ─────────────────────────────────────────────────────────────────
add_subdirectory(Tests)
