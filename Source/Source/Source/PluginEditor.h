#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AJERRYAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AJERRYAudioProcessorEditor(AJERRYAudioProcessor&);
    ~AJERRYAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    AJERRYAudioProcessor& audioProcessor;
    juce::Slider gainSlider, driveSlider, toneSlider, mixSlider;
    juce::Label gainLabel, driveLabel, toneLabel, mixLabel;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttach, driveAttach, toneAttach, mixAttach;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AJERRYAudioProcessorEditor)
};
