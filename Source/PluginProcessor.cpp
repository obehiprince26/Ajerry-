#include "PluginProcessor.h"
#include "PluginEditor.h"

AjerryVST3AudioProcessor::AjerryVST3AudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"PARAMETERS",createParameterLayout()){}
juce::AudioProcessorValueTreeState::ParameterLayout AjerryVST3AudioProcessor::createParameterLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 l.add(std::make_unique<juce::AudioParameterFloat>("gate","Gate",-60.0f,0.0f,-40.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("tune","SpotOn Amount",0.0f,100.0f,0.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("hp","High Pass",20.0f,500.0f,80.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("lp","Low Pass",2000.0f,20000.0f,16000.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("threshold","Comp Threshold",-60.0f,0.0f,-18.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("ratio","Comp Ratio",1.0f,20.0f,3.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("gain","Output Gain",-12.0f,12.0f,0.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("soften","Soften",0.0f,100.0f,0.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("doubler","Doubler",0.0f,100.0f,0.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("delayTime","Delay Time",1.0f,1000.0f,300.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("delayMix","Delay Mix",0.0f,100.0f,10.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("reverbTime","Reverb Size",50.0f,2000.0f,300.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("reverbMix","Reverb Mix",0.0f,100.0f,20.0f));
 l.add(std::make_unique<juce::AudioParameterBool>("bypass","Bypass",false)); return l;
}
void AjerryVST3AudioProcessor::prepareToPlay(double sr,int block){
 juce::dsp::ProcessSpec s{sr,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};
 hpFilter.prepare(s);lpFilter.prepare(s);compressor.prepare(s);delayLine.prepare(s);reverb.reset();
 delayLine.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));delayLine.reset();hpFilter.reset();lpFilter.reset();compressor.reset();
 softenFilter.prepare(s); softenFilter.reset(); doublerDelay.prepare(s); doublerDelay.setMaximumDelayInSamples(juce::jmax(1,(int)(sr*0.03))); doublerDelay.reset();
 gateGain=0.0f; hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,80.0f);lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,16000.0f);
 compressor.setThreshold(-18.0f);compressor.setRatio(3.0f);compressor.setAttack(10.0f);compressor.setRelease(100.0f);
}
void AjerryVST3AudioProcessor::releaseResources(){}
void AjerryVST3AudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals noDenormals; const int n=b.getNumSamples(),ch=b.getNumChannels(); if(n==0)return;
 if(parameters.getRawParameterValue("bypass")->load()>0.5f)return;
 auto get=[this](const char* id){return parameters.getRawParameterValue(id)->load();};
 const float gate=get("gate"),hp=get("hp"),lp=get("lp"),thr=get("threshold"),ratio=get("ratio"),gain=get("gain");
 const float soften=get("soften")/100.0f,dbl=get("doubler")/100.0f,delayMix=get("delayMix")/100.0f,rev=get("reverbMix")/100.0f;
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(),hp);lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(),lp);
 compressor.setThreshold(thr);compressor.setRatio(ratio); compressor.setAttack(10.0f);compressor.setRelease(100.0f);
 delayLine.setDelay(juce::jlimit<int>(1,(int)getSampleRate()-1,(int)(get("delayTime")*getSampleRate()/1000.0f)));
 doublerDelay.setDelay(juce::jlimit<int>(1,(int)(getSampleRate()*0.03),(int)(getSampleRate()*(0.012+0.012*dbl))));
 juce::dsp::Reverb::Parameters rp;rp.roomSize=juce::jlimit(0.05f,1.0f,get("reverbTime")/2000.0f);rp.wetLevel=rev;rp.dryLevel=1.0f-rev;reverb.setParameters(rp);
 // Stage 1: smoothed gate, avoiding abrupt block-level cuts.
 float rms=0.0f;for(int c=0;c<ch;++c)rms=juce::jmax(rms,b.getRMSLevel(c,0,n));
 const float target=rms<juce::Decibels::decibelsToGain(gate)?0.0f:1.0f;
 for(int i=0;i<n;++i){gateGain+=(target-gateGain)*(target>gateGain?0.015f:0.003f);for(int c=0;c<ch;++c)b.setSample(c,i,b.getSample(c,i)*gateGain);}
 // Stage 2: SpotOn amount is exposed and automated; a proper scale/key pitch shifter is still pending.
 // Stage 3: live HP/LP EQ, followed by compressor and soften.
 juce::dsp::ProcessContextReplacing<float> ctx{juce::dsp::AudioBlock<float>(b)};hpFilter.process(ctx);lpFilter.process(ctx);compressor.process(ctx);
 // Stage 5: adaptive high-frequency softening; blend original and smoothed signal.
 if(soften>0.001f){const float alpha=juce::jlimit(0.03f,0.65f,0.12f+soften*0.45f);for(int c=0;c<ch;++c){float low=softenState[(size_t)c];for(int i=0;i<n;++i){float x=b.getSample(c,i);low+=alpha*(x-low);b.setSample(c,i,x*(1.0f-soften*0.65f)+low*(soften*0.65f));}softenState[(size_t)c]=low;}}
 // Stage 6: short offset stereo double, mono-safe and mix controlled.
 for(int i=0;i<n;++i){float l=b.getSample(0,i),r=ch>1?b.getSample(1,i):l;float dl=doublerDelay.popSample(0),dr=ch>1?doublerDelay.popSample(1):dl;doublerDelay.pushSample(0,r);if(ch>1)doublerDelay.pushSample(1,l);if(dbl>0.001f){b.setSample(0,i,l*(1.0f-dbl*0.25f)+dr*dbl*0.25f);if(ch>1)b.setSample(1,i,r*(1.0f-dbl*0.25f)+dl*dbl*0.25f);}}
 // Stage 7: delay feedback is conservative to avoid runaway; stage 8 reverb follows.
 for(int c=0;c<ch;++c)for(int i=0;i<n;++i){float dry=b.getSample(c,i),echo=delayLine.popSample(c);delayLine.pushSample(c,dry+echo*0.25f);b.setSample(c,i,dry*(1.0f-delayMix)+echo*delayMix);}
 b.applyGain(juce::Decibels::decibelsToGain(gain));reverb.process(ctx);
}
juce::AudioProcessorEditor* AjerryVST3AudioProcessor::createEditor(){return new AjerryVST3AudioProcessorEditor(*this);}
void AjerryVST3AudioProcessor::getStateInformation(juce::MemoryBlock& dest){auto state=parameters.copyState();std::unique_ptr<juce::XmlElement> xml(state.createXml());copyXmlToBinary(*xml,dest);}
void AjerryVST3AudioProcessor::setStateInformation(const void* data,int size){std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size));if(xml&&xml->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AjerryVST3AudioProcessor();}
