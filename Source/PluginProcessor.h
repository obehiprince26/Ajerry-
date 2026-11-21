#pragma once
#include <JuceHeader.h>
class AjerryVST3AudioProcessor : public juce::AudioProcessor
{
public:
 AjerryVST3AudioProcessor(); ~AjerryVST3AudioProcessor() override = default;
 void prepareToPlay(double,int) override; void releaseResources() override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override{return true;}
 const juce::String getName() const override{return "AJERRY CHANNEL STRIP - Compact";}
 bool acceptsMidi() const override{return false;} bool producesMidi() const override{return false;} bool isMidiEffect() const override{return false;}
 double getTailLengthSeconds() const override{return 2.0;} int getNumPrograms() override{return 1;} int getCurrentProgram() override{return 0;} void setCurrentProgram(int){}
 const juce::String getProgramName(int) override{return {};} void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState parameters; static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
 float meterGR=0.0f;
private:
 juce::dsp::IIR::Filter<float> hpFilter,lpFilter,softenFilter; juce::dsp::Compressor<float> compressor;
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> delayLine{96000},doublerDelay{4096};
 juce::dsp::Reverb reverb; float gateGain=0.0f; float softenState[2]={0.0f,0.0f};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AjerryVST3AudioProcessor)
};