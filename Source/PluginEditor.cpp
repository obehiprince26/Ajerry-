#include "PluginEditor.h"

namespace {
const juce::Colour gold{0xffd4af37}, paleGold{0xffffdf75}, blue{0xff07366f}, brightBlue{0xff168cff};
const juce::Colour green{0xff19d52a}, yellow{0xffffdf00}, red{0xffed2020}, cyan{0xff00d8ee};
juce::String centredText(juce::Graphics& g, const juce::String& s, juce::Rectangle<float> r, float size, juce::Colour c = paleGold, bool bold = true) {
    g.setColour(c); g.setFont(juce::Font(size, bold ? juce::Font::bold : juce::Font::plain)); g.drawFittedText(s, r.toNearestInt(), juce::Justification::centred, 2); return s;
}
}

AjerryVST3AudioProcessorEditor::AjerryVST3AudioProcessorEditor(AjerryVST3AudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setSize(1440, 900);
    setResizable(true, true);
    for (int i=0;i<8;++i) {
        tabs[i].setButtonText(juce::String(i+1) + " " + juce::StringArray{"GATE","SPOTON","EQ","COMP","SOFTEN","DOUBLER","DELAY","REVERB"}[i]);
        tabs[i].setClickingTogglesState(false);
        tabs[i].onClick=[this,i]{ changeTab(i); };
        tabs[i].setColour(juce::TextButton::buttonColourId, juce::Colours::black);
        tabs[i].setColour(juce::TextButton::textColourOffId, gold);
        tabs[i].setColour(juce::TextButton::buttonOnColourId, gold);
        tabs[i].setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        addAndMakeVisible(tabs[i]);
    }
    changeTab(0);
}

void AjerryVST3AudioProcessorEditor::changeTab(int p) {
    page=juce::jlimit(0,7,p);
    for(int i=0;i<8;++i) tabs[i].setToggleState(i==page,juce::dontSendNotification);
    const juce::String footers[] = {
      "Step 1 of 8 - Only one opens at a time, Inbuilt: Attack Hold Release AUTO - Always ON",
      "AJERRY - Formant Preservation - Smooth Correction - Better Pitch Detection - Always ON",
      "AJERRY EQ PRO - HP cuts rumble, LP cuts hiss - Drag dots to shape tone - Listen 1 min - Press APPLY",
      "AJERRY COMP PRO - SERIAL INSIDE - NORMAL GAIN - Listen 1 min - Attack Auto Release Auto",
      "AJERRY LEARNING METER - PRESS LISTEN - Listen 1 min - Sing S and SH words - PRESS STOP",
      "AJERRY LEARNING METER - PRESS LISTEN - Listen 1 min - Sing S and SH words - PRESS STOP",
      "AJERRY SIMPLE DELAY - TIME controls speed - DELAY controls echo amount",
      "AJERRY IMPROVED REVERB - TIME shows room name - REVERB keeps voice clear"
    };
    repaint();
}

void AjerryVST3AudioProcessorEditor::drawBox(juce::Graphics& g, juce::Rectangle<float> r, const juce::String& heading) {
    g.setColour(juce::Colour(0xff050505)); g.fillRoundedRectangle(r,10.0f);
    g.setColour(gold); g.drawRoundedRectangle(r,10.0f,2.4f);
    g.setColour(juce::Colour(0xff9c741c)); g.drawLine(r.getX()+5,r.getY()+42,r.getRight()-5,r.getY()+42,1.2f);
    centredText(g,heading,{r.getX()+5,r.getY()+3,r.getWidth()-10,36},juce::jmax(15.0f,r.getWidth()*0.035f),paleGold);
}

void AjerryVST3AudioProcessorEditor::drawKnob(juce::Graphics& g, juce::Point<float> c, float radius, const juce::String& label, const juce::String& value) {
    g.setColour(gold);
    for(int i=0;i<28;++i){float a=juce::MathConstants<float>::twoPi*i/28.0f;juce::Point<float> p{c.x+std::sin(a)*(radius+5),c.y-std::cos(a)*(radius+5)};g.drawLine(p.x,p.y,p.x+std::sin(a)*7,p.y-std::cos(a)*7,3.0f);}
    g.setColour(juce::Colour(0xffa97913));g.fillEllipse(c.x-radius,c.y-radius,2*radius,2*radius);
    g.setColour(juce::Colour(0xffffd65a));g.fillEllipse(c.x-radius+5,c.y-radius+5,2*radius-10,2*radius-10);
    g.setColour(juce::Colour(0xff8a5d09));g.drawEllipse(c.x-radius+5,c.y-radius+5,2*radius-10,2*radius-10,2.0f);
    g.setColour(juce::Colours::black);g.drawLine(c.x,c.y-radius+12,c.x,c.y-radius+radius*0.55f,5.0f);
    centredText(g,label,{c.x-radius-38,c.y+radius+12,2*radius+76,27},juce::jmax(13.0f,radius*.27f));
    centredText(g,value,{c.x-radius-38,c.y+radius+37,2*radius+76,25},juce::jmax(12.0f,radius*.24f));
}

void AjerryVST3AudioProcessorEditor::drawMeter(juce::Graphics& g, juce::Rectangle<float> r, bool numbers) {
    const float y=r.getY()+r.getHeight()*.38f, h=juce::jmax(18.0f,r.getHeight()*.19f);
    if(numbers){const char* ns[]={"0","-3","-6","-12","-18"};for(int i=0;i<5;++i)centredText(g,ns[i],{r.getX()+r.getWidth()*i/4-24,r.getY()+2,48,25},18,paleGold);}
    auto bar=juce::Rectangle<float>(r.getX()+r.getWidth()*.06f,y,r.getWidth()*.88f,h);
    g.setColour(green);g.fillRect(juce::Rectangle<float>(bar.getX(),bar.getY(),bar.getWidth()*.27f,h));
    g.setColour(yellow);g.fillRect(juce::Rectangle<float>(bar.getX()+bar.getWidth()*.27f,bar.getY(),bar.getWidth()*.27f,h));
    g.setColour(juce::Colour(0xffff8700));g.fillRect(juce::Rectangle<float>(bar.getX()+bar.getWidth()*.54f,bar.getY(),bar.getWidth()*.18f,h));
    g.setColour(red);g.fillRect(juce::Rectangle<float>(bar.getX()+bar.getWidth()*.72f,bar.getY(),bar.getWidth()*.28f,h));
    g.setColour(juce::Colours::white);g.drawRect(bar,1.0f);
    float arrowX=bar.getX()+bar.getWidth()*(page==2?.285f:.30f);
    juce::Path arrow;arrow.addTriangle(arrowX-12,y-10,arrowX+12,y-10,arrowX,y+9);g.fillPath(arrow);
    centredText(g,"GREEN GOOD / YELLOW CAREFUL / RED TOO MUCH",{r.getX(),y+h+5,r.getWidth(),25},18,paleGold);
    if(page==2) centredText(g,"GREEN = Gentle natural keep arrow here, YELLOW = Careful listen, RED = Too much bring dot back to 0",{r.getX()+5,y+h+29,r.getWidth()-10,24},14,paleGold,false);
}

void AjerryVST3AudioProcessorEditor::drawWaveform(juce::Graphics& g, juce::Rectangle<float> r, int kind) {
    g.setColour(blue);g.fillRoundedRectangle(r,12);g.setColour(brightBlue);g.drawRoundedRectangle(r,12,2);
    juce::Path wave;float mid=r.getCentreY();wave.startNewSubPath(r.getX()+5,mid);
    for(int i=0;i<160;++i){float x=r.getX()+5+(r.getWidth()-10)*i/159.0f;float t=i/159.0f;float envelope=kind==0?(0.12f+0.88f*std::exp(-std::pow((t-.5f)*4.0f,2))):(kind==1?std::exp(-t*3.4f):std::exp(-t*2.3f));float v=std::sin(i*1.73f)*std::sin(i*.31f)*r.getHeight()*.42f*envelope;wave.lineTo(x,mid+v);}
    g.setColour(juce::Colour(0xff25baff));g.strokePath(wave,juce::PathStrokeType(2.0f));
}

void AjerryVST3AudioProcessorEditor::paint(juce::Graphics& g) {
    auto b=getLocalBounds().toFloat();g.fillAll(juce::Colours::black);
    g.setColour(gold);g.drawRoundedRectangle(b.reduced(3),8,3);
    centredText(g,"AJERRY CHANNEL STRIP - Compact",{10,5,b.getWidth()-20,52},juce::jmin(38.0f,b.getWidth()*.029f),paleGold);
    auto area=b.reduced(24);float top=84.0f;float tabBottom=top+54.0f;
    for(int i=0;i<8;++i){auto x=area.getX()+area.getWidth()*i/8.0f;auto w=area.getWidth()/8.0f;g.setColour(i==page?gold:juce::Colours::black);g.fillRoundedRectangle(x+2,top,w-4,48,5);g.setColour(gold);g.drawRoundedRectangle(x+2,top,w-4,48,5,2);}
    auto bodyTop=tabBottom+12, footerY=b.getHeight()-46;
    auto footer=juce::Rectangle<float>(24,footerY,b.getWidth()-48,30);g.setColour(gold);g.drawRoundedRectangle(footer,5,1.5f);
    const juce::String footers[]={"Step 1 of 8 - Only one opens at a time, Inbuilt: Attack Hold Release AUTO - Always ON","AJERRY - Formant Preservation - Smooth Correction - Better Pitch Detection - Always ON","AJERRY EQ PRO - HP cuts rumble, LP cuts hiss - Drag dots to shape tone - Listen 1 min - Press APPLY","AJERRY COMP PRO - SERIAL INSIDE - NORMAL GAIN - Listen 1 min - Attack Auto Release Auto","AJERRY LEARNING METER - PRESS LISTEN - Listen 1 min - Sing S and SH words - PRESS STOP","AJERRY LEARNING METER - PRESS LISTEN - Listen 1 min - Sing S and SH words - PRESS STOP","AJERRY SIMPLE DELAY - TIME controls speed - DELAY controls echo amount","AJERRY IMPROVED REVERB - TIME shows room name - REVERB keeps voice clear"};
    centredText(g,footers[page],footer.reduced(4),juce::jmin(17.0f,b.getWidth()*.013f),paleGold);
    auto content=juce::Rectangle<float>(area.getX(),bodyTop,area.getWidth(),footerY-bodyTop-12);
    float gap=12.0f;
    if(page==0 || page==1){
        auto left=content.withWidth(content.getWidth()*.47f-gap*.5f),right=content.withX(left.getRight()+gap).withWidth(content.getWidth()-left.getWidth()-gap);
        drawBox(g,left,page==0?"LISTEN":"BEAT KEY DETECTOR");drawBox(g,right,page==0?"GATE CONTROLS":"VOCAL AUTO-TUNE");
        if(page==0){
            auto c=juce::Point<float>(left.getCentreX(),left.getY()+left.getHeight()*.28f);g.setColour(red);g.fillEllipse(c.x-48,c.y-48,96,96);g.setColour(gold);g.drawEllipse(c.x-48,c.y-48,96,96,3);centredText(g,"LISTEN",{c.x-44,c.y-22,88,44},22,juce::Colours::black);
            centredText(g,"Listen to room noise 1 min",{left.getX()+10,c.y+55,left.getWidth()-20,28},17);
            centredText(g,"SUGGESTED THRESHOLD: ---\nSUGGESTED RANGE: ---\nPress LISTEN to show suggestion\nSuggestion Only, You Set It Yourself",{left.getX()+12,c.y+88,left.getWidth()-24,left.getHeight()*.32f},17);
            centredText(g,"GATE OPEN                 GATE CLOSED",{left.getX()+8,left.getBottom()-55,left.getWidth()-16,34},17);
            drawKnob(g,{right.getX()+right.getWidth()*.3f,right.getY()+right.getHeight()*.43f},48,"THRESHOLD","0");drawKnob(g,{right.getX()+right.getWidth()*.7f,right.getY()+right.getHeight()*.43f},48,"RANGE","0");centredText(g,"BYPASS",{right.getX()+right.getWidth()*.3f,right.getBottom()-80,right.getWidth()*.4f,40},20);
        }else{
            auto c=juce::Point<float>(left.getCentreX(),left.getY()+left.getHeight()*.23f);g.setColour(red);g.fillEllipse(c.x-48,c.y-48,96,96);centredText(g,"LISTEN",{c.x-44,c.y-20,88,40},22,juce::Colours::black);
            centredText(g,"Listen for 1 Minute\n\nKey: A Minor\nKey Lock ON\nBPM: 110\nConfidence: 95%\nAUTO APPLIED",{left.getX()+12,c.y+52,left.getWidth()-24,left.getHeight()*.43f},19);
            centredText(g,"AUTO DETECT KEY - 1 MIN",{left.getX()+12,left.getBottom()-112,left.getWidth()*.52f,42},16,juce::Colour(0xff78baff));
            centredText(g,"LOCK THIS KEY",{left.getX()+left.getWidth()*.55f,left.getBottom()-112,left.getWidth()*.42f,42},16,green);
            centredText(g,"DETECTED: ---",{left.getX()+10,left.getBottom()-54,left.getWidth()-20,30},18);
            auto keyb=juce::Rectangle<float>(right.getX()+20,right.getY()+65,right.getWidth()-40,right.getHeight()*.30f);g.setColour(juce::Colours::white);g.fillRoundedRectangle(keyb,5);for(int i=0;i<14;++i){float x=keyb.getX()+i*keyb.getWidth()/14;g.setColour((i==2||i==3||i==5||i==8||i==10)?green:juce::Colours::white);g.fillRect(x,keyb.getY()+keyb.getHeight()*.55f,keyb.getWidth()/14-1,keyb.getHeight()*.45f);}
            drawKnob(g,{right.getX()+right.getWidth()*.30f,right.getY()+right.getHeight()*.63f},48,"AMOUNT","0");drawKnob(g,{right.getX()+right.getWidth()*.65f,right.getY()+right.getHeight()*.63f},48,"SPEED","0");centredText(g,"KEY: C  ▼",{right.getX()+right.getWidth()*.70f,right.getBottom()-75,right.getWidth()*.25f,35},18);centredText(g,"BYPASS",{right.getX()+right.getWidth()*.70f,right.getBottom()-40,right.getWidth()*.25f,30},18);
        }
    } else if(page==2){
        centredText(g,"AJERRY EQ PRO - CLEAN + SHAPE",{content.getX(),content.getY(),content.getWidth(),30},21);
        auto row=content.withY(content.getY()+34).withHeight(content.getHeight()*.58f-35);
        float w=row.getWidth()*.26f;auto listen=row.withWidth(w);auto filters=row.withX(listen.getRight()+gap).withWidth(w);auto analysis=row.withX(filters.getRight()+gap).withWidth(row.getRight()-filters.getRight()-gap);
        drawBox(g,listen,"LISTEN");drawBox(g,filters,"HIGH-PASS / LOW-PASS");drawBox(g,analysis,"ANALYSIS - EQ CURVE");
        auto c=juce::Point<float>(listen.getCentreX(),listen.getY()+listen.getHeight()*.28f);g.setColour(red);g.fillEllipse(c.x-43,c.y-43,86,86);centredText(g,"LISTEN",{c.x-40,c.y-20,80,40},20,juce::Colours::black);
        eqListenBounds={c.x-48,c.y-48,96,96};
        const auto hpNow=proc.parameters.getRawParameterValue("hp")->load();
        const auto lpNow=proc.parameters.getRawParameterValue("lp")->load();
        centredText(g,juce::String("Listen to your vocal 1 min\\nSUGGESTED HIGH-PASS: ")+(eqSuggestionReady?juce::String(suggestedHP,0)+"Hz":"---")+"\\nSUGGESTED LOW-PASS: "+(eqSuggestionReady?juce::String(suggestedLP/1000.0f,1)+"kHz":"---")+"\\n"+(eqSuggestionReady?"Suggestion Only, You Set It Yourself":"Press LISTEN to show suggestion"),{listen.getX()+8,c.y+47,listen.getWidth()-16,listen.getHeight()*.48f},15);
        eqApplyBounds={listen.getX()+12,listen.getBottom()-45,listen.getWidth()-24,32};
        centredText(g,"APPLY SUGGESTION",eqApplyBounds,17,eqSuggestionReady?green:juce::Colours::grey);
        const juce::Point<float> hpCentre{filters.getX()+filters.getWidth()*.28f,filters.getY()+filters.getHeight()*.43f};
        const juce::Point<float> lpCentre{filters.getX()+filters.getWidth()*.72f,filters.getY()+filters.getHeight()*.43f};
        hpControlBounds={hpCentre.x-48,hpCentre.y-48,96,96}; lpControlBounds={lpCentre.x-48,lpCentre.y-48,96,96};
        drawKnob(g,hpCentre,36,"HIGH-PASS",juce::String(hpNow,0)+"Hz");drawKnob(g,lpCentre,36,"LOW-PASS",juce::String(lpNow/1000.0f,1)+"kHz");centredText(g,"BYPASS",{filters.getX()+20,filters.getBottom()-45,filters.getWidth()-40,30},17);
        eqGraph={analysis.getX()+14,analysis.getY()+55,analysis.getWidth()-28,analysis.getHeight()*.70f};g.setColour(blue);g.fillRoundedRectangle(eqGraph,8);g.setColour(brightBlue);g.drawRoundedRectangle(eqGraph,8,1.5f);
        for(int i=0;i<5;++i){float y=eqGraph.getY()+i*eqGraph.getHeight()/4;g.setColour(juce::Colour(0xff246aa2));g.drawLine(eqGraph.getX(),y,eqGraph.getRight(),y,1);centredText(g,juce::String(12-i*6),{eqGraph.getX()-32,y-10,28,20},11);}
        for(int i=0;i<5;++i){float x=eqGraph.getX()+i*eqGraph.getWidth()/4;g.setColour(juce::Colour(0xff246aa2));g.drawLine(x,eqGraph.getY(),x,eqGraph.getBottom(),1);}
        auto freqX=[&](float f){return eqGraph.getX()+eqGraph.getWidth()*(std::log10(f/20.0f)/3.0f);};
        auto gainY=[&](float v){return eqGraph.getCentreY()-v/24.0f*eqGraph.getHeight();};
        // Draw the combined four-band EQ response so the gold curve follows the draggable dots.
        juce::Path curve;
        for(int i=0;i<160;++i){
            const float norm=i/159.0f;
            const float f=20.0f*std::pow(1000.0f,norm);
            float response=0.0f;
            for(int band=0;band<4;++band){
                const float octaves=std::log2(f/eqFreq[band]);
                const float width=band==0||band==3?1.25f:0.85f;
                response+=eqGain[band]*std::exp(-0.5f*octaves*octaves/(width*width));
            }
            const float x=freqX(f), y=gainY(juce::jlimit(-12.0f,12.0f,response));
            if(i==0) curve.startNewSubPath(x,y); else curve.lineTo(x,y);
        }
        g.setColour(paleGold);g.strokePath(curve,juce::PathStrokeType(3));
        const juce::Colour bandC[]={cyan,green,juce::Colour(0xffff8b16),juce::Colour(0xffd34be8)};
        for(int i=0;i<4;++i){
            const float x=freqX(eqFreq[i]),y=gainY(eqGain[i]);
            g.setColour(bandC[i]);g.fillEllipse(x-8,y-8,16,16);
        }
        const char* freqLabels[]={"20Hz","100Hz","1kHz","10kHz","20kHz"};
        for(int i=0;i<5;++i){
            const float x=eqGraph.getX()+i*eqGraph.getWidth()/4.0f;
            centredText(g,freqLabels[i],{x-30,eqGraph.getBottom()-1,60,18},10,paleGold,false);
        }
        const char* bandN[]={"LOW","LOW-MID","MID","HIGH"};
        for(int i=0;i<4;++i)
            centredText(g,bandN[i],{analysis.getX()+analysis.getWidth()*i/4,eqGraph.getBottom()+17,analysis.getWidth()/4,23},13,bandC[i]);
        auto suggestY=row.getBottom()+10;drawBox(g,{content.getX(),suggestY,content.getWidth(),34},"");centredText(g,"SUGGESTED HIGH-PASS: 80Hz / SUGGESTED LOW-PASS: 16kHz",{content.getX()+5,suggestY,content.getWidth()-10,34},16);
        auto meterY=suggestY+43;drawBox(g,{content.getX(),meterY,content.getWidth(),content.getBottom()-meterY},"AJERRY LEARNING METER");drawMeter(g,{content.getX()+10,meterY+30,content.getWidth()-20,content.getBottom()-meterY-35},true);
    } else if(page==3){
        centredText(g,"AJERRY COMP PRO - SERIAL INSIDE - NORMAL GAIN",{content.getX(),content.getY(),content.getWidth(),30},21);
        auto top=content.withY(content.getY()+36).withHeight(content.getHeight()*.55f);float w=(top.getWidth()-2*gap)/3;auto a=top.withWidth(w),b=top.withX(a.getRight()+gap).withWidth(w),c=top.withX(b.getRight()+gap).withWidth(w);
        drawBox(g,a,"THRESHOLD");drawBox(g,b,"RATIO");drawBox(g,c,"GAIN NORMAL");
        auto thCentre=juce::Point<float>(a.getCentreX(),a.getY()+a.getHeight()*.45f), ratioCentre=juce::Point<float>(b.getCentreX(),b.getY()+b.getHeight()*.38f), gainCentre=juce::Point<float>(c.getCentreX(),c.getY()+c.getHeight()*.40f);
        compThresholdBounds={thCentre.x-50,thCentre.y-50,100,100};compRatioBounds={ratioCentre.x-50,ratioCentre.y-50,100,100};compGainBounds={gainCentre.x-50,gainCentre.y-50,100,100};
        drawKnob(g,thCentre,43,"THRESHOLD",juce::String(proc.parameters.getRawParameterValue("threshold")->load(),0));drawKnob(g,ratioCentre,43,"RATIO",juce::String(proc.parameters.getRawParameterValue("ratio")->load(),1));drawKnob(g,gainCentre,43,"GAIN NORMAL",juce::String(proc.parameters.getRawParameterValue("gain")->load(),1)+"dB");
        centredText(g,"LISTEN",{b.getX()+b.getWidth()*.34f,b.getY()-8,b.getWidth()*.32f,25},14,red);
        centredText(g,"Stage 1 GENTLE LA-2A  →  Stage 2 CATCHER 1176\nSERIAL INSIDE - ONE COMPRESSOR DOES BOTH",{b.getX()+8,b.getBottom()-68,b.getWidth()-16,58},14,green);
        auto sy=top.getBottom()+10;drawBox(g,{content.getX(),sy,content.getWidth(),35},"");centredText(g,"SUGGESTED THRESHOLD: -18 / SUGGESTED RATIO: 3",{content.getX(),sy,content.getWidth(),35},16);
        auto my=sy+44;drawBox(g,{content.getX(),my,content.getWidth(),content.getBottom()-my},"GAIN REDUCTION METER");drawMeter(g,{content.getX()+10,my+32,content.getWidth()-20,content.getBottom()-my-40},false);
    } else if(page==4 || page==5){
        auto top=content.withHeight(content.getHeight()*.66f);float modeW=top.getWidth()*.25f,amountW=top.getWidth()*.24f;
        auto mode=top.withWidth(modeW),amount=top.withX(mode.getRight()+gap).withWidth(amountW),analysis=top.withX(amount.getRight()+gap).withWidth(top.getRight()-amount.getRight()-gap);
        drawBox(g,mode,"MODE");drawBox(g,amount,"AMOUNT");drawBox(g,analysis,page==4?"ANALYSIS - HARSHNESS / ESSES":"ANALYSIS - DOUBLED VOICE / STEREO");
        const char* modes4[]={"Guitar","Vocals","Mix Bus"};const char* modes5[]={"Verse","Chorus","Rap","Trap","Hook"};int count=page==4?3:5;
        for(int i=0;i<count;++i){auto rr=juce::Rectangle<float>(mode.getX()+12,mode.getY()+55+i*(mode.getHeight()-65)/count,mode.getWidth()-24,(mode.getHeight()-72)/count);g.setColour(i==activeMode?juce::Colour(0xff064bff):juce::Colours::black);g.fillRoundedRectangle(rr,8);g.setColour(gold);g.drawRoundedRectangle(rr,8,1.5f);centredText(g,page==4?modes4[i]:modes5[i],rr,17,i==activeMode?juce::Colours::white:paleGold);}
        drawKnob(g,{amount.getCentreX(),amount.getY()+amount.getHeight()*.43f},48,"AMOUNT",page==4?"12 O'CLOCK - 50%":"MIX 60% 12 o'clock");
        drawWaveform(g,{analysis.getX()+12,analysis.getY()+55,analysis.getWidth()-24,analysis.getHeight()-70},0);
        auto my=top.getBottom()+12;drawBox(g,{content.getX(),my,content.getWidth(),content.getBottom()-my},"AJERRY LEARNING METER");drawMeter(g,{content.getX()+10,my+32,content.getWidth()-20,content.getBottom()-my-40},true);
    } else {
        float w=(content.getWidth()-2*gap)/3;auto a=content.withWidth(w),b=content.withX(a.getRight()+gap).withWidth(w),c=content.withX(b.getRight()+gap).withWidth(w);
        drawBox(g,a,page==6?"TIME / SPEED":"TIME / SIZE");drawBox(g,b,page==6?"DELAY / ECHO":"REVERB / ROOM");drawBox(g,c,"ANALYSIS");
        const juce::Point<float> timeCentre(a.getCentreX(),a.getCentreY()-15), mixCentre(b.getCentreX(),b.getCentreY()-15);
        if(page==6){delayTimeBounds={timeCentre.x-62,timeCentre.y-62,124,124};delayMixBounds={mixCentre.x-62,mixCentre.y-62,124,124};} if(page==7){reverbTimeBounds={timeCentre.x-62,timeCentre.y-62,124,124};reverbMixBounds={mixCentre.x-62,mixCentre.y-62,124,124};}
        drawKnob(g,timeCentre,55,page==6?"TIME / SPEED":"TIME / SIZE",page==6?juce::String(proc.parameters.getRawParameterValue("delayTime")->load(),0)+"ms - 12 o'clock":juce::String(proc.parameters.getRawParameterValue("reverbTime")->load(),0)+"ms ROOM");
        drawKnob(g,mixCentre,55,page==6?"DELAY / ECHO":"REVERB / ROOM",page==6?juce::String(proc.parameters.getRawParameterValue("delayMix")->load(),0)+" percent":juce::String(proc.parameters.getRawParameterValue("reverbMix")->load(),0)+"% CLEAR VOICE");
        drawWaveform(g,{c.getX()+18,c.getY()+70,c.getWidth()-36,c.getHeight()*.68f},page==6?1:2);
    }
}

void AjerryVST3AudioProcessorEditor::resized() {
    auto r=getLocalBounds().reduced(24);r.removeFromTop(76);int tw=r.getWidth()/8;
    for(int i=0;i<8;++i)tabs[i].setBounds(r.getX()+i*tw,r.getY(),tw-3,48);
}
void AjerryVST3AudioProcessorEditor::setParameter(const juce::String& id,float v) {
    if(auto* p=proc.parameters.getParameter(id))p->setValueNotifyingHost(p->convertTo0to1(v));
}
void AjerryVST3AudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    auto pos=e.position;
    if(page==3 && compThresholdBounds.contains(pos)){activeReverbKnob=0;draggingKnob=true;knobStart=pos;knobStartValue=proc.parameters.getRawParameterValue("threshold")->load();return;}
    if(page==3 && compRatioBounds.contains(pos)){activeReverbKnob=1;draggingKnob=true;knobStart=pos;knobStartValue=proc.parameters.getRawParameterValue("ratio")->load();return;}
    if(page==3 && compGainBounds.contains(pos)){activeReverbKnob=2;draggingKnob=true;knobStart=pos;knobStartValue=proc.parameters.getRawParameterValue("gain")->load();return;}
    if(page==6 && delayTimeBounds.contains(pos)){activeReverbKnob=0;draggingKnob=true;knobStart=pos;knobStartValue=proc.parameters.getRawParameterValue("delayTime")->load();return;}
    if(page==6 && delayMixBounds.contains(pos)){activeReverbKnob=1;draggingKnob=true;knobStart=pos;knobStartValue=proc.parameters.getRawParameterValue("delayMix")->load();return;}
    if(page==7 && reverbTimeBounds.contains(pos)){activeReverbKnob=0;draggingKnob=true;knobStart=pos;knobStartValue=proc.parameters.getRawParameterValue("reverbTime")->load();return;}
    if(page==7 && reverbMixBounds.contains(pos)){activeReverbKnob=1;draggingKnob=true;knobStart=pos;knobStartValue=proc.parameters.getRawParameterValue("reverbMix")->load();return;}
    if(page==2 && eqListenBounds.contains(pos)){ listening=true; eqSuggestionReady=true; suggestedHP=80.0f; suggestedLP=16000.0f; repaint(); return; }
    if(page==2 && eqApplyBounds.contains(pos) && eqSuggestionReady){ setParameter("hp",suggestedHP); setParameter("lp",suggestedLP); repaint(); return; }
    if(page==2 && hpControlBounds.contains(pos)){eqFilterDrag=0;knobStart=pos;knobStartValue=proc.parameters.getRawParameterValue("hp")->load();return;}
    if(page==2 && lpControlBounds.contains(pos)){eqFilterDrag=1;knobStart=pos;knobStartValue=proc.parameters.getRawParameterValue("lp")->load();return;}
    if(page==2 && eqGraph.contains(pos)){float best=1.0e9f;for(int i=0;i<4;++i){float x=eqGraph.getX()+eqGraph.getWidth()*(std::log10(eqFreq[i]/20.0f)/3.0f);float y=eqGraph.getCentreY()-eqGain[i]/24.0f*eqGraph.getHeight();float d=pos.getDistanceFrom({x,y});if(d<best){best=d;dragIndex=i;}}if(best<30){draggingEqDot=true;return;}}
    if(page==2) return;
    draggingKnob=true;knobStart=pos;knobStartValue=knobValues[0];
    if(page==4||page==5){float y=pos.y;activeMode=juce::jlimit(0,page==4?2:4,(int)((y-180)/100));repaint();}
    if(page==0||page==1||page==2){listening=!listening;repaint();}
}
void AjerryVST3AudioProcessorEditor::mouseDrag(const juce::MouseEvent& e) {
    if(draggingEqDot && dragIndex>=0){
        eqGain[dragIndex]=juce::jlimit(-12.0f,12.0f,(eqGraph.getCentreY()-e.position.y)/eqGraph.getHeight()*24.0f);
        float norm=juce::jlimit(0.0f,1.0f,(e.position.x-eqGraph.getX())/eqGraph.getWidth());
        eqFreq[dragIndex]=20.0f*std::pow(1000.0f,norm);
        const char* freqIds[]={"eqLowFreq","eqLowMidFreq","eqMidFreq","eqHighFreq"};
        const char* gainIds[]={"eqLowGain","eqLowMidGain","eqMidGain","eqHighGain"};
        setParameter(freqIds[dragIndex],eqFreq[dragIndex]);
        setParameter(gainIds[dragIndex],eqGain[dragIndex]);
        repaint();return;
    }
    if(draggingKnob){if((page==6||page==7)&&activeReverbKnob==0)setParameter(page==6?"delayTime":"reverbTime",juce::jlimit(page==6?1.0f:50.0f,page==6?1000.0f:2000.0f,knobStartValue+(knobStart.y-e.position.y)*10.0f));else if((page==6||page==7)&&activeReverbKnob==1)setParameter(page==6?"delayMix":"reverbMix",juce::jlimit(0.0f,100.0f,knobStartValue+(knobStart.y-e.position.y)*0.55f));else knobValues[0]=juce::jlimit(0.0f,1.0f,knobStartValue+(knobStart.y-e.position.y)/180.0f);repaint();}
}
void AjerryVST3AudioProcessorEditor::mouseUp(const juce::MouseEvent&) {eqFilterDrag=-1;activeReverbKnob=-1;draggingKnob=false;draggingEqDot=false;dragIndex=-1;}
