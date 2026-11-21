#pragma once
#include <JuceHeader.h>
class AjerryVST3AudioProcessor : public juce::AudioProcessor
{
public:
 AjerryVST3AudioProcessor();
 ~AjerryVST3AudioProcessor() override = default;
 void prepareToPlay(double,int) override;
 void releaseResources() override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName() const override {return "AJERRY CHANNEL STRIP - Compact";}
 bool acceptsMidi() const override{return false;} bool producesMidi() const override{return false;} bool isMidiEffect() const override{return false;}
 double getTailLengthSeconds() const override{return 2.5;}
 int getNumPrograms() override{return 1;} int getCurrentProgram() override{return 0;} void setCurrentProgram(int) override{}
 const juce::String getProgramName(int) override{return {};} void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState parameters;
 static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
 float meterGR=0.0f;
private:
 juce::dsp::IIR::Filter<float> hpFilter,lpFilter;
 juce::dsp::Compressor<float> compressor;
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> delayLine{96000};
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> doublerLine{96000};
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> pitchLine{96000};
 juce::dsp::Reverb reverb;
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> outputGainSmoothed,delayMixSmoothed,reverbMixSmoothed,softenSmoothed,doublerSmoothed,gateGainSmoothed,hpCutoffSmoothed,lpCutoffSmoothed;
 float gateEnvelope=0.0f;
 float doublerPhase=0.0f;
 std::array<float, 2048> pitchAnalysisBuffer{};
 int pitchAnalysisWrite=0, pitchAnalysisCount=0, pitchAnalysisCountdown=0;
 float detectedPitchHz=0.0f, pitchRatio=1.0f, pitchShiftPhase=0.0f;
 bool gateIsOpen=false;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AjerryVST3AudioProcessor)
};
