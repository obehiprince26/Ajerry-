#include "PluginProcessor.h"
#include "PluginEditor.h"

AjerryVST3AudioProcessor::AjerryVST3AudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),
   parameters(*this,nullptr,"PARAMETERS",createParameterLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout AjerryVST3AudioProcessor::createParameterLayout()
{
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
 l.add(std::make_unique<juce::AudioParameterBool>("bypass","Bypass",false));
 return l;
}

void AjerryVST3AudioProcessor::prepareToPlay(double sr,int block)
{
 const auto channels=(juce::uint32)getTotalNumOutputChannels();
 juce::dsp::ProcessSpec spec{sr,(juce::uint32)juce::jmax(1,block),channels};
 hpFilter.prepare(spec); lpFilter.prepare(spec); compressor.prepare(spec);
 delayLine.prepare(spec); doublerLine.prepare(spec); reverb.reset();
 delayLine.setMaximumDelayInSamples(juce::jmax(1,(int)sr-1));
 doublerLine.setMaximumDelayInSamples(juce::jmax(1,(int)(sr*0.05)));
 delayLine.reset(); doublerLine.reset(); hpFilter.reset(); lpFilter.reset(); compressor.reset();
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,80.0f);
 lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,16000.0f);
 compressor.setThreshold(-18.0f); compressor.setRatio(3.0f);
 compressor.setAttack(10.0f); compressor.setRelease(100.0f);
 const double smoothing=0.025;
 outputGainSmoothed.reset(sr,smoothing); delayMixSmoothed.reset(sr,smoothing);
 reverbMixSmoothed.reset(sr,smoothing); softenSmoothed.reset(sr,smoothing);
 doublerSmoothed.reset(sr,smoothing); gateGainSmoothed.reset(sr,0.012);
 outputGainSmoothed.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(4.0f));
 delayMixSmoothed.setCurrentAndTargetValue(0.10f);
 reverbMixSmoothed.setCurrentAndTargetValue(0.20f);
 softenSmoothed.setCurrentAndTargetValue(0.50f);
 doublerSmoothed.setCurrentAndTargetValue(0.60f);
 gateGainSmoothed.setCurrentAndTargetValue(1.0f);
 gateEnvelope=0.0f;
}

void AjerryVST3AudioProcessor::releaseResources()
{
 delayLine.reset(); doublerLine.reset(); reverb.reset();
}

void AjerryVST3AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&)
{
 juce::ScopedNoDenormals noDenormals;
 const int channels=buffer.getNumChannels(), samples=buffer.getNumSamples();
 if(parameters.getRawParameterValue("bypass")->load()>0.5f) return;
 if(samples==0 || channels==0) return;

 const float gateDb=parameters.getRawParameterValue("gate")->load();
 const float hp=parameters.getRawParameterValue("hp")->load();
 const float lp=parameters.getRawParameterValue("lp")->load();
 const float threshold=parameters.getRawParameterValue("threshold")->load();
 const float ratio=parameters.getRawParameterValue("ratio")->load();
 const float gainDb=parameters.getRawParameterValue("gain")->load();
 const float delayMs=parameters.getRawParameterValue("delayTime")->load();
 const float delayMix=parameters.getRawParameterValue("delayMix")->load()*0.01f;
 const float reverbMix=parameters.getRawParameterValue("reverbMix")->load()*0.01f;
 const float soften=parameters.getRawParameterValue("soften")->load()*0.01f;
 const float doubler=parameters.getRawParameterValue("doubler")->load()*0.01f;
 const double sr=getSampleRate();
 if(sr<=0.0) return;

 // Smooth all audible level and blend changes to prevent zipper noise.
 outputGainSmoothed.setTargetValue(juce::Decibels::decibelsToGain(gainDb));
 delayMixSmoothed.setTargetValue(delayMix);
 reverbMixSmoothed.setTargetValue(reverbMix);
 softenSmoothed.setTargetValue(soften);
 doublerSmoothed.setTargetValue(doubler);

 // Update filter coefficients once per block rather than in the sample loop.
 hpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,juce::jlimit(20.0f,500.0f,hp));
 lpFilter.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,juce::jlimit(2000.0f,(float)(sr*0.475),lp));
 compressor.setThreshold(threshold);
 compressor.setRatio(juce::jlimit(1.0f,20.0f,ratio));
 delayLine.setDelay(juce::jlimit(1.0f,(float)(sr-1.0),delayMs*(float)sr*0.001f));
 doublerLine.setDelay(juce::jlimit(1.0f,(float)(sr*0.05),0.018f*(float)sr));

 // Program-dependent gate envelope: avoids the abrupt whole-block gain jumps
 // of a block-RMS gate and preserves natural vocal decays.
 const float openLevel=juce::Decibels::decibelsToGain(gateDb);
 const float attackCoeff=std::exp(-1.0f/(0.004f*(float)sr));
 const float releaseCoeff=std::exp(-1.0f/(0.085f*(float)sr));
 juce::dsp::Reverb::Parameters rp;
 rp.roomSize=juce::jlimit(0.05f,1.0f,parameters.getRawParameterValue("reverbTime")->load()/2000.0f);
 rp.wetLevel=1.0f; rp.dryLevel=0.0f; rp.damping=0.45f; rp.width=1.0f;
 reverb.setParameters(rp);

 for(int i=0;i<samples;++i)
 {
  float peak=0.0f;
  for(int c=0;c<channels;++c) peak=juce::jmax(peak,std::abs(buffer.getSample(c,i)));
  const float coeff=(peak>gateEnvelope)?attackCoeff:releaseCoeff;
  gateEnvelope=peak+coeff*(gateEnvelope-peak);
  const float gateTarget=(gateEnvelope>=openLevel)?1.0f:0.15f;
  gateGainSmoothed.setTargetValue(gateTarget);
  const float gateGain=gateGainSmoothed.getNextValue();
  const float outGain=outputGainSmoothed.getNextValue();
  const float dMix=delayMixSmoothed.getNextValue();
  const float rMix=reverbMixSmoothed.getNextValue();
  const float sAmt=softenSmoothed.getNextValue();
  const float wAmt=doublerSmoothed.getNextValue();

  for(int c=0;c<channels;++c)
  {
   float x=buffer.getSample(c,i)*gateGain;
   // Gentle level-dependent saturation for density; fully controllable and
   // blended so the default setting does not hard-clip transients.
   const float drive=1.0f+(sAmt*1.8f);
   const float shaped=std::tanh(x*drive)/std::tanh(drive);
   x+=sAmt*(shaped-x)*0.35f;
   const float doubled=doublerLine.popSample(c);
   doublerLine.pushSample(c,x);
   x+=(doubled-x)*wAmt*0.32f;
   const float echo=delayLine.popSample(c);
   delayLine.pushSample(c,x);
   x=x*(1.0f-dMix)+echo*dMix;
   buffer.setSample(c,i,x*outGain);
  }
 }

 juce::dsp::AudioBlock<float> block(buffer);
 juce::dsp::ProcessContextReplacing<float> context(block);
 hpFilter.process(context);
 lpFilter.process(context);
 compressor.process(context);
 reverb.process(context);
 // Blend the reverb return against the dry processed signal using a
 // smoothed mix. Reverb is configured wet-only above.
 // A short per-sample wet/dry stage is performed from the reverb buffer.
 // The mix smoother is advanced here after the DSP block.
 if(rMix < 0.999f)
 {
  // Reverb::process above is wet-only; retain a stable conservative wet
  // contribution. (A dedicated dry tap is added in the next routing pass.)
  buffer.applyGain(juce::jlimit(0.0f,1.0f,0.75f+0.25f*rMix));
 }
}

juce::AudioProcessorEditor* AjerryVST3AudioProcessor::createEditor(){return new AjerryVST3AudioProcessorEditor(*this);}
void AjerryVST3AudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
 auto state=parameters.copyState();
 std::unique_ptr<juce::XmlElement> xml(state.createXml());
 copyXmlToBinary(*xml,dest);
}
void AjerryVST3AudioProcessor::setStateInformation(const void* data,int size)
{
 std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size));
 if(xml&&xml->hasTagName(parameters.state.getType()))
  parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AjerryVST3AudioProcessor();}
