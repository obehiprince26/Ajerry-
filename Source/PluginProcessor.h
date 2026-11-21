#pragma once
#include <JuceHeader.h>
#include <array>
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
 double getTailLengthSeconds() const override{return 1.0;}
 int getNumPrograms() override{return 1;} int getCurrentProgram() override{return 0;} void setCurrentProgram(int) override{}
 const juce::String getProgramName(int) override{return {};} void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState parameters;
 static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
 float meterGR=0.0f;
 std::atomic<float> analysisRms{0.0f}, analysisLowRms{0.0f}, analysisHighRms{0.0f};\n std::atomic<float> spotPitchHz{0.0f}, spotConfidence{0.0f};\n std::atomic<int> spotMidiNote{-1};
private:
 juce::dsp::IIR::Filter<float> hpFilter,lpFilter,softenFilter;
 std::array<juce::dsp::IIR::Filter<float>,4> eqFilters;
 juce::dsp::Compressor<float> compressor; // Stage 1: gentle leveler
 juce::dsp::Compressor<float> peakCatcher; // Stage 2: fast peak control
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> delayLine{96000};
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> doublerLine{96000};
 juce::dsp::Reverb reverb;
 float gateEnvelope = 1.0f;
 int gateHoldSamples = 0;
 float analysisLowState = 0.0f, analysisHighState = 0.0f;
 double analysisSum = 0.0, analysisLowSum = 0.0, analysisHighSum = 0.0;
 uint64_t analysisSamples = 0;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AjerryVST3AudioProcessor)
};
