#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class AjerryVST3AudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              public juce::Button::Listener
{
public:
    AjerryVST3AudioProcessorEditor (AjerryVST3AudioProcessor&);
    ~AjerryVST3AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked (juce::Button* button) override;

private:
    AjerryVST3AudioProcessor& audioProcessor;

    int currentPage = 0; // 0=GATE 1=SPOTON 2=EQ 3=COMP 4=SOFTEN 5=DOUBLER 6=DELAY 7=REVERB

    juce::TextButton gateButton   { "GATE" };
    juce::TextButton spotonButton { "SPOTON" };
    juce::TextButton eqButton     { "EQ" };
    juce::TextButton compButton   { "COMP" };
    juce::TextButton softenButton { "SOFTEN" };
    juce::TextButton doublerButton{ "DOUBLER" };
    juce::TextButton delayButton  { "DELAY" };
    juce::TextButton reverbButton { "REVERB" };

    juce::Slider knob1, knob2, knob3, knob4, knob5, knob6;
    juce::Label label1, label2, label3, label4, label5, label6;
    juce::Label pageLabel;
    juce::Label titleLabel;

    void updateLabelsForPage();
    void styleButton (juce::TextButton& b);
    void styleKnob (juce::Slider& s, juce::Label& l, juce::String name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AjerryVST3AudioProcessorEditor)
};
