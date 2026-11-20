#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AjerryVST3AudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    AjerryVST3AudioProcessorEditor (AjerryVST3AudioProcessor&);
    ~AjerryVST3AudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void updateKnobs();

private:
    AjerryVST3AudioProcessor& proc;
    juce::Label title, leftTitle, rightTitle, middleTitle, serialLabel, footer, eqHint, hpLabel, lpLabel, inOutLabel, suggHP, suggLP, hpValue, lpValue, keyLabel, detectedLabel, suggestLabel, knobLabel1, knobLabel2, knobLabel3, knobValue1, knobValue2, knobValue3;
    juce::TextButton tabs[8], bypassBtn, listenBtn, mode1, mode2, mode3, mode4, mode5, autoKeyBtn, lockKeyBtn, inOutBtn;
    juce::Slider knobs[3];
    juce::ComboBox keyBox;

    // These are defined in PluginEditor.cpp
    class Panel;
    class WaveView;
    class PianoView;
    class MeterWithArrow;
    class EQGraph;
    std::unique_ptr<Panel> leftPanel, rightPanel, middlePanel, analysis;
    std::unique_ptr<WaveView> wave;
    std::unique_ptr<PianoView> piano;
    std::unique_ptr<MeterWithArrow> compMeter, learnMeter;
    std::unique_ptr<EQGraph> graph;

    int current = 0;
    int modeSel = 1;
    int pendingKey = 0;
    int listenSecondsLeft = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AjerryVST3AudioProcessorEditor)
};
