#include "PluginProcessor.h"
#include "PluginEditor.h"
AjerryVST3AudioProcessor::AjerryVST3AudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"PARAMETERS",createParameterLayout()){}
juce::AudioProcessorValueTreeState::ParameterLayout AjerryVST3AudioProcessor::createParameterLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 // Stage 1 — Gate
 l.add(std::make_unique<juce::AudioParameterFloat>("gate","Gate Threshold",-80.0f,0.0f,-40.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("gateRange","Gate Range",-80.0f,0.0f,-24.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("gateAttack","Gate Attack",0.1f,100.0f,5.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("gateRelease","Gate Release",5.0f,1000.0f,120.0f));
 // Stage 2 — SPOTON; key is 0=C through 11=B. Scale default is major.
 l.add(std::make_unique<juce::AudioParameterFloat>("tune","SPOTON Amount",0.0f,100.0f,0.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("tuneSpeed","SPOTON Speed",1.0f,100.0f,35.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("tuneKey","SPOTON Key",0.0f,11.0f,0.0f));
 // Stage 3 — EQ
 l.add(std::make_unique<juce::AudioParameterFloat>("hp","High Pass",20.0f,500.0f,80.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("lp","Low Pass",2000.0f,20000.0f,16000.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("eqGain","EQ Gain",-12.0f,12.0f,0.0f));
 // Stage 4 — Compressor
 l.add(std::make_unique<juce::AudioParameterFloat>("threshold","Comp Threshold",-60.0f,0.0f,-18.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("ratio","Comp Ratio",1.0f,20.0f,3.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("gain","Output Gain",-12.0f,12.0f,4.0f));
 // Stage 5 — Soften
 l.add(std::make_unique<juce::AudioParameterFloat>("soften","Soften Amount",0.0f,100.0f,50.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("softenFreq","Soften Frequency",1500.0f,16000.0f,8500.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("softenMix","Soften Mix",0.0f,100.0f,35.0f));
 // Stage 6 — Doubler
 l.add(std::make_unique<juce::AudioParameterFloat>("doubler","Doubler Amount",0.0f,100.0f,60.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("doublerWidth","Doubler Width",0.0f,100.0f,60.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("doublerDelay","Doubler Delay",5.0f,40.0f,18.0f));
 // Stage 7 — Delay
 l.add(std::make_unique<juce::AudioParameterFloat>("delayTime","Delay Time",1.0f,1000.0f,300.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("delayMix","Delay Mix",0.0f,100.0f,10.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("delayFeedback","Delay Feedback",0.0f,85.0f,20.0f));
 // Stage 8 — Reverb
 l.add(std::make_unique<juce::AudioParameterFloat>("reverbTime","Reverb Size",50.0f,2000.0f,300.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("reverbMix","Reverb Mix",0.0f,100.0f,20.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("reverbWidth","Reverb Width",0.0f,100.0f,100.0f));
 l.add(std::make_unique<juce::AudioParameterBool>("bypass","Bypass",false));
 return l;
}
void AjerryVST3AudioProcessor::prepareToPlay(double sr,int block){
 juce::dsp::ProcessSpec spec{sr,(juce::uint32)juce::jmax(1,block),(juce::uint32)juce::jmax(1,getTotalNumOutputChannels())};
 hpFilter.prepare(spec);lpFilter.prepare(spec);softenFilter.prepare(spec);compressor.prepare(spec);delayLine.prepare(spec);doublerLine.prepare(spec);reverb.reset();
 delayLine.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));doublerLine.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1);
 delayLine.reset();doublerLine.reset();hpFilter.reset();lpFilter.reset();softenFilter.reset();compressor.reset();gateEnvelope[0]=gateEnvelope[1]=0.0f;
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,80.0f);
 lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,16000.0f);
 softenFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,8500.0f);
 compressor.setThreshold(-18.0f);compressor.setRatio(3.0f);compressor.setAttack(10.0f);compressor.setRelease(100.0f);
}
void AjerryVST3AudioProcessor::releaseResources(){}
void AjerryVST3AudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals noDenormals;
 const int channels=b.getNumChannels(),samples=b.getNumSamples();
 for(int c=getTotalNumInputChannels();c<channels;++c)b.clear(c,0,samples);
 if(parameters.getRawParameterValue("bypass")->load()>0.5f)return;
 const float gateDb=parameters.getRawParameterValue("gate")->load(),gateRangeDb=parameters.getRawParameterValue("gateRange")->load(),gateAttack=parameters.getRawParameterValue("gateAttack")->load(),gateRelease=parameters.getRawParameterValue("gateRelease")->load();
 const float hp=parameters.getRawParameterValue("hp")->load(),lp=parameters.getRawParameterValue("lp")->load(),eqGain=parameters.getRawParameterValue("eqGain")->load();
 const float thr=parameters.getRawParameterValue("threshold")->load(),ratio=parameters.getRawParameterValue("ratio")->load(),gain=parameters.getRawParameterValue("gain")->load();
 const float softenAmt=parameters.getRawParameterValue("soften")->load()/100.0f,softenFreq=parameters.getRawParameterValue("softenFreq")->load(),softenMix=parameters.getRawParameterValue("softenMix")->load()/100.0f;
 const float doublerAmt=parameters.getRawParameterValue("doubler")->load()/100.0f,doublerWidth=parameters.getRawParameterValue("doublerWidth")->load()/100.0f,doublerMs=parameters.getRawParameterValue("doublerDelay")->load();
 const float delayMs=parameters.getRawParameterValue("delayTime")->load(),delayMix=parameters.getRawParameterValue("delayMix")->load()/100.0f,feedback=parameters.getRawParameterValue("delayFeedback")->load()/100.0f;
 const float reverbMs=parameters.getRawParameterValue("reverbTime")->load(),reverbMix=parameters.getRawParameterValue("reverbMix")->load()/100.0f,reverbWidth=parameters.getRawParameterValue("reverbWidth")->load()/100.0f;
 const float sr=(float)getSampleRate();
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,juce::jlimit(20.0f,500.0f,hp));
 lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,juce::jlimit(2000.0f,20000.0f,lp));
 softenFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,juce::jlimit(1500.0f,16000.0f,softenFreq));
 compressor.setThreshold(thr);compressor.setRatio(juce::jlimit(1.0f,20.0f,ratio));
 delayLine.setDelay(juce::jlimit(1.0f,sr-1.0f,delayMs*sr/1000.0f));
 doublerLine.setDelay(juce::jlimit(1.0f,sr-1.0f,doublerMs*sr/1000.0f));
 juce::dsp::Reverb::Parameters rp;rp.roomSize=juce::jlimit(0.05f,1.0f,reverbMs/2000.0f);rp.wetLevel=reverbMix;rp.dryLevel=1.0f-reverbMix;rp.width=reverbWidth*100.0f;reverb.setParameters(rp);
 // Stage 1: sample-wise envelope gate with attack/release smoothing and adjustable attenuation.
 const float gateLin=juce::Decibels::decibelsToGain(gateDb),rangeLin=juce::Decibels::decibelsToGain(gateRangeDb);
 const float attackCoef=std::exp(-1.0f/(sr*juce::jmax(0.0001f,gateAttack/1000.0f))),releaseCoef=std::exp(-1.0f/(sr*juce::jmax(0.005f,gateRelease/1000.0f)));
 for(int c=0;c<channels;++c){float env=gateEnvelope[juce::jmin(c,1)];auto* data=b.getWritePointer(c);for(int i=0;i<samples;++i){float x=std::abs(data[i]);float coef=x>env?attackCoef:releaseCoef;env=coef*env+(1.0f-coef)*x;float g=env>=gateLin?1.0f:rangeLin;data[i]*=g;}gateEnvelope[juce::jmin(c,1)]=env;}
 // Stage 2: SPOTON analysis — autocorrelation estimates voiced pitch and chooses the nearest
 // major-scale target. Correction telemetry is calculated; the pitch-shifting renderer is a separate
 // unfinished component and is deliberately not falsely applied here.
 spotonDetectedHz=spotonTargetHz=spotonCorrectionCents=0.0f;
 const float tuneAmount=parameters.getRawParameterValue("tune")->load()/100.0f,tuneSpeed=parameters.getRawParameterValue("tuneSpeed")->load(),keyVal=parameters.getRawParameterValue("tuneKey")->load();
 if(channels>0&&samples>=128&&tuneAmount>0.001f){
  const int lagMin=juce::jmax(2,(int)(sr/500.0f)),lagMax=juce::jmin(samples/2,(int)(sr/80.0f));float best=-1.0f;int bestLag=0;
  for(int lag=lagMin;lag<=lagMax;++lag){double corr=0,energyA=0,energyB=0;for(int i=lag;i<samples;++i){float a=b.getSample(0,i),bb=b.getSample(0,i-lag);corr+=a*bb;energyA+=a*a;energyB+=bb*bb;}float norm=(float)(corr/std::sqrt(energyA*energyB+1.0e-12));if(norm>best){best=norm;bestLag=lag;}}
  if(bestLag>0&&best>0.35f){float hz=sr/(float)bestLag;spotonDetectedHz=hz;float midi=69.0f+12.0f*std::log2(hz/440.0f);int nearest=(int)std::round(midi);const int majorOffsets[7]={0,2,4,5,7,9,11};int key=(int)std::round(keyVal)%12;float targetMidi=(float)nearest;float bestDist=1000.0f;for(int n=nearest-12;n<=nearest+12;++n){int pc=((n-key)%12+12)%12;bool inScale=false;for(int j=0;j<7;++j)if(pc==majorOffsets[j])inScale=true;if(inScale){float d=std::abs(midi-n);if(d<bestDist){bestDist=d;targetMidi=(float)n;}}}spotonTargetHz=440.0f*std::pow(2.0f,(targetMidi-69.0f)/12.0f);spotonCorrectionCents=(targetMidi-midi)*100.0f*tuneAmount; (void)tuneSpeed;}
 }
 // Stage 3: high/low-pass filters plus output gain controlled by EQ Gain.
 juce::dsp::ProcessContextReplacing<float> context{juce::dsp::AudioBlock<float>(b)};hpFilter.process(context);lpFilter.process(context);b.applyGain(juce::Decibels::decibelsToGain(eqGain));
 // Stage 4: dynamics compression.
 compressor.process(context);
 // Stage 5: blend the low-passed signal toward the dry signal to tame harshness.
 if(softenAmt>0.0001f&&softenMix>0.0001f){for(int c=0;c<channels;++c){auto* data=b.getWritePointer(c);for(int i=0;i<samples;++i){float dry=data[i],filtered=softenFilter.processSample(c,dry);float blend=softenAmt*softenMix;data[i]=dry+(filtered-dry)*blend;}}}
 // Stage 6: short cross-channel delayed doubling; mono-safe and adjustable width.
 if(doublerAmt>0.0001f){for(int i=0;i<samples;++i){float inL=b.getSample(0,i),inR=channels>1?b.getSample(1,i):inL;float dl=doublerLine.popSample(0),dr=channels>1?doublerLine.popSample(1):dl;doublerLine.pushSample(0,inL);if(channels>1)doublerLine.pushSample(1,inR);float wet=doublerAmt*0.35f;float widen=doublerWidth;float dL=channels>1?dl*(1.0f-widen)+dr*widen:dl;float dR=channels>1?dr*(1.0f-widen)+dl*widen:dr;b.setSample(0,i,inL+dL*wet);if(channels>1)b.setSample(1,i,inR+dR*wet);}}
 // Stage 7: delay with feedback, bounded to prevent runaway.
 if(delayMix>0.0001f){for(int c=0;c<channels;++c){auto* data=b.getWritePointer(c);for(int i=0;i<samples;++i){float dry=data[i],echo=delayLine.popSample(c);delayLine.pushSample(c,dry+echo*juce::jlimit(0.0f,0.85f,feedback));data[i]=dry*(1.0f-delayMix)+echo*delayMix;}}}
 b.applyGain(juce::Decibels::decibelsToGain(gain));
 // Stage 8: reverb is last in the chain; wet/dry and room width follow their controls.
 reverb.process(context);
}
juce::AudioProcessorEditor* AjerryVST3AudioProcessor::createEditor(){return new AjerryVST3AudioProcessorEditor(*this);}
void AjerryVST3AudioProcessor::getStateInformation(juce::MemoryBlock& dest){auto state=parameters.copyState();std::unique_ptr<juce::XmlElement> xml(state.createXml());copyXmlToBinary(*xml,dest);}
void AjerryVST3AudioProcessor::setStateInformation(const void* data,int size){std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size));if(xml&&xml->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AjerryVST3AudioProcessor();}
