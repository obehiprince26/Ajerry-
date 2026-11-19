cmake_minimum_required(VERSION 3.22)
project(AJERRY VERSION 1.0.0)
add_subdirectory(JUCE)
juce_add_plugin(AJERRY
    COMPANY_NAME "Ajerry"
    PLUGIN_MANUFACTURER_CODE Ajry
    PLUGIN_CODE Ajry
    FORMATS VST3
    PRODUCT_NAME "AJERRY")
target_sources(AJERRY PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_link_libraries(AJERRY PRIVATE juce::juce_audio_utils juce::juce_audio_processors)
