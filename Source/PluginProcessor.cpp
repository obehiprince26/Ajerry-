#include "PluginProcessor.h"
#include "PluginEditor.h"
AjerryVST3AudioProcessor::AjerryVST3AudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"PARAMETERS",createParameterLayout()){}
juce::AudioProcessorValueTreeState::ParameterLayout AjerryVST3AudioProcessor::createParameterLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 l.add(std::make_unique<juce::AudioParameterFloat>("gate","Gate",-60.0f,0.0f,-40.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("gateRange","Gate Range",0.0f,60.0f,40.0f));
 l.add(std::make_unique<juce::AudioParameterBool>("gateBypass","Gate Bypass",false));
 l.add(std::make_unique<juce::AudioParameterFloat>("tune","SpotOn Amount",0.0f,100.0f,0.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("tuneSpeed","SpotOn Speed",0.0f,100.0f,50.0f));
l.add(std::make_unique<juce::AudioParameterInt>("spotKey","SpotOn Key",0,11,0));
l.add(std::make_unique<juce::AudioParameterBool>("spotKeyLock","SpotOn Key Lock",true));
 l.add(std::make_unique<juce::AudioParameterBool>("spotMinor","SpotOn Minor Scale",false));
l.add(std::make_unique<juce::AudioParameterBool>("spotBypass","SpotOn Bypass",false));
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
 l.add(std::make_unique<juce::AudioParameterInt>("softenMode","Soften Mode",0,2,1));
 l.add(std::make_unique<juce::AudioParameterFloat>("doubler","Doubler",0.0f,100.0f,60.0f));
 l.add(std::make_unique<juce::AudioParameterInt>("doublerMode","Doubler Mode",0,4,0));
 l.add(std::make_unique<juce::AudioParameterFloat>("delayTime","Delay Time",1.0f,1000.0f,300.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("delayMix","Delay Mix",0.0f,100.0f,10.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("reverbTime","Reverb Size",50.0f,2000.0f,300.0f));
 l.add(std::make_unique<juce::AudioParameterFloat>("reverbMix","Reverb Mix",0.0f,100.0f,20.0f));
 l.add(std::make_unique<juce::AudioParameterBool>("bypass","Bypass",false)); return l;
}
void AjerryVST3AudioProcessor::prepareToPlay(double sr,int block){
 juce::dsp::ProcessSpec s{sr,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};
 spotDelayBuffer.setSize(2,juce::jmax(512,(int)(sr*0.12)),false,false,true);spotDelayBuffer.clear();spotDelayWrite=0;spotPhaseA=0.0f;spotPhaseB=0.5f;spotRatioSmooth=1.0f;for(auto& h:spotKeyHistogram)h.store(0.0f);spotDetectedKey.store(0);hpFilter.prepare(s);lpFilter.prepare(s);softenFilter.prepare(s);for(auto& f:eqFilters)f.prepare(s);compressor.prepare(s);peakCatcher.prepare(s);delayLine.prepare(s);doublerLine.prepare(s);reverb.reset();
 delayLine.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));doublerLine.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));delayLine.reset();doublerLine.reset();hpFilter.reset();lpFilter.reset();softenFilter.reset();for(auto& f:eqFilters)f.reset();compressor.reset();peakCatcher.reset();
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,80.0f);lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,16000.0f);
 compressor.setThreshold(-24.0f);compressor.setRatio(2.0f);compressor.setAttack(25.0f);compressor.setRelease(180.0f);
 peakCatcher.setThreshold(-18.0f);peakCatcher.setRatio(3.0f);peakCatcher.setAttack(2.0f);peakCatcher.setRelease(60.0f);
}
void AjerryVST3AudioProcessor::releaseResources(){}
void AjerryVST3AudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals noDenormals;if(parameters.getRawParameterValue("bypass")->load()>0.5f)return;
 // SPOTON live pitch tracking (autocorrelation; voiced-note estimate).
 if (b.getNumSamples() >= 128) {
  const int n=b.getNumSamples(), minLag=juce::jmax(2,(int)(getSampleRate()/400.0)), maxLag=juce::jmin(n-2,(int)(getSampleRate()/65.0));
  float best=0.0f; int bestLag=0; double mean=0.0;
  for(int i=0;i<n;++i){float m=0;for(int ch=0;ch<b.getNumChannels();++ch)m+=b.getSample(ch,i);mean+=m/b.getNumChannels();}
  mean/=n;
  for(int lag=minLag;lag<=maxLag;lag++){double corr=0,energyA=0,energyB=0;for(int i=lag;i<n;++i){float a=0,bv=0;for(int ch=0;ch<b.getNumChannels();++ch){a+=b.getSample(ch,i);bv+=b.getSample(ch,i-lag);}a/=b.getNumChannels();bv/=b.getNumChannels();a-=mean;bv-=mean;corr+=a*bv;energyA+=a*a;energyB+=bv*bv;}float norm=(float)(corr/std::sqrt(juce::jmax(1.0e-12,energyA*energyB)));if(norm>best){best=norm;bestLag=lag;}}
  if(bestLag>0 && best>0.55f){float hz=(float)getSampleRate()/bestLag;spotPitchHz.store(hz);const float midi=69.0f+12.0f*std::log2(hz/440.0f);spotMidiFloat.store(midi);spotMidiNote.store((int)juce::roundToInt(midi));spotConfidence.store(best);
  const int pc=((int)juce::roundToInt(midi)%12+12)%12;for(int k=0;k<12;++k)spotKeyHistogram[k].store(spotKeyHistogram[k].load()*0.995f);spotKeyHistogram[pc].store(spotKeyHistogram[pc].load()+best);
  const int major[]={0,2,4,5,7,9,11},minor[]={0,2,3,5,7,8,10};float bestScore=-1.0f;int bestRoot=0;bool bestMinor=false;for(int root=0;root<12;++root){float maj=0,min=0;for(int i=0;i<7;++i){maj+=spotKeyHistogram[(root+major[i])%12].load();min+=spotKeyHistogram[(root+minor[i])%12].load();}if(maj>bestScore){bestScore=maj;bestRoot=root;bestMinor=false;}if(min>bestScore){bestScore=min;bestRoot=root;bestMinor=true;}}spotDetectedKey.store(bestRoot);spotDetectedMinor.store(bestMinor);}
 }
  // Lightweight input analysis for the EQ Listen/Suggestion workflow.
 // Metrics are measured before processing, then published for the editor.
 double blockRms=0.0, blockLow=0.0, blockHigh=0.0;
 const float lowAlpha=1.0f-std::exp(-2.0f*juce::MathConstants<float>::pi*120.0f/(float)getSampleRate());
 const float highAlpha=1.0f-std::exp(-2.0f*juce::MathConstants<float>::pi*8000.0f/(float)getSampleRate());
 for(int i=0;i<b.getNumSamples();++i){
  float mono=0.0f; for(int ch=0;ch<b.getNumChannels();++ch) mono+=b.getSample(ch,i);
  mono/=juce::jmax(1,b.getNumChannels());
  analysisLowState+=lowAlpha*(mono-analysisLowState);
  analysisHighState+=highAlpha*(mono-analysisHighState);
  float high=mono-analysisHighState;
  blockRms+=mono*mono; blockLow+=analysisLowState*analysisLowState; blockHigh+=high*high;
 }
 const double denom=juce::jmax(1,b.getNumSamples());
 analysisRms.store((float)std::sqrt(blockRms/denom));
 analysisLowRms.store((float)std::sqrt(blockLow/denom));
 analysisHighRms.store((float)std::sqrt(blockHigh/denom));
 const float gate=parameters.getRawParameterValue("gate")->load(),gateRange=parameters.getRawParameterValue("gateRange")->load(),hp=parameters.getRawParameterValue("hp")->load(),lp=parameters.getRawParameterValue("lp")->load();
 const int softenMode=(int)parameters.getRawParameterValue("softenMode")->load(),doublerMode=(int)parameters.getRawParameterValue("doublerMode")->load();
 const float soften=parameters.getRawParameterValue("soften")->load()/100.0f,doubler=parameters.getRawParameterValue("doubler")->load()/100.0f;
 const float thr=parameters.getRawParameterValue("threshold")->load(),ratio=parameters.getRawParameterValue("ratio")->load(),gain=parameters.getRawParameterValue("gain")->load();
 const float ms=parameters.getRawParameterValue("delayTime")->load(),mix=parameters.getRawParameterValue("delayMix")->load()/100.0f,rev=parameters.getRawParameterValue("reverbMix")->load()/100.0f;
 const float outputGain=parameters.getRawParameterValue("gain")->load();
 const float reverbSizeMs=parameters.getRawParameterValue("reverbTime")->load();
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(),hp);lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(),lp);
 const float softenCeiling=softenMode==0?12000.0f:(softenMode==1?7500.0f:10000.0f);
 softenFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(),juce::jmap(soften,20000.0f,softenCeiling));
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
 // SPOTON pitch correction: two cross-faded modulated delay taps (real audio DSP).
 const float spotAmount=parameters.getRawParameterValue("tune")->load()/100.0f;
 const float spotSpeed=parameters.getRawParameterValue("tuneSpeed")->load()/100.0f;
 const int keyRoot=(int)parameters.getRawParameterValue("spotKey")->load();
 const bool keyLock=parameters.getRawParameterValue("spotKeyLock")->load()>0.5f, spotBypass=parameters.getRawParameterValue("spotBypass")->load()>0.5f;
 float midiIn=spotMidiFloat.load();
 if(!spotBypass && spotAmount>0.001f && midiIn>0.0f && spotConfidence.load()>0.55f){
  float targetMidi=std::round(midiIn);
  if(keyLock){
   const int majorScale[]={0,2,4,5,7,9,11},minorScale[]={0,2,3,5,7,8,10};const int* scale=parameters.getRawParameterValue("spotMinor")->load()>0.5f?minorScale:majorScale;float bestDist=1000.0f;int chosen=(int)std::round(midiIn);
   for(int candidate=(int)std::floor(midiIn)-12;candidate<=(int)std::ceil(midiIn)+12;++candidate){int pc=(candidate-keyRoot+120)%12;bool allowed=false;for(int v:scale)if(pc==v)allowed=true;if(allowed&&std::abs(candidate-midiIn)<bestDist){bestDist=std::abs(candidate-midiIn);chosen=candidate;}}
   targetMidi=(float)chosen;
  }
  const float semitones=(targetMidi-midiIn)*spotAmount;
  const float targetRatio=std::pow(2.0f,semitones/12.0f);
  const float smoothingMs=juce::jmap(spotSpeed,180.0f,8.0f);
  const float smoothCoeff=std::exp(-1.0f/(smoothingMs*0.001f*(float)getSampleRate()));
  const int ringSize=spotDelayBuffer.getNumSamples(),window=juce::jmax(64,(int)(getSampleRate()*0.030f)),baseDelay=juce::jmax(32,(int)(getSampleRate()*0.020f));
  for(int i=0;i<b.getNumSamples();++i){
   spotRatioSmooth=targetRatio+(spotRatioSmooth-targetRatio)*smoothCoeff;
   for(int ch=0;ch<b.getNumChannels();++ch)spotDelayBuffer.setSample(ch,spotDelayWrite,b.getSample(ch,i));
   auto readTap=[&](float phase,int ch){float delay=baseDelay+phase*window;float read=spotDelayWrite-delay;while(read<0)read+=ringSize;int i0=(int)read%ringSize,i1=(i0+1)%ringSize;float frac=read-i0;return spotDelayBuffer.getSample(ch,i0)*(1.0f-frac)+spotDelayBuffer.getSample(ch,i1)*frac;};
   float pA=spotPhaseA,pB=spotPhaseB;
   float wA=0.5f-0.5f*std::cos(juce::MathConstants<float>::twoPi*pA),wB=0.5f-0.5f*std::cos(juce::MathConstants<float>::twoPi*pB),sum=wA+wB;
   for(int ch=0;ch<b.getNumChannels();++ch){float a=readTap(pA,ch),bb=readTap(pB,ch);b.setSample(ch,i,sum>1.0e-6f?(a*wA+bb*wB)/sum:0.0f);}
   const float phaseInc=(1.0f-spotRatioSmooth)/window;spotPhaseA+=phaseInc;spotPhaseB+=phaseInc;
   if(spotPhaseA>=1.0f)spotPhaseA-=1.0f;else if(spotPhaseA<0.0f)spotPhaseA+=1.0f;if(spotPhaseB>=1.0f)spotPhaseB-=1.0f;else if(spotPhaseB<0.0f)spotPhaseB+=1.0f;
   spotDelayWrite=(spotDelayWrite+1)%ringSize;
  }
 }
 const float gateThreshold=juce::Decibels::decibelsToGain(gate); const bool gateIsBypassed=parameters.getRawParameterValue("gateBypass")->load()>0.5f; const float gateFloor=juce::Decibels::decibelsToGain(-gateRange); const float attackCoeff=std::exp(-1.0f/(0.008f*getSampleRate())); const float releaseCoeff=std::exp(-1.0f/(0.120f*getSampleRate()));
 for(int i=0;i<b.getNumSamples();++i){float detector=0.0f;for(int ch=0;ch<b.getNumChannels();++ch)detector=juce::jmax(detector,std::abs(b.getSample(ch,i))); if(detector>gateThreshold)gateHoldSamples=(int)(0.025f*getSampleRate());else if(gateHoldSamples>0)--gateHoldSamples; if(gateIsBypassed) gateEnvelope=1.0f; else if(detector>gateThreshold || gateHoldSamples>0)gateEnvelope=1.0f+(gateEnvelope-1.0f)*attackCoeff;else gateEnvelope=gateFloor+(gateEnvelope-gateFloor)*releaseCoeff; for(int ch=0;ch<b.getNumChannels();++ch)b.setSample(ch,i,b.getSample(ch,i)*gateEnvelope);}
 juce::dsp::ProcessContextReplacing<float> ctx{juce::dsp::AudioBlock<float>(b)};hpFilter.process(ctx);lpFilter.process(ctx);for(auto& f:eqFilters)f.process(ctx);softenFilter.process(ctx);compressor.process(ctx);peakCatcher.process(ctx);
 for(int i=0;i<b.getNumSamples();++i){
  float left=b.getSample(0,i),right=b.getNumChannels()>1?b.getSample(1,i):left;
  float delayedL=doublerLine.popSample(0),delayedR=b.getNumChannels()>1?doublerLine.popSample(1):delayedL;
  doublerLine.pushSample(0,left);if(b.getNumChannels()>1)doublerLine.pushSample(1,right);
  const float modeScale[]={0.20f,0.28f,0.36f,0.42f,0.50f};const float width=doubler*modeScale[juce::jlimit(0,4,doublerMode)];
  b.setSample(0,i,left*(1.0f-width)+delayedR*width);if(b.getNumChannels()>1)b.setSample(1,i,right*(1.0f-width)+delayedL*width);
 }
 for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<b.getNumSamples();++i){float dry=b.getSample(c,i),echo=delayLine.popSample(c);delayLine.pushSample(c,dry+echo*0.22f);b.setSample(c,i,dry*(1.0f-mix)+echo*mix);}
 juce::dsp::ProcessContextReplacing<float> reverbContext{juce::dsp::AudioBlock<float>(b)};
 reverb.process(reverbContext);
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
