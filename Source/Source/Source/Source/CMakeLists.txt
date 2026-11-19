cmake_minimum_required(VERSION 3.22)
project(AJERRY VERSION 1.0.0)
add_subdirectory(JUCE)
juce_add_plugin(AJERRY
    COMPANY_NAME "AJERRY"
    PLUGIN_MANUFACTURER_CODE Ajry
    PLUGIN_CODE Ajry
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "AJERRY"
)
target_sources(AJERRY PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)
target_compile_definitions(AJERRY PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(AJERRY PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
)
