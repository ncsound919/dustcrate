juce_add_console_app(DustCrateTests
    PRODUCT_NAME "DustCrateTests"
)

target_sources(DustCrateTests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp
    # Production sources under test (non-UI only)
    ${CMAKE_SOURCE_DIR}/Source/PresetValidator.cpp
    ${CMAKE_SOURCE_DIR}/Source/SampleLibrary.cpp
)

target_include_directories(DustCrateTests PRIVATE ${CMAKE_SOURCE_DIR}/Source)

target_compile_definitions(DustCrateTests
    PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    # Disable plugin-specific defines that don't apply in the test runner
    JUCE_STANDALONE_APPLICATION=1
)

target_link_libraries(DustCrateTests
    PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Convenience target: build + run tests in one step.
add_custom_target(run_tests
    COMMAND DustCrateTests
    DEPENDS DustCrateTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running DustCrate unit tests..."
    VERBATIM
)
