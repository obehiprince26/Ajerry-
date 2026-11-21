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
 page=p; for(auto& attachment : attachments) attachment.reset(); for(int i=0;i<8;++i){tabs[i].setColour(juce::TextButton::buttonColourId,juce::Colours::black);tabs[i].setColour(juce::TextButton::textColourOffId,gold);tabs[i].setColour(juce::TextButton::buttonOnColourId,gold);tabs[i].setColour(juce::TextButton::textColourOnId,juce::Colours::black);tabs[i].setToggleState(i==p,juce::dontSendNotification);}
 juce::String a[3],v[3],f;
 switch(p){
 case 0:a[0]="THRESHOLD";a[1]="RANGE";a[2]="ATTACK";v[0]="-40 dB";v[1]="0";v[2]="OPEN / CLOSED";f="Step 1 of 8 - Attack Hold Release AUTO - Always ON";break;
 case 1:a[0]="AMOUNT";a[1]="SPEED";a[2]="KEY";v[0]="0";v[1]="0";v[2]="A Minor";f="AJERRY - Formant Preservation - Smooth Correction - Always ON";break;
 case 2:a[0]="HIGH-PASS";a[1]="LOW-PASS";a[2]="EQ GAIN";v[0]="80 Hz";v[1]="16 kHz";v[2]="0 dB";f="AJERRY EQ PRO - HP cuts rumble - LP cuts hiss - Press APPLY";break;
 case 3:a[0]="THRESHOLD";a[1]="RATIO";a[2]="GAIN";v[0]="-18";v[1]="3";v[2]="+4 dB";f="AJERRY COMP PRO - SERIAL INSIDE - Attack Auto - Release Auto";break;
 case 4:a[0]="AMOUNT";a[1]="FREQUENCY";a[2]="MIX";v[0]="50%";v[1]="8500 Hz";v[2]="35%";f="Learning meter - Listen 1 min - Sing S and SH words";break;
 case 5:a[0]="AMOUNT";a[1]="WIDTH";a[2]="DELAY";v[0]="60%";v[1]="60%";v[2]="18 ms";f="Learning meter - Verse / Chorus / Rap / Trap / Hook";break;
 case 6:a[0]="TIME";a[1]="MIX";a[2]="FEEDBACK";v[0]="300 ms";v[1]="10%";v[2]="20%";f="AJERRY SIMPLE DELAY - Time controls speed - Delay controls echo amount";break;
 default:a[0]="ROOM SIZE";a[1]="MIX";a[2]="WIDTH";v[0]="300 ms";v[1]="20%";v[2]="100%";f="AJERRY IMPROVED REVERB - Time shows room - Voice stays clear";break;
 }
 const char* mapped[8][3] = {
  {"gate","gateRange","gateAttack"},
  {"tune","tuneSpeed","tuneKey"},
  {"hp","lp","eqGain"},
  {"threshold","ratio","gain"},
  {"soften","softenFreq","softenMix"},
  {"doubler","doublerWidth","doublerDelay"},
  {"delayTime","delayMix","delayFeedback"},
  {"reverbTime","reverbMix","reverbWidth"}
 };
 for(int i=0;i<3;++i){
  labels[i].setText(a[i],juce::dontSendNotification);
  vals[i].setText(v[i],juce::dontSendNotification);
  knobs[i].setEnabled(true);
  knobs[i].onValueChange=[this,i]{ vals[i].setText(knobs[i].getTextFromValue(knobs[i].getValue()),juce::dontSendNotification); };
  attachments[i]=std::make_unique<SliderAttachment>(proc.parameters,mapped[p][i],knobs[i]);
  vals[i].setText(knobs[i].getTextFromValue(knobs[i].getValue()),juce::dontSendNotification);
 }
 footer.setText(f,juce::dontSendNotification);repaint();
}
void AjerryVST3AudioProcessorEditor::paint(juce::Graphics& g){g.fillAll(juce::Colours::black);g.setColour(gold);g.drawRect(getLocalBounds(),2);auto area=getLocalBounds().reduced(18);area.removeFromTop(100);int w=area.getWidth()/3;for(int i=0;i<3;++i){auto box=juce::Rectangle<int>(area.getX()+i*w,area.getY(),w-10,area.getHeight()-50);g.setColour(juce::Colour(0xff242424));g.fillRoundedRectangle(box.toFloat(),8);g.setColour(gold);g.drawRoundedRectangle(box.toFloat(),8,1.5f);g.setColour(blue);g.fillRoundedRectangle(box.reduced(10).toFloat(),6);}}
void AjerryVST3AudioProcessorEditor::resized(){auto r=getLocalBounds().reduced(18);title.setBounds(r.removeFromTop(38));r.removeFromTop(5);int tw=r.getWidth()/8;for(int i=0;i<8;++i)tabs[i].setBounds(r.getX()+i*tw,r.getY(),tw-2,36);r.removeFromTop(18);int cw=r.getWidth()/3;for(int i=0;i<3;++i){auto b=juce::Rectangle<int>(r.getX()+i*cw,r.getY(),cw-10,r.getHeight()-40);labels[i].setBounds(b.removeFromTop(32));knobs[i].setBounds(b.removeFromTop(150).reduced(28,10));vals[i].setBounds(b.removeFromTop(28));}footer.setBounds(getWidth()/2-400,getHeight()-35,800,28);}
