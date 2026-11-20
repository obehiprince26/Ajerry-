#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AjerryVST3AudioProcessorEditor : public juce::AudioProcessorEditor,
                                      public juce::Button::Listener
{
public:
    AjerryVST3AudioProcessorEditor (AjerryVST3AudioProcessor&);
    ~AjerryVST3AudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked (juce::Button*) override;

private:
    AjerryVST3AudioProcessor& audioProcessor;
    int currentPage = 0;
    juce::Label titleLabel;
    juce::Label pageLabel;
    juce::TextButton gateButton { "1 GATE" };
    juce::TextButton spotonButton { "2 SPOTON" };
    juce::TextButton eqButton { "3 EQ" };
    juce::TextButton compButton { "4 COMP" };
    juce::TextButton softenButton { "5 SOFTEN" };
    juce::TextButton doublerButton { "6 DOUBLER" };
    juce::TextButton delayButton { "7 DELAY" };
    juce::TextButton reverbButton { "8 REVERB" };
    juce::Slider knob1, knob2, knob3, knob4, knob5, knob6;
    juce::Label label1, label2, label3, label4, label5, label6;
    void styleButton (juce::TextButton& b);
    void styleKnob (juce::Slider& s, juce::Label& l, juce::String name);
    void updateLabelsForPage();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AjerryVST3AudioProcessorEditor)
};
