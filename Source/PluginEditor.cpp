#include "PluginProcessor.h"
#include "PluginEditor.h"

AjerryVST3AudioProcessorEditor::AjerryVST3AudioProcessorEditor (AjerryVST3AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (900, 520);
    titleLabel.setText ("AJERRY  CHANNEL STRIP  -  Compact", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (26.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xffd4af37));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);
    pageLabel.setText ("1  GATE", juce::dontSendNotification);
    pageLabel.setFont (juce::Font (20.0f, juce::Font::bold));
    pageLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    pageLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (pageLabel);
    styleButton (gateButton);
    styleButton (spotonButton);
    styleButton (eqButton);
    styleButton (compButton);
    styleButton (softenButton);
    styleButton (doublerButton);
    styleButton (delayButton);
    styleButton (reverbButton);
    gateButton.addListener (this);
    spotonButton.addListener (this);
    eqButton.addListener (this);
    compButton.addListener (this);
    softenButton.addListener (this);
    doublerButton.addListener (this);
    delayButton.addListener (this);
    reverbButton.addListener (this);
    addAndMakeVisible (gateButton);
    addAndMakeVisible (spotonButton);
    addAndMakeVisible (eqButton);
    addAndMakeVisible (compButton);
    addAndMakeVisible (softenButton);
    addAndMakeVisible (doublerButton);
    addAndMakeVisible (delayButton);
    addAndMakeVisible (reverbButton);
    styleKnob (knob1, label1, "THRESHOLD");
    styleKnob (knob2, label2, "RANGE");
    styleKnob (knob3, label3, "ATTACK");
    styleKnob (knob4, label4, "RELEASE");
    styleKnob (knob5, label5, "HOLD");
    styleKnob (knob6, label6, "LOOKAHEAD");
    updateLabelsForPage();
}
AjerryVST3AudioProcessorEditor::~AjerryVST3AudioProcessorEditor() {}
void AjerryVST3AudioProcessorEditor::styleButton (juce::TextButton& b)
{
    b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a1a1a));
    b.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffd4af37));
    b.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    b.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffd4af37));
}
void AjerryVST3AudioProcessorEditor::styleKnob (juce::Slider& s, juce::Label& l, juce::String name)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffd4af37));
    s.setColour (juce::Slider::thumbColourId, juce::Colour (0xffd4af37));
    s.setRange (0.0, 100.0, 0.1);
    s.setValue (50.0);
    addAndMakeVisible (s);
    l.setText (name, juce::dontSendNotification);
    l.setFont (juce::Font (13.0f, juce::Font::bold));
    l.setColour (juce::Label::textColourId, juce::Colour (0xffd4af37));
    l.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (l);
}
void AjerryVST3AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setColour (juce::Colour (0xffd4af37));
    g.drawRect (getLocalBounds(), 3);
    g.setColour (juce::Colour (0xff1a1a1a));
    if (currentPage == 0)
    {
        g.fillRect (30, 130, 400, 260);
        g.fillRect (460, 130, 410, 260);
        g.setColour (juce::Colour (0xffd4af37));
        g.drawRect (30, 130, 400, 260, 2);
        g.drawRect (460, 130, 410, 260, 2);
        g.setFont (juce::Font (16.0f, juce::Font::bold));
        g.drawText ("LISTEN", 30, 140, 400, 25, juce::Justification::centred);
        g.drawText ("THRESHOLD / RANGE", 460, 140, 410, 25, juce::Justification::centred);
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (13.0f));
        g.drawText ("Step 1 of 8 - Only one opens at a time - Cleans noise, clicks and unwanted sounds", 30, 400, 840, 30, juce::Justification::centred);
        g.drawText ("GATE OPEN / GATE CLOSED", 30, 430, 840, 25, juce::Justification::centred);
    }
    else if (currentPage == 1)
    {
        g.fillRect (30, 130, 430, 280);
        g.fillRect (490, 130, 380, 280);
        g.setColour (juce::Colour (0xffd4af37));
        g.drawRect (30, 130, 430, 280, 2);
        g.drawRect (490, 130, 380, 280, 2);
        g.setFont (juce::Font (16.0f, juce::Font::bold));
        g.drawText ("BEAT KEY DETECTOR", 30, 140, 430, 25, juce::Justification::centred);
        g.drawText ("VOCAL AUTO-TUNE", 490, 140, 380, 25, juce::Justification::centred);
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (14.0f));
        g.drawText ("Key: A Minor  BPM: 110  [LISTEN]", 30, 180, 430, 25, juce::Justification::centred);
        g.drawText ("AUTO DETECT KEY - 1 MIN  [LOCK THIS KEY]", 30, 210, 430, 25, juce::Justification::centred);
        g.drawText ("KEY: C  [BYPASS]", 490, 300, 380, 25, juce::Justification::centred);
    }
    else if (currentPage == 3)
    {
        g.fillRect (30, 130, 840, 200);
        g.setColour (juce::Colour (0xffd4af37));
        g.drawRect (30, 130, 840, 200, 2);
        g.setFont (juce::Font (16.0f, juce::Font::bold));
        g.drawText ("THRESHOLD -18   RATIO 3   GAIN NORMAL +4dB", 30, 140, 840, 25, juce::Justification::centred);
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (13.0f));
        g.drawText ("GAIN REDUCTION METER - GREEN GOOD / YELLOW CAREFUL / RED TOO MUCH", 30, 350, 840, 25, juce::Justification::centred);
    }
    else if (currentPage == 4)
    {
        g.fillRect (30, 130, 260, 180);
        g.fillRect (320, 130, 260, 180);
        g.fillRect (610, 130, 260, 180);
        g.setColour (juce::Colour (0xffd4af37));
        g.drawRect (30, 130, 260, 180, 2);
        g.drawRect (320, 130, 260, 180, 2);
        g.drawRect (610, 130, 260, 180, 2);
        g.setFont (juce::Font (15.0f, juce::Font::bold));
        g.drawText ("MODE", 30, 140, 260, 25, juce::Justification::centred);
        g.drawText ("AMOUNT - 50%", 320, 140, 260, 25, juce::Justification::centred);
        g.drawText ("ANALYSIS", 610, 140, 260, 25, juce::Justification::centred);
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (13.0f));
        g.drawText ("Guitar / Vocals / Mix Bus", 30, 180, 260, 25, juce::Justification::centred);
        g.drawText ("AJERRY LEARNING METER", 30, 350, 840, 25, juce::Justification::centred);
    }
    else if (currentPage == 5)
    {
        g.fillRect (30, 130, 260, 180);
        g.fillRect (320, 130, 260, 180);
        g.fillRect (610, 130, 260, 180);
        g.setColour (juce::Colour (0xffd4af37));
        g.drawRect (30, 130, 260, 180, 2);
        g.drawRect (320, 130, 260, 180, 2);
        g.drawRect (610, 130, 260, 180, 2);
        g.setFont (juce::Font (15.0f, juce::Font::bold));
        g.drawText ("MODE", 30, 140, 260, 25, juce::Justification::centred);
        g.drawText ("AMOUNT - MIX 60%", 320, 140, 260, 25, juce::Justification::centred);
        g.drawText ("ANALYSIS", 610, 140, 260, 25, juce::Justification::centred);
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (13.0f));
        g.drawText ("Verse / Chorus / Rap / Trap / Hook", 30, 180, 260, 25, juce::Justification::centred);
        g.drawText ("AJERRY LEARNING METER", 30, 350, 840, 25, juce::Justification::centred);
    }
    else if (currentPage == 6)
    {
        g.fillRect (30, 130, 260, 180);
        g.fillRect (320, 130, 260, 180);
        g.fillRect (610, 130, 260, 180);
        g.setColour (juce::Colour (0xffd4af37));
        g.drawRect (30, 130, 260, 180, 2);
        g.drawRect (320, 130, 260, 180, 2);
        g.drawRect (610, 130, 260, 180, 2);
        g.setFont (juce::Font (15.0f, juce::Font::bold));
        g.drawText ("TIME / SPEED - 300ms", 30, 140, 260, 25, juce::Justification::centred);
        g.drawText ("DELAY / ECHO - 10%", 320, 140, 260, 25, juce::Justification::centred);
        g.drawText ("ANALYSIS", 610, 140, 260, 25, juce::Justification::centred);
    }
    else if (currentPage == 7)
    {
        g.fillRect (30, 130, 260, 180);
        g.fillRect (320, 130, 260, 180);
        g.fillRect (610, 130, 260, 180);
        g.setColour (juce::Colour (0xffd4af37));
        g.drawRect (30, 130, 260, 180, 2);
        g.drawRect (320, 130, 260, 180, 2);
        g.drawRect (610, 130, 260, 180, 2);
        g.setFont (juce::Font (15.0f, juce::Font::bold));
        g.drawText ("TIME / SIZE - Studio 300ms", 30, 140, 260, 25, juce::Justification::centred);
        g.drawText ("REVERB / ROOM - 20% CLEAR VOICE", 320, 140, 260, 25, juce::Justification::centred);
        g.drawText ("ANALYSIS", 610, 140, 260, 25, juce::Justification::centred);
    }
    else if (currentPage == 2)
    {
        g.fillRect (30, 130, 840, 200);
        g.setColour (juce::Colour (0xffd4af37));
        g.drawRect (30, 130, 840, 200, 2);
        g.setFont (juce::Font (16.0f, juce::Font::bold));
        g.drawText ("EQ - LOW / MID / HIGH", 30, 140, 840, 25, juce::Justification::centred);
    }
}
void AjerryVST3AudioProcessorEditor::resized()
{
    titleLabel.setBounds (0, 10, 900, 35);
    pageLabel.setBounds (0, 90, 900, 30);
    int x = 30;
    juce::TextButton* btns[8] = { &gateButton, &spotonButton, &eqButton, &compButton, &softenButton, &doublerButton, &delayButton, &reverbButton };
    for (int i = 0; i < 8; ++i)
    {
        btns[i]->setBounds (x, 55, 105, 30);
        btns[i]->setToggleState (i == currentPage, juce::dontSendNotification);
        x += 107;
    }
    if (currentPage == 0)
    {
        knob1.setBounds (500, 180, 120, 120); label1.setBounds (500, 300, 120, 20);
        knob2.setBounds (660, 180, 120, 120); label2.setBounds (660, 300, 120, 20);
        knob1.setVisible (true); label1.setVisible (true);
        knob2.setVisible (true); label2.setVisible (true);
        knob3.setVisible (false); label3.setVisible (false);
        knob4.setVisible (false); label4.setVisible (false);
        knob5.setVisible (false); label5.setVisible (false);
        knob6.setVisible (false); label6.setVisible (false);
    }
    else if (currentPage == 1)
    {
        knob1.setBounds (530, 180, 120, 120); label1.setBounds (530, 300, 120, 20);
        knob2.setBounds (660, 180, 120, 120); label2.setBounds (660, 300, 120, 20);
        knob1.setVisible (true); label1.setVisible (true);
        knob2.setVisible (true); label2.setVisible (true);
        knob3.setVisible (false); label3.setVisible (false);
        knob4.setVisible (false); label4.setVisible (false);
        knob5.setVisible (false); label5.setVisible (false);
        knob6.setVisible (false); label6.setVisible (false);
    }
    else if (currentPage == 3)
    {
        knob1.setBounds (100, 180, 120, 120); label1.setBounds (100, 300, 120, 20);
        knob2.setBounds (390, 180, 120, 120); label2.setBounds (390, 300, 120, 20);
        knob3.setBounds (680, 180, 120, 120); label3.setBounds (680, 300, 120, 20);
        knob1.setVisible (true); label1.setVisible (true);
        knob2.setVisible (true); label2.setVisible (true);
        knob3.setVisible (true); label3.setVisible (true);
        knob4.setVisible (false); label4.setVisible (false);
        knob5.setVisible (false); label5.setVisible (false);
        knob6.setVisible (false); label6.setVisible (false);
    }
    else
    {
        knob1.setBounds (390, 180, 120, 120); label1.setBounds (390, 300, 120, 20);
        knob2.setBounds (520, 180, 90, 90); label2.setBounds (520, 270, 90, 20);
        knob1.setVisible (true); label1.setVisible (true);
        knob2.setVisible (currentPage == 6 || currentPage == 7 || currentPage == 2);
        label2.setVisible (currentPage == 6 || currentPage == 7 || currentPage == 2);
        knob3.setVisible (false); label3.setVisible (false);
        knob4.setVisible (false); label4.setVisible (false);
        knob5.setVisible (false); label5.setVisible (false);
        knob6.setVisible (false); label6.setVisible (false);
    }
}
void AjerryVST3AudioProcessorEditor::buttonClicked (juce::Button* b)
{
    if (b == &gateButton) currentPage = 0;
    else if (b == &spotonButton) currentPage = 1;
    else if (b == &eqButton) currentPage = 2;
    else if (b == &compButton) currentPage = 3;
    else if (b == &softenButton) currentPage = 4;
    else if (b == &doublerButton) currentPage = 5;
    else if (b == &delayButton) currentPage = 6;
    else if (b == &reverbButton) currentPage = 7;
    updateLabelsForPage();
    resized();
    repaint();
}
void AjerryVST3AudioProcessorEditor::updateLabelsForPage()
{
    juce::String names[8] = { "1  GATE", "2  SPOTON", "3  EQ", "4  COMP", "5  SOFTEN", "6  DOUBLER", "7  DELAY", "8  REVERB" };
    pageLabel.setText (names[currentPage], juce::dontSendNotification);
    if (currentPage == 0) { label1.setText ("THRESHOLD", juce::dontSendNotification); label2.setText ("RANGE", juce::dontSendNotification); }
    else if (currentPage == 1) { label1.setText ("AMOUNT", juce::dontSendNotification); label2.setText ("SPEED", juce::dontSendNotification); }
    else if (currentPage == 2) { label1.setText ("LOW", juce::dontSendNotification); label2.setText ("MID", juce::dontSendNotification); }
    else if (currentPage == 3) { label1.setText ("THRESHOLD", juce::dontSendNotification); label2.setText ("RATIO", juce::dontSendNotification); label3.setText ("GAIN", juce::dontSendNotification); }
    else if (currentPage == 4) { label1.setText ("AMOUNT - 50%", juce::dontSendNotification); }
    else if (currentPage == 5) { label1.setText ("MIX 60%", juce::dontSendNotification); }
    else if (currentPage == 6) { label1.setText ("TIME 300ms", juce::dontSendNotification); label2.setText ("ECHO 10%", juce::dontSendNotification); }
    else if (currentPage == 7) { label1.setText ("SIZE 300ms", juce::dontSendNotification); label2.setText ("ROOM 20%", juce::dontSendNotification); }
}
