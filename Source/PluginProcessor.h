#pragma once
#include <JuceHeader.h>
#include <vector>
#include <array>
#include <atomic>
class AjerryVST3AudioProcessor : public juce::AudioProcessor
{
public:
 AjerryVST3AudioProcessor();
 ~AjerryVST3AudioProcessor() override = default;
 void prepareToPlay(double,int) override; void releaseResources() override; void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override{return true;}
 const juce::String getName() const override{return "AJERRY CHANNEL STRIP - Compact";}
 bool acceptsMidi() const override{return false;} bool producesMidi() const override{return false;} bool isMidiEffect() const override{return false;}
 double getTailLengthSeconds() const override{return 3.0;}
 int getNumPrograms() override{return 1;} int getCurrentProgram() override{return 0;} void setCurrentProgram(int){}
 const juce::String getProgramName(int) override{return {};} void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState parameters; static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(); std::atomic<float> meterGR{0.0f};
 std::atomic<bool> spotOnListening{false}; std::atomic<bool> spotOnResetRequested{false}; std::atomic<int> spotOnSuggestedKey{-1}; std::atomic<float> spotOnConfidence{0.0f}; std::atomic<bool> spotOnSuggestedMinor{false};
private:
 juce::dsp::IIR::Filter<float> hpFilter,lpFilter,softenFilter;juce::dsp::Compressor<float> compressor;
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> delayLine{96000},doubleLineL{96000},doubleLineR{96000};juce::dsp::Reverb reverb; juce::AudioBuffer<float> scratchBuffer;
 float gateGain=1.0f,softEnvelope=0.0f,doublerPhase=0.0f,compressorEnvelope=0.0f; std::array<float,12> pitchClassEnergy{}; int listenAnalysisBlocks=0;
 juce::dsp::FFT keyFFT{12}; juce::dsp::WindowingFunction<float> keyWindow{4096,juce::dsp::WindowingFunction<float>::hann,false}; std::array<float,8192> keyFftData{}; std::array<float,4096> keyRing{}; int keyRingWrite=0,keySamplesSinceFFT=0; void analyseKeyInput(const juce::AudioBuffer<float>&); void updateKeySuggestion();
 std::vector<float> pitchBufferL,pitchBufferR; int pitchWrite=0; float pitchPhase=0.0f,pitchRatio=1.0f,pitchMidi=0.0f; float detectPitchHz(const juce::AudioBuffer<float>&) const; void processSpotOn(juce::AudioBuffer<float>&,float);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AjerryVST3AudioProcessor)
};
