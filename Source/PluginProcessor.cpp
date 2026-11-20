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
 hpFilter.prepare(s);lpFilter.prepare(s);compressor.prepare(s);delayLine.prepare(s);reverb.reset();
 delayLine.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));delayLine.reset();hpFilter.reset();lpFilter.reset();compressor.reset();
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,80.0f);lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,16000.0f);
 compressor.setThreshold(-18.0f);compressor.setRatio(3.0f);compressor.setAttack(10.0f);compressor.setRelease(100.0f);
}
void AjerryVST3AudioProcessor::releaseResources(){}
void AjerryVST3AudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals noDenormals;if(parameters.getRawParameterValue("bypass")->load()>0.5f)return;
 const float gate=parameters.getRawParameterValue("gate")->load(),hp=parameters.getRawParameterValue("hp")->load(),lp=parameters.getRawParameterValue("lp")->load();
 const float thr=parameters.getRawParameterValue("threshold")->load(),ratio=parameters.getRawParameterValue("ratio")->load(),gain=parameters.getRawParameterValue("gain")->load();
 const float ms=parameters.getRawParameterValue("delayTime")->load(),mix=parameters.getRawParameterValue("delayMix")->load()/100.0f,rev=parameters.getRawParameterValue("reverbMix")->load()/100.0f;
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(),hp);lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(),lp);
 compressor.setThreshold(thr);compressor.setRatio(ratio);delayLine.setDelay(juce::jlimit<int>(1,(int)getSampleRate()-1,(int)(ms*getSampleRate()/1000.0f)));
 juce::dsp::Reverb::Parameters reverbParams; reverbParams.roomSize=juce::jlimit(0.05f,1.0f,parameters.getRawParameterValue("reverbTime")->load()/2000.0f); reverbParams.wetLevel=rev; reverbParams.dryLevel=1.0f-rev; reverb.setParameters(reverbParams);
 for(int c=0;c<b.getNumChannels();++c){if(b.getRMSLevel(c,0,b.getNumSamples())<juce::Decibels::decibelsToGain(gate))b.applyGain(c,0,b.getNumSamples(),0.15f);}
 juce::dsp::ProcessContextReplacing<float> ctx{juce::dsp::AudioBlock<float>(b)};hpFilter.process(ctx);lpFilter.process(ctx);compressor.process(ctx);
 for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<b.getNumSamples();++i){float dry=b.getSample(c,i),echo=delayLine.popSample(c);delayLine.pushSample(c,dry);b.setSample(c,i,dry*(1.0f-mix)+echo*mix);}
 b.applyGain(juce::Decibels::decibelsToGain(gain)); reverb.process(ctx);
}
juce::AudioProcessorEditor* AjerryVST3AudioProcessor::createEditor(){return new AjerryVST3AudioProcessorEditor(*this);}
void AjerryVST3AudioProcessor::getStateInformation(juce::MemoryBlock& dest){auto state=parameters.copyState();std::unique_ptr<juce::XmlElement> xml(state.createXml());copyXmlToBinary(*xml,dest);}
void AjerryVST3AudioProcessor::setStateInformation(const void* data,int size){std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size));if(xml&&xml->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AjerryVST3AudioProcessor();}
