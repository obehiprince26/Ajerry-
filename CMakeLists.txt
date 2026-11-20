cmake_minimum_required(VERSION 3.22)
project(AJERRY_Channel_Strip VERSION 0.1.0)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
  message(FATAL_ERROR "JUCE not found. Clone JUCE 8.0.8 into ./JUCE first.")
endif()
add_subdirectory(JUCE)
juce_add_plugin(AJERRY
    COMPANY_NAME "AJERRY"
    PLUGIN_MANUFACTURER_CODE Ajry
    PLUGIN_CODE AjCS
    FORMATS VST3 AAX Standalone
    PRODUCT_NAME "AJERRY CHANNEL STRIP - Compact"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)
juce_generate_juce_header(AJERRY)
target_sources(AJERRY PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(AJERRY PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(AJERRY PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_audio_processors juce::juce_gui_basics juce::juce_graphics juce::juce_gui_extra)
