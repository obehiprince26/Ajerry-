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
 l.add(std::make_unique<juce::AudioParameterFloat>("eqLowFreq","EQ Low Frequency",20.0f,20000.0f,120.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("eqLowGain","EQ Low Gain",-12.0f,12.0f,0.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("eqLowMidFreq","EQ Low-Mid Frequency",20.0f,20000.0f,400.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("eqLowMidGain","EQ Low-Mid Gain",-12.0f,12.0f,0.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("eqMidFreq","EQ Mid Frequency",20.0f,20000.0f,1000.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("eqMidGain","EQ Mid Gain",-12.0f,12.0f,0.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("eqHighFreq","EQ High Frequency",20.0f,20000.0f,5000.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("eqHighGain","EQ High Gain",-12.0f,12.0f,0.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("threshold","Comp Threshold",-60.0f,0.0f,-18.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("ratio","Comp Ratio",1.0f,20.0f,3.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("gain","Output Gain",-12.0f,12.0f,4.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("soften","Soften",0.0f,100.0f,50.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("doubler","Doubler",0.0f,100.0f,60.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("delayTime","Delay Time",1.0f,1000.0f,300.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("delayMix","Delay Mix",0.0f,100.0f,10.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("reverbTime","Reverb Size",50.0f,2000.0f,300.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("reverbMix","Reverb Mix",0.0f,100.0f,20.0f));
 l.add(std::make_unique<juce::AudioParameterBool>("bypass","Bypass",false)); return l;
}
void AjerryVST3AudioProcessor::prepareToPlay(double sr,int block){
 juce::dsp::ProcessSpec s{sr,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};
 hpFilter.prepare(s);lpFilter.prepare(s);softenFilter.prepare(s);for(auto& f:eqFilters)f.prepare(s);compressor.prepare(s);peakCatcher.prepare(s);delayLine.prepare(s);doublerLine.prepare(s);reverb.reset();
 delayLine.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));doublerLine.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));delayLine.reset();doublerLine.reset();hpFilter.reset();lpFilter.reset();softenFilter.reset();for(auto& f:eqFilters)f.reset();compressor.reset();peakCatcher.reset();
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,80.0f);lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,16000.0f);
 compressor.setThreshold(-24.0f);compressor.setRatio(2.0f);compressor.setAttack(25.0f);compressor.setRelease(180.0f);
 peakCatcher.setThreshold(-18.0f);peakCatcher.setRatio(3.0f);peakCatcher.setAttack(2.0f);peakCatcher.setRelease(60.0f);
}
void AjerryVST3AudioProcessor::releaseResources(){}
void AjerryVST3AudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals noDenormals;if(parameters.getRawParameterValue("bypass")->load()>0.5f)return;
 const float gate=parameters.getRawParameterValue("gate")->load(),hp=parameters.getRawParameterValue("hp")->load(),lp=parameters.getRawParameterValue("lp")->load();
 const float soften=parameters.getRawParameterValue("soften")->load()/100.0f,doubler=parameters.getRawParameterValue("doubler")->load()/100.0f;
 const float thr=parameters.getRawParameterValue("threshold")->load(),ratio=parameters.getRawParameterValue("ratio")->load(),gain=parameters.getRawParameterValue("gain")->load();
 const float ms=parameters.getRawParameterValue("delayTime")->load(),mix=parameters.getRawParameterValue("delayMix")->load()/100.0f,rev=parameters.getRawParameterValue("reverbMix")->load()/100.0f;
 const float outputGain=parameters.getRawParameterValue("gain")->load();
 const float reverbSizeMs=parameters.getRawParameterValue("reverbTime")->load();
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(),hp);lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(),lp);
 softenFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(),juce::jmap(soften,20000.0f,6500.0f));
 for(int band=0;band<4;++band){
  const char* freqIds[]={"eqLowFreq","eqLowMidFreq","eqMidFreq","eqHighFreq"};
  const char* gainIds[]={"eqLowGain","eqLowMidGain","eqMidGain","eqHighGain"};
  const float freq=parameters.getRawParameterValue(freqIds[band])->load();
  const float db=parameters.getRawParameterValue(gainIds[band])->load();
  const float safeFreq=juce::jlimit(20.0f,(float)(getSampleRate()*0.45),freq);
  eqFilters[band].coefficients=juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),safeFreq,0.85f,juce::Decibels::decibelsToGain(db));
 }
 compressor.setThreshold(juce::jlimit(-60.0f,0.0f,thr+6.0f));compressor.setRatio(juce::jlimit(1.0f,3.0f,ratio*0.67f));
 peakCatcher.setThreshold(thr);peakCatcher.setRatio(ratio);peakCatcher.setAttack(2.0f);peakCatcher.setRelease(60.0f);delayLine.setDelay(juce::jlimit<int>(1,(int)getSampleRate()-1,(int)(ms*getSampleRate()/1000.0f)));doublerLine.setDelay(juce::jlimit<int>(1,(int)getSampleRate()-1,(int)(0.015f*getSampleRate())));
 juce::dsp::Reverb::Parameters reverbParams; reverbParams.roomSize=juce::jmap(juce::jlimit(50.0f,2000.0f,reverbSizeMs),50.0f,2000.0f,0.08f,0.92f); reverbParams.damping=juce::jmap(reverbParams.roomSize,0.15f,0.75f); reverbParams.wetLevel=rev; reverbParams.dryLevel=1.0f-rev; reverbParams.width=1.0f; reverbParams.freezeMode=0.0f; reverb.setParameters(reverbParams);
 for(int c=0;c<b.getNumChannels();++c){if(b.getRMSLevel(c,0,b.getNumSamples())<juce::Decibels::decibelsToGain(gate))b.applyGain(c,0,b.getNumSamples(),0.0f);}
 juce::dsp::ProcessContextReplacing<float> ctx{juce::dsp::AudioBlock<float>(b)};hpFilter.process(ctx);lpFilter.process(ctx);for(auto& f:eqFilters)f.process(ctx);softenFilter.process(ctx);compressor.process(ctx);peakCatcher.process(ctx);
 for(int i=0;i<b.getNumSamples();++i){
  float left=b.getSample(0,i),right=b.getNumChannels()>1?b.getSample(1,i):left;
  float delayedL=doublerLine.popSample(0),delayedR=b.getNumChannels()>1?doublerLine.popSample(1):delayedL;
  doublerLine.pushSample(0,left);if(b.getNumChannels()>1)doublerLine.pushSample(1,right);
  b.setSample(0,i,left*(1.0f-doubler*0.30f)+delayedR*doubler*0.30f);if(b.getNumChannels()>1)b.setSample(1,i,right*(1.0f-doubler*0.30f)+delayedL*doubler*0.30f);
 }
 for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<b.getNumSamples();++i){float dry=b.getSample(c,i),echo=delayLine.popSample(c);delayLine.pushSample(c,dry);b.setSample(c,i,dry*(1.0f-mix)+echo*mix);}
 if(b.getNumChannels()>=2) reverb.processStereo(b.getWritePointer(0),b.getWritePointer(1),b.getNumSamples());
 else if(b.getNumChannels()==1) reverb.processMono(b.getWritePointer(0),b.getNumSamples());
 b.applyGain(juce::Decibels::decibelsToGain(outputGain));
}

void AjerryVST3AudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}


void AjerryVST3AudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
