#pragma once
#include <JuceHeader.h>

// SPOTON - pitch fixer
struct SpotOn
{
    void prepare(double sr_) { sr = sr_; reset(); }
    void reset() { writePos = 0; readPos = 0; smoothRatio = 1.0f; for (auto &s : buf) s = 0; for (auto &h : pitchHist) h = 0; }
    void resetHist() { for (auto &h : pitchHist) h = 0; }
    float detectPitch();
    float nearestInKey(float freq, int key);
    int getAutoKey();
    void process(float* d, int n, float amount, float speed, int key);
    double sr = 44100; float buf[8192] = {}; int writePos = 0; float readPos = 0; float smoothRatio = 1.0f; int pitchHist[12] = {};
};

class AjerryVST3AudioProcessor : public juce::AudioProcessor
{
public:
    AjerryVST3AudioProcessor();
    ~AjerryVST3AudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void reset() override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "AJERRY Channel Strip"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // 1 GATE
    juce::AudioParameterFloat* gateThresh = nullptr;
    juce::AudioParameterFloat* gateRange = nullptr;
    // 2 SPOTON
    juce::AudioParameterFloat* tuneAmount = nullptr;
    juce::AudioParameterFloat* tuneSpeed = nullptr;
    juce::AudioParameterInt* lockedKey = nullptr;
    // 3 EQ
    juce::AudioParameterFloat* hp = nullptr;
    juce::AudioParameterFloat* eqLow = nullptr;
    juce::AudioParameterFloat* eqLowMid = nullptr;
    juce::AudioParameterFloat* eqMid = nullptr;
    juce::AudioParameterFloat* eqHigh = nullptr;
    juce::AudioParameterFloat* lp = nullptr;
    // 4 COMP
    juce::AudioParameterFloat* compThresh = nullptr;
    juce::AudioParameterFloat* compRatio = nullptr;
    juce::AudioParameterFloat* compGain = nullptr;
    // 5 SOFTEN
    juce::AudioParameterFloat* softenAmt = nullptr;
    juce::AudioParameterFloat* softenSens = nullptr;
    // 6 DOUBLER
    juce::AudioParameterFloat* doubMix = nullptr;
    juce::AudioParameterFloat* doubWidth = nullptr;
    // 7 DELAY
    juce::AudioParameterFloat* delayTime = nullptr;
    juce::AudioParameterFloat* delayEcho = nullptr;
    // 8 REVERB
    juce::AudioParameterFloat* revSize = nullptr;
    juce::AudioParameterFloat* revRoom = nullptr;

    juce::AudioParameterBool* bypass[8] = {};

    float lastCompGR = 0.0f;
    float lastLearn = 6.0f;

    int getAutoKey();
    void resetAutoKey();

private:
    juce::dsp::NoiseGate<float> gate;
    juce::dsp::IIR::Filter<float> hpF, lowF, lowMidF, midF, highF, lpF;
    juce::dsp::Compressor<float> compA, compB;
    juce::dsp::DelayLine<float,96000> delayLine;
    juce::dsp::Reverb verb;
    SpotOn spoton[2];
    float lastLowFilt[2] = {0,0};
    float doubBuffer[2][800] = {};
    int doubPos[2] = {0,0};
    float smoothedCompGain = 1.0f;
    float smoothedDelay = 13230.0f;
    float lastHp = -9999, lastLp = -9999, lastLow = -9999, lastLowMid = -9999, lastMid = -9999, lastHigh = -9999;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AjerryVST3AudioProcessor)
};
