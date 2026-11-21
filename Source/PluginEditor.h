#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AjerryVST3AudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    explicit AjerryVST3AudioProcessorEditor(AjerryVST3AudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void timerCallback() override;

private:
    void changeTab(int);
    void drawBox(juce::Graphics&, juce::Rectangle<float>, const juce::String&);
    void drawKnob(juce::Graphics&, juce::Point<float>, float, const juce::String&, const juce::String&);
    void drawMeter(juce::Graphics&, juce::Rectangle<float>, bool);
    void drawWaveform(juce::Graphics&, juce::Rectangle<float>, int);
    void setParameter(const juce::String&, float);
    AjerryVST3AudioProcessor& proc;
    juce::TextButton tabs[8];
    juce::Label title, footer;
    int page = 0;
    int activeMode = 1;
    int activeKey = 0;
    bool listening = false;
    bool lockedKey = true;
    bool draggingKnob = false;
    bool draggingEqDot = false;
    bool eqSuggestionReady = false;
 int eqListenSeconds = 0;
    double eqRmsSum = 0.0, eqLowSum = 0.0, eqHighSum = 0.0;
    int eqSamplesCollected = 0;
    int eqFilterDrag = -1;
    float suggestedHP = 80.0f, suggestedLP = 16000.0f;
    juce::Rectangle<float> eqListenBounds, eqApplyBounds, hpControlBounds, lpControlBounds;
    int dragIndex = -1;
    juce::Point<float> knobStart;
    float knobStartValue = 0.0f;
    float knobValues[4] {0.0f, 0.0f, 0.0f, 0.0f};
    float eqFreq[4] {120.0f, 400.0f, 1000.0f, 5000.0f};
    float eqGain[4] {0.0f, 0.0f, 0.0f, 0.0f};
    juce::Rectangle<float> eqGraph;
    juce::Rectangle<float> reverbTimeBounds, reverbMixBounds, delayTimeBounds, delayMixBounds, compThresholdBounds, compRatioBounds, compGainBounds, softenAmountBounds, doublerAmountBounds, softenModeBounds, doublerModeBounds;
    int activeReverbKnob = -1;
    int softenMode = 1, doublerMode = 0;
    juce::Rectangle<float> gateThresholdBounds, gateRangeBounds, gateBypassBounds;\n juce::Rectangle<float> spotAmountBounds, spotSpeedBounds, spotKeyBounds, spotLockBounds, spotBypassBounds, spotDetectKeyBounds;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AjerryVST3AudioProcessorEditor)
};
