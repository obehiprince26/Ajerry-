#include "PluginProcessor.h"
#include "PluginEditor.h"

AJERRYAudioProcessorEditor::AJERRYAudioProcessorEditor(AJERRYAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(400, 300);
    auto setupSlider = [&](juce::Slider& s, juce::Label& l, juce::String name) {
        addAndMakeVisible(s);
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(l);
        l.setText(name, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.attachToComponent(&s, false);
    };
    setupSlider(gainSlider, gainLabel, "GAIN");
    setupSlider(driveSlider, driveLabel, "DRIVE");
    setupSlider(toneSlider, toneLabel, "TONE");
    setupSlider(mixSlider, mixLabel, "MIX");

    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "gain", gainSlider);
    driveAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "drive", driveSlider);
    toneAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "tone", toneSlider);
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mix", mixSlider);
}

AJERRYAudioProcessorEditor::~AJERRYAudioProcessorEditor() {}

void AJERRYAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(28.0f);
    g.drawText("AJERRY", getLocalBounds().removeFromTop(50), juce::Justification::centred, true);
}

void AJERRYAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().removeFromBottom(230).reduced(10);
    int w = area.getWidth() / 4;
    gainSlider.setBounds(area.removeFromLeft(w).reduced(5));
    driveSlider.setBounds(area.removeFromLeft(w).reduced(5));
    toneSlider.setBounds(area.removeFromLeft(w).reduced(5));
    mixSlider.setBounds(area.removeFromLeft(w).reduced(5));
}
