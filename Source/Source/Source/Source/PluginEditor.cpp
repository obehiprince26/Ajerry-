#include "PluginEditor.h"

AJERRYAudioProcessorEditor::AJERRYAudioProcessorEditor(AJERRYAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      gainAttach(p.apvts, "gain", gainSlider),
      driveAttach(p.apvts, "drive", driveSlider),
      toneAttach(p.apvts, "tone", toneSlider),
      mixAttach(p.apvts, "mix", mixSlider)
{
    setSize(500, 300);
    auto setup = [&](juce::Slider& s, juce::Label& l, juce::String text) {
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.attachToComponent(&s, false);
        addAndMakeVisible(s);
        addAndMakeVisible(l);
    };
    setup(gainSlider, gainLabel, "GAIN");
    setup(driveSlider, driveLabel, "DRIVE");
    setup(toneSlider, toneLabel, "TONE");
    setup(mixSlider, mixLabel, "MIX");
}

AJERRYAudioProcessorEditor::~AJERRYAudioProcessorEditor() {}

void AJERRYAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111111));
    g.setColour(juce::Colours::white);
    g.setFont(28.0f);
    g.drawText("AJERRY", getLocalBounds().removeFromTop(50), juce::Justification::centred, true);
}

void AJERRYAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().removeFromBottom(230).reduced(10);
    int w = area.getWidth() / 4;
    gainSlider.setBounds(area.removeFromLeft(w));
    driveSlider.setBounds(area.removeFromLeft(w));
    toneSlider.setBounds(area.removeFromLeft(w));
    mixSlider.setBounds(area.removeFromLeft(w));
}
