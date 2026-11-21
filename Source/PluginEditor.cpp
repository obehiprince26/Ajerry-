#include "PluginEditor.h"
static const juce::Colour gold{0xffd4af37}, blue{0xff123c80};
AjerryVST3AudioProcessorEditor::AjerryVST3AudioProcessorEditor(AjerryVST3AudioProcessor& p):AudioProcessorEditor(&p),proc(p){
 setSize(900,520);title.setText("AJERRY CHANNEL STRIP - Compact",juce::dontSendNotification);title.setColour(juce::Label::textColourId,gold);title.setFont(juce::Font(22.0f,juce::Font::bold));title.setJustificationType(juce::Justification::centred);addAndMakeVisible(title);
 const char* names[]={"1 GATE","2 SPOTON","3 EQ","4 COMP","5 SOFTEN","6 DOUBLER","7 DELAY","8 REVERB"};
 for(int i=0;i<8;++i){tabs[i].setButtonText(names[i]);tabs[i].onClick=[this,i]{changeTab(i);};addAndMakeVisible(tabs[i]);}
 for(int i=0;i<3;++i){knobs[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,20);knobs[i].setColour(juce::Slider::rotarySliderFillColourId,gold);knobs[i].setColour(juce::Slider::thumbColourId,gold);addAndMakeVisible(knobs[i]);labels[i].setColour(juce::Label::textColourId,gold);labels[i].setJustificationType(juce::Justification::centred);addAndMakeVisible(labels[i]);vals[i].setColour(juce::Label::textColourId,juce::Colours::white);vals[i].setJustificationType(juce::Justification::centred);addAndMakeVisible(vals[i]);}
 footer.setColour(juce::Label::textColourId,gold);footer.setJustificationType(juce::Justification::centred);addAndMakeVisible(footer);changeTab(0);
}
void AjerryVST3AudioProcessorEditor::changeTab(int p){
 page=p;
 for(int i=0;i<8;++i){tabs[i].setColour(juce::TextButton::buttonColourId,juce::Colours::black);tabs[i].setColour(juce::TextButton::textColourOffId,gold);tabs[i].setColour(juce::TextButton::buttonOnColourId,gold);tabs[i].setColour(juce::TextButton::textColourOnId,juce::Colours::black);tabs[i].setToggleState(i==p,juce::dontSendNotification);}
 juce::String labelsForPage[3],values[3],footerText;
 const char* parameterIds[8][3]={{"gate","gateRange","gateAttack"},{"tune","tuneSpeed","tuneKey"},{"hp","lp","eqAmount"},{"threshold","ratio","gain"},{"soften","softenFreq","softenMix"},{"doubler","doublerDelay","doublerWidth"},{"delayTime","delayMix","delayFeedback"},{"reverbTime","reverbMix","reverbPreDelay"}};
 switch(p){
 case 0: labelsForPage[0]="THRESHOLD";labelsForPage[1]="RANGE";labelsForPage[2]="ATTACK";values[0]="dB";values[1]="%";values[2]="ms";footerText="Step 1 of 8 - Gate threshold, attenuation range and attack";break;
 case 1: labelsForPage[0]="AMOUNT";labelsForPage[1]="SPEED";labelsForPage[2]="KEY";values[0]="%";values[1]="%";values[2]="Key";footerText="SPOTON - Correction amount, response speed and tonic key";break;
 case 2: labelsForPage[0]="HIGH-PASS";labelsForPage[1]="LOW-PASS";labelsForPage[2]="EQ AMOUNT";values[0]="Hz";values[1]="Hz";values[2]="%";footerText="AJERRY EQ - High-pass, low-pass and EQ intensity";break;
 case 3: labelsForPage[0]="THRESHOLD";labelsForPage[1]="RATIO";labelsForPage[2]="GAIN";values[0]="dB";values[1]=":1";values[2]="dB";footerText="AJERRY COMP - Dynamics threshold, ratio and makeup gain";break;
 case 4: labelsForPage[0]="SOFTEN";labelsForPage[1]="FREQUENCY";labelsForPage[2]="MIX";values[0]="%";values[1]="Hz";values[2]="%";footerText="SOFTEN - Amount, target frequency and blend";break;
 case 5: labelsForPage[0]="AMOUNT";labelsForPage[1]="DELAY";labelsForPage[2]="WIDTH";values[0]="%";values[1]="ms";values[2]="%";footerText="DOUBLER - Blend, voice offset and stereo width";break;
 case 6: labelsForPage[0]="TIME";labelsForPage[1]="MIX";labelsForPage[2]="FEEDBACK";values[0]="ms";values[1]="%";values[2]="%";footerText="DELAY - Time, wet mix and feedback";break;
 default: labelsForPage[0]="SIZE / TIME";labelsForPage[1]="MIX";labelsForPage[2]="PRE-DELAY";values[0]="ms";values[1]="%";values[2]="ms";footerText="REVERB - Room/decay, wet blend and pre-delay";break;
 }
 for(int i=0;i<3;++i){
   attachments[i].reset();
   knobs[i].setRange(0.0,100.0,0.01);
   knobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,20);
   labels[i].setText(labelsForPage[i],juce::dontSendNotification);
   vals[i].setText(values[i],juce::dontSendNotification);
   attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.parameters,parameterIds[p][i],knobs[i]);
 }
 footer.setText(footerText,juce::dontSendNotification);repaint();
}
void AjerryVST3AudioProcessorEditor::paint(juce::Graphics& g){g.fillAll(juce::Colours::black);g.setColour(gold);g.drawRect(getLocalBounds(),2);auto area=getLocalBounds().reduced(18);area.removeFromTop(100);int w=area.getWidth()/3;for(int i=0;i<3;++i){auto box=juce::Rectangle<int>(area.getX()+i*w,area.getY(),w-10,area.getHeight()-50);g.setColour(juce::Colour(0xff242424));g.fillRoundedRectangle(box.toFloat(),8);g.setColour(gold);g.drawRoundedRectangle(box.toFloat(),8,1.5f);g.setColour(blue);g.fillRoundedRectangle(box.reduced(10).toFloat(),6);}}
void AjerryVST3AudioProcessorEditor::resized(){auto r=getLocalBounds().reduced(18);title.setBounds(r.removeFromTop(38));r.removeFromTop(5);int tw=r.getWidth()/8;for(int i=0;i<8;++i)tabs[i].setBounds(r.getX()+i*tw,r.getY(),tw-2,36);r.removeFromTop(18);int cw=r.getWidth()/3;for(int i=0;i<3;++i){auto b=juce::Rectangle<int>(r.getX()+i*cw,r.getY(),cw-10,r.getHeight()-40);labels[i].setBounds(b.removeFromTop(32));knobs[i].setBounds(b.removeFromTop(150).reduced(28,10));vals[i].setBounds(b.removeFromTop(28));}footer.setBounds(getWidth()/2-400,getHeight()-35,800,28);}
