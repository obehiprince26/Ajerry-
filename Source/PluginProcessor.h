#pragma once
#include <array>
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
 int getDetectedKeyRoot() const noexcept { return detectedKeyRoot; }
 bool isDetectedKeyMinor() const noexcept { return detectedKeyMinor; }
 float getDetectedKeyConfidence() const noexcept { return detectedKeyConfidence; }
 juce::String getDetectedKeyName() const;
private:
 juce::dsp::IIR::Filter<float> hpFilter,lpFilter;
 juce::dsp::Compressor<float> compressor;
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> delayLine{96000};
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> doublerLine{96000};
 // Third-order Lagrange interpolation reduces fractional-delay quantisation
 // in the dual-head vocal pitch shifter; the other effect delay lines stay linear.
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> pitchLine{96000};
 juce::dsp::Reverb reverb;
 // Beat-key analysis: 2048-point FFT chroma accumulation, independent of monophonic SPOTON tracking.
 juce::dsp::FFT keyFFT{11};
 std::array<float, 4096> keyFFTData{};
 std::array<float, 2048> keyAnalysisBuffer{};
 std::array<float, 12> keyChromaHistory{};
 int keyAnalysisWrite=0, keyAnalysisCount=0, keyAnalysisCountdown=0, keyAnalysisFrames=0;
 int detectedKeyRoot=-1, candidateKeyRoot=-1, candidateKeyFrames=0;
 bool candidateKeyMinor=false; detectedKeyRoot=-1;
 bool detectedKeyMinor=false;
 float detectedKeyConfidence=0.0f;
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> outputGainSmoothed,delayTimeSmoothed,delayMixSmoothed,reverbMixSmoothed,softenSmoothed,doublerSmoothed,gateGainSmoothed,hpCutoffSmoothed,lpCutoffSmoothed;
 float gateEnvelope=0.0f;
 float doublerPhase=0.0f;
 std::array<float, 2048> pitchAnalysisBuffer{};
 // Reused audio-thread scratch avoids repeated stack allocation during pitch analysis.
 std::array<float, 2048> pitchFrameScratch{};
 std::array<float, 2048> pitchCmndScratch{};
 int pitchAnalysisWrite=0, pitchAnalysisCount=0, pitchAnalysisCountdown=0;
 int pitchAnalysisDecimation=1, pitchDecimationCounter=0;
 float pitchAnalysisSampleRate=48000.0f;
 float detectedPitchHz=0.0f, pitchRatio=1.0f, pitchShiftPhase=0.0f;
 int pitchHoldSamples=0;
 int stableMidiNote=-1, candidateMidiNote=-1, candidateNoteFrames=0;
 std::array<float, 2> pitchChannelEnergySmoothed{};
 int pitchAnalysisChannel=0;
 juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pitchRatioSmoothed;
 juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pitchConfidenceSmoothed;
 float pitchConfidence=0.0f;
 bool gateIsOpen=false;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AjerryVST3AudioProcessor)
};
