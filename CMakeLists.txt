cmake_minimum_required(VERSION 3.22)

project(FiveParksVST3 VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(FiveParksVST3
    COMPANY_NAME "5parks Audio"
    BUNDLE_ID "com.5parksaudio.5parksvst3"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE FPRK
    PLUGIN_CODE F5V3
    FORMATS VST3
    PRODUCT_NAME "5parks VST3"
)

target_sources(FiveParksVST3 PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    source/CreatePluginFilter.cpp
)

target_include_directories(FiveParksVST3 PRIVATE source)

target_compile_definitions(FiveParksVST3
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(FiveParksVST3
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
