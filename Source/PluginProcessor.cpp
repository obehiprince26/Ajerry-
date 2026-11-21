#include "PluginProcessor.h"
#include "PluginEditor.h"
AjerryVST3AudioProcessor::AjerryVST3AudioProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"PARAMETERS",createParameterLayout()){}
juce::AudioProcessorValueTreeState::ParameterLayout AjerryVST3AudioProcessor::createParameterLayout(){juce::AudioProcessorValueTreeState::ParameterLayout l;
l.add(std::make_unique<juce::AudioParameterFloat>("gate","Gate",-60.0f,0.0f,-40.0f));l.add(std::make_unique<juce::AudioParameterFloat>("tune","SpotOn Amount",0.0f,100.0f,0.0f));l.add(std::make_unique<juce::AudioParameterFloat>("speed","SpotOn Speed",1.0f,100.0f,45.0f));l.add(std::make_unique<juce::AudioParameterFloat>("key","SpotOn Key",0.0f,11.0f,0.0f));l.add(std::make_unique<juce::AudioParameterBool>("minor","SpotOn Minor Scale",false));l.add(std::make_unique<juce::AudioParameterFloat>("hp","High Pass",20.0f,500.0f,80.0f));l.add(std::make_unique<juce::AudioParameterFloat>("lp","Low Pass",2000.0f,20000.0f,16000.0f));l.add(std::make_unique<juce::AudioParameterFloat>("threshold","Comp Threshold",-60.0f,0.0f,-18.0f));l.add(std::make_unique<juce::AudioParameterFloat>("ratio","Comp Ratio",1.0f,20.0f,3.0f));l.add(std::make_unique<juce::AudioParameterFloat>("gain","Output Gain",-12.0f,12.0f,4.0f));l.add(std::make_unique<juce::AudioParameterFloat>("soften","Soften",0.0f,100.0f,50.0f));l.add(std::make_unique<juce::AudioParameterFloat>("doubler","Doubler",0.0f,100.0f,60.0f));l.add(std::make_unique<juce::AudioParameterFloat>("delayTime","Delay Time",1.0f,1000.0f,300.0f));l.add(std::make_unique<juce::AudioParameterFloat>("delayMix","Delay Mix",0.0f,100.0f,10.0f));l.add(std::make_unique<juce::AudioParameterFloat>("delayFeedback","Delay Feedback",0.0f,80.0f,20.0f));l.add(std::make_unique<juce::AudioParameterFloat>("reverbTime","Reverb Size",50.0f,2000.0f,300.0f));l.add(std::make_unique<juce::AudioParameterFloat>("reverbMix","Reverb Mix",0.0f,100.0f,20.0f));l.add(std::make_unique<juce::AudioParameterBool>("bypass","Bypass",false));return l;}
void AjerryVST3AudioProcessor::prepareToPlay(double sr,int block){juce::dsp::ProcessSpec s{sr,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};hpFilter.prepare(s);lpFilter.prepare(s);softenFilter.prepare(s);compressor.prepare(s);delayLine.prepare(s);doubleLineL.prepare(s);doubleLineR.prepare(s);reverb.reset();delayLine.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));doubleLineL.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));doubleLineR.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));delayLine.reset();doubleLineL.reset();doubleLineR.reset();hpFilter.reset();lpFilter.reset();softenFilter.reset();compressor.reset();gateGain=1.0f;softEnvelope=0.0f;pitchBufferL.assign((size_t)juce::jmax(4096,(int)(getSampleRate()*0.12)),0.0f);pitchBufferR.assign(pitchBufferL.size(),0.0f);pitchWrite=0;pitchPhase=0.0f;pitchRatio=1.0f;pitchMidi=0.0f; keyRing.fill(0.0f); keyFftData.fill(0.0f); keyRingWrite=0; keySamplesSinceFFT=0; pitchClassEnergy.fill(0.0f); listenAnalysisBlocks=0; spotOnSuggestedKey.store(-1); spotOnSuggestedMinor.store(false); spotOnConfidence.store(0.0f);hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,80.0f);lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,16000.0f);softenFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,10000.0f);compressor.setThreshold(-18.0f);compressor.setRatio(3.0f);compressor.setAttack(10.0f);compressor.setRelease(100.0f);}
void AjerryVST3AudioProcessor::releaseResources(){}

float AjerryVST3AudioProcessor::detectPitchHz(const juce::AudioBuffer<float>& b) const
{
 const int n=b.getNumSamples(); if(n<256) return 0.0f;
 const int minLag=juce::jmax(2,(int)(getSampleRate()/1000.0)), maxLag=juce::jmin(n/2,(int)(getSampleRate()/65.0));
 float best=0.0f; int bestLag=0;
 for(int lag=minLag;lag<=maxLag;++lag){double corr=0.0,aa=0.0,bb=0.0;for(int i=0;i<n-lag;++i){float x=0.0f,y=0.0f;for(int ch=0;ch<b.getNumChannels();++ch){x+=b.getSample(ch,i);y+=b.getSample(ch,i+lag);}x/=b.getNumChannels();y/=b.getNumChannels();corr+=(double)x*y;aa+=(double)x*x;bb+=(double)y*y;}float score=(aa>1.0e-10&&bb>1.0e-10)?(float)(corr/std::sqrt(aa*bb)):0.0f;if(score>best){best=score;bestLag=lag;}}
 return best>0.72f&&bestLag>0?(float)getSampleRate()/bestLag:0.0f;
}

void AjerryVST3AudioProcessor::updateKeySuggestion()
{
 const float majorProfile[12]={6.35f,2.23f,3.48f,2.33f,4.38f,4.09f,2.52f,5.19f,2.39f,3.66f,2.29f,2.88f};
 const float minorProfile[12]={6.33f,2.68f,3.52f,5.38f,2.60f,3.53f,2.54f,4.75f,3.98f,2.69f,3.34f,3.17f};
 float mean=0.0f; for(float v:pitchClassEnergy) mean+=v; mean/=12.0f;
 float norm=0.0f; for(float v:pitchClassEnergy) norm+=(v-mean)*(v-mean); norm=std::sqrt(norm);
 if(norm<1.0e-6f) return;
 float best=-2.0f,second=-2.0f; int bestRoot=0; bool bestMinor=false;
 for(int mode=0;mode<2;++mode) for(int root=0;root<12;++root) {
  const float* profile=mode?minorProfile:majorProfile; float pm=0.0f, dot=0.0f,pn=0.0f;
  for(int i=0;i<12;++i) pm+=profile[i]; pm/=12.0f;
  for(int i=0;i<12;++i) { float x=pitchClassEnergy[(size_t)((i+root)%12)]-mean; float y=profile[i]-pm; dot+=x*y; pn+=y*y; }
  float score=dot/(norm*std::sqrt(pn)+1.0e-9f);
  if(score>best){second=best;best=score;bestRoot=root;bestMinor=(mode==1);} else if(score>second) second=score;
 }
 spotOnSuggestedKey.store(bestRoot,std::memory_order_relaxed);
 spotOnSuggestedMinor.store(bestMinor,std::memory_order_relaxed);
 spotOnConfidence.store(juce::jlimit(0.0f,1.0f,(best-second)*2.5f),std::memory_order_relaxed);
}
void AjerryVST3AudioProcessor::analyseKeyInput(const juce::AudioBuffer<float>& b)
{
 if(spotOnResetRequested.exchange(false,std::memory_order_relaxed)) { pitchClassEnergy.fill(0.0f); listenAnalysisBlocks=0; keyRing.fill(0.0f); keyRingWrite=0; keySamplesSinceFFT=0; spotOnSuggestedKey.store(-1); spotOnConfidence.store(0.0f); }
 if(!spotOnListening.load(std::memory_order_relaxed)) return;
 for(int i=0;i<b.getNumSamples();++i) {
  float mono=0.0f; for(int ch=0;ch<b.getNumChannels();++ch) mono+=b.getSample(ch,i); mono/=juce::jmax(1,b.getNumChannels());
  keyRing[(size_t)keyRingWrite]=mono; keyRingWrite=(keyRingWrite+1)&4095; ++keySamplesSinceFFT;
  if(keySamplesSinceFFT>=1024) {
   keySamplesSinceFFT=0;
   for(int n=0;n<4096;++n) keyFftData[(size_t)n]=keyRing[(size_t)((keyRingWrite+n)&4095)];
   keyWindow.multiplyWithWindowingTable(keyFftData.data(),4096);
   keyFFT.performFrequencyOnlyForwardTransform(keyFftData.data());
   for(int bin=5;bin<2048;++bin) {
    float hz=(float)bin*(float)getSampleRate()/4096.0f;
    if(hz<55.0f||hz>2200.0f) continue;
    float magnitude=keyFftData[(size_t)bin]; if(magnitude<0.0001f) continue;
    int midi=(int)std::lround(69.0+12.0*std::log2((double)hz/440.0));
    int pc=(midi%12+12)%12; pitchClassEnergy[(size_t)pc]+=magnitude;
   }
   ++listenAnalysisBlocks; if(listenAnalysisBlocks>=8) updateKeySuggestion();
  }
 }
}

void AjerryVST3AudioProcessor::processSpotOn(juce::AudioBuffer<float>& b,float amount)
{
 if(amount<0.001f||pitchBufferL.empty()) return;
 float hz=detectPitchHz(b);
 if(hz>0.0f){float midi=69.0f+12.0f*std::log2(hz/440.0f);const int root=(int)parameters.getRawParameterValue("key")->load();const bool minor=parameters.getRawParameterValue("minor")->load()>0.5f; const float majorScale[]={0,2,4,5,7,9,11}; const float minorScale[]={0,2,3,5,7,8,10}; const float* scale=minor?minorScale:majorScale;float target=midi;float bestDistance=1000.0f;int centerOctave=(int)std::floor((midi-(60.0f+(float)root))/12.0f);for(int octave=centerOctave-1;octave<=centerOctave+1;++octave)for(int degreeIndex=0;degreeIndex<7;++degreeIndex){float degree=scale[degreeIndex];float note=60.0f+(float)root+12.0f*(float)octave+degree;float distance=std::abs(note-midi);if(distance<bestDistance){bestDistance=distance;target=note;}}float semitones=(target-midi)*(amount*0.01f);float desired=std::pow(2.0f,semitones/12.0f);float speed=parameters.getRawParameterValue("speed")->load();float smoothing=juce::jmap(speed,1.0f,100.0f,0.015f,0.45f);pitchRatio+=smoothing*(desired-pitchRatio);}
 const int size=(int)pitchBufferL.size(),depth=juce::jmax(64,size/3);const float base=(float)(size-depth-8);
 for(int i=0;i<b.getNumSamples();++i){float inL=b.getSample(0,i),inR=b.getNumChannels()>1?b.getSample(1,i):inL;pitchBufferL[(size_t)pitchWrite]=inL;pitchBufferR[(size_t)pitchWrite]=inR;
 float p1=pitchPhase,p2=pitchPhase+0.5f;if(p2>=1.0f)p2-=1.0f;
 auto read=[&](const std::vector<float>& data,float phase){float d=base+phase*(float)depth;float pos=(float)pitchWrite-d;while(pos<0)pos+=(float)size;int a=(int)pos,bx=(a+1)%size;float frac=pos-a;return data[(size_t)a]+frac*(data[(size_t)bx]-data[(size_t)a]);};
 float w1=0.5f-0.5f*std::cos(juce::MathConstants<float>::twoPi*p1),w2=1.0f-w1;
 float outL=read(pitchBufferL,p1)*w1+read(pitchBufferL,p2)*w2;float outR=read(pitchBufferR,p1)*w1+read(pitchBufferR,p2)*w2;
 b.setSample(0,i,outL);if(b.getNumChannels()>1)b.setSample(1,i,outR);
 pitchWrite=(pitchWrite+1)%size;pitchPhase+=((1.0f-pitchRatio)/(float)depth);while(pitchPhase<0.0f)pitchPhase+=1.0f;while(pitchPhase>=1.0f)pitchPhase-=1.0f;
 }
}

void AjerryVST3AudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){juce::ScopedNoDenormals noDenormals;const int n=b.getNumSamples(),channels=b.getNumChannels();if(n<=0)return; analyseKeyInput(b); if(parameters.getRawParameterValue("bypass")->load()>0.5f)return;
const float tune=parameters.getRawParameterValue("tune")->load();
const float gateDb=parameters.getRawParameterValue("gate")->load(),hp=parameters.getRawParameterValue("hp")->load(),lp=parameters.getRawParameterValue("lp")->load(),thr=parameters.getRawParameterValue("threshold")->load(),ratio=parameters.getRawParameterValue("ratio")->load(),gain=parameters.getRawParameterValue("gain")->load(),soften=parameters.getRawParameterValue("soften")->load()/100.0f,dbl=parameters.getRawParameterValue("doubler")->load()/100.0f,ms=parameters.getRawParameterValue("delayTime")->load(),mix=parameters.getRawParameterValue("delayMix")->load()/100.0f,feedback=parameters.getRawParameterValue("delayFeedback")->load()/100.0f,rev=parameters.getRawParameterValue("reverbMix")->load()/100.0f;
hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(),hp);lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(),lp);softenFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(),juce::jmap(soften,18000.0f,6500.0f));compressor.setThreshold(thr);compressor.setRatio(ratio);delayLine.setDelay(juce::jlimit<int>(1,(int)getSampleRate()-1,(int)(ms*getSampleRate()/1000.0f)));
juce::dsp::Reverb::Parameters rp;rp.roomSize=juce::jlimit(0.05f,1.0f,parameters.getRawParameterValue("reverbTime")->load()/2000.0f);rp.wetLevel=rev;rp.dryLevel=1.0f-rev;reverb.setParameters(rp);
const float gateTarget=juce::Decibels::decibelsToGain(gateDb);for(int i=0;i<n;++i){float peak=0;for(int c=0;c<channels;++c)peak=juce::jmax(peak,std::abs(b.getSample(c,i)));float target=peak<gateTarget?0.15f:1.0f;gateGain+=(target-gateGain)*(target<gateGain?0.08f:0.003f);for(int c=0;c<channels;++c)b.setSample(c,i,b.getSample(c,i)*gateGain);}
juce::dsp::ProcessContextReplacing<float> ctx{juce::dsp::AudioBlock<float>(b)};hpFilter.process(ctx);lpFilter.process(ctx);compressor.process(ctx);processSpotOn(b,tune);
if(soften>0.001f){juce::AudioBuffer<float> filtered(b);juce::dsp::AudioBlock<float> fblock(filtered);juce::dsp::ProcessContextReplacing<float> fctx(fblock);softenFilter.process(fctx);for(int c=0;c<channels;++c)for(int i=0;i<n;++i)b.setSample(c,i,juce::jmap(soften*0.35f,b.getSample(c,i),filtered.getSample(c,i)));}
// Independent fixed micro-delays create a real stereo doubling offset.
const int doubleDelayL=juce::jlimit(1,(int)getSampleRate()-1,(int)(0.0175*getSampleRate()));const int doubleDelayR=juce::jlimit(1,(int)getSampleRate()-1,(int)(0.0235*getSampleRate()));doubleLineL.setDelay((float)doubleDelayL);doubleLineR.setDelay((float)doubleDelayR);
for(int i=0;i<n;++i){float l=b.getSample(0,i),r=channels>1?b.getSample(1,i):l,dl=delayLine.popSample(0),dr=channels>1?delayLine.popSample(1):dl;delayLine.pushSample(0,l+dl*feedback);if(channels>1)delayLine.pushSample(1,r+dr*feedback);float outL=l*(1-mix)+dl*mix,outR=r*(1-mix)+dr*mix;float dL=doubleLineL.popSample(0),dR=doubleLineR.popSample(0);doubleLineL.pushSample(0,l);doubleLineR.pushSample(0,r);if(channels>1){b.setSample(0,i,outL+dR*dbl*0.22f);b.setSample(1,i,outR+dL*dbl*0.22f);}else b.setSample(0,i,outL+dL*dbl*0.15f);}
b.applyGain(juce::Decibels::decibelsToGain(gain));juce::dsp::AudioBlock<float> finalBlock(b);juce::dsp::ProcessContextReplacing<float> finalCtx(finalBlock);reverb.process(finalCtx);}
juce::AudioProcessorEditor* AjerryVST3AudioProcessor::createEditor(){return new AjerryVST3AudioProcessorEditor(*this);}
void AjerryVST3AudioProcessor::getStateInformation(juce::MemoryBlock& dest){auto state=parameters.copyState();std::unique_ptr<juce::XmlElement> xml(state.createXml());copyXmlToBinary(*xml,dest);}
void AjerryVST3AudioProcessor::setStateInformation(const void* data,int size){std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size));if(xml&&xml->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AjerryVST3AudioProcessor();}
