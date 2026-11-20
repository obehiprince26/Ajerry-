// AJERRY V7.1 - PART 1/4 - PASTE FIRST
#include <JuceHeader.h>

static void setParam(juce::RangedAudioParameter* p, float v)
{ if (p) p->setValueNotifyingHost(p->convertTo0to1(v)); }
static void setParamBool(juce::RangedAudioParameter* p, bool b)
{ if (p) p->setValueNotifyingHost(b ? 1.0f : 0.0f); }

// ---------- SPOTON = AUTO-TUNE ----------
struct SpotOn
{
    void prepare(double sr_) { sr = sr_; reset(); }
    void reset() { writePos = 0; readPos = 0; smoothRatio = 1.0f; for (auto &s : buf) s = 0; for (auto &h : pitchHist) h = 0; }
    void resetHist() { for (auto &h : pitchHist) h = 0; }
    float detectPitch()
    {
        int minLag = (int)(sr / 400.0); int maxLag = (int)(sr / 80.0);
        float bestCorr = 0; int bestLag = 0;
        for (int lag = minLag; lag < maxLag; ++lag)
        {
            float corr = 0, e1 = 0, e2 = 0;
            for (int i = 0; i < 1024; ++i)
            {
                float a = buf[(writePos - 2048 + i + 8192) % 8192];
                float b = buf[(writePos - 2048 + i + lag + 8192) % 8192];
                corr += a * b; e1 += a*a; e2 += b*b;
            }
            float norm = corr / (std::sqrt(e1*e2) + 1e-6f);
            if (norm > bestCorr) { bestCorr = norm; bestLag = lag; }
        }
        if (bestCorr < 0.80f) return 0; return (float)sr / (float)bestLag;
    }
    float nearestInKey(float freq, int key)
    {
        if (freq <= 0) return freq;
        float midi = 69.0f + 12.0f * std::log2(freq / 440.0f);
        int nearest = (int)std::round(midi);
        static const int scale[7] = {0,2,4,5,7,9,11};
        int best = nearest, bestDist = 100;
        for (int oct = -1; oct <= 1; ++oct)
            for (int s : scale)
            {
                int cand = key + s + 12 * ((nearest - key) / 12 + oct);
                int d = std::abs(cand - nearest);
                if (d < bestDist) { bestDist = d; best = cand; }
            }
        return 440.0f * std::pow(2.0f, (best - 69) / 12.0f);
    }
    int getAutoKey()
    {
        int bestKey = 0, bestScore = -1;
        for(int key=0; key<12; ++key){ int score = 0; static const int scale[7] = {0,2,4,5,7,9,11};
            for(int s: scale){ int note = (key + s) % 12; score += pitchHist[note]; }
            if(score > bestScore){ bestScore = score; bestKey = key; } }
        return bestKey;
    }
    void process(float* d, int n, float amount, float speed, int key)
    {
        if (amount <= 0.01f) return;
        for (int i = 0; i < n; ++i) { buf[writePos] = d[i]; writePos = (writePos + 1) % 8192; }
        float f0 = detectPitch();
        if (f0 > 0){ float midi = 69.0f + 12.0f * std::log2(f0 / 440.0f); int pc = ((int)std::round(midi) % 12 + 12) % 12; pitchHist[pc]++; }
        float target = 1.0f;
        if (f0 > 0) { target = nearestInKey(f0, key) / f0; target = juce::jlimit(0.89f, 1.12f, target); }
        float smooth = 0.01f + (speed / 100.0f) * 0.15f;
        smoothRatio += (target - smoothRatio) * smooth;
        float ratio = 1.0f + (smoothRatio - 1.0f) * (amount / 100.0f);
        if (std::abs(ratio - 1.0f) < 0.002f) return;
        if (readPos == 0) readPos = (float)(writePos - 1024 + 8192) % 8192;
        for (int i = 0; i < n; ++i)
        {
            int i0 = (int)readPos % 8192; int i1 = (i0 + 1) % 8192; float frac = readPos - (int)readPos;
            float shifted = buf[i0] + (buf[i1] - buf[i0]) * frac; float dry = d[i];
            d[i] = dry + (shifted - dry) * (amount / 100.0f); readPos += ratio;
            if (readPos >= 8192) readPos -= 8192;
            int dist = (writePos - (int)readPos + 8192) % 8192;
            if (dist < 100 || dist > 2048) readPos = (float)(writePos - 1024 + 8192) % 8192;
        }
    }
    double sr = 44100; float buf[8192] = {}; int writePos = 0; float readPos = 0; float smoothRatio = 1.0f; int pitchHist[12] = {};
};

// ---------- PROCESSOR ----------
class AjerryProcessor : public juce::AudioProcessor
{
public:
    AjerryProcessor() : AudioProcessor(BusesProperties().withInput("In", juce::AudioChannelSet::stereo(), true).withOutput("Out", juce::AudioChannelSet::stereo(), true))
    {
        addParameter(gateThresh = new juce::AudioParameterFloat("gateThresh","1 GATE THRESHOLD",-60,0,-30));
        addParameter(gateRange = new juce::AudioParameterFloat("gateRange","1 GATE RANGE",-60,0,-30));
        addParameter(tuneAmount = new juce::AudioParameterFloat("tuneAmount","2 SPOTON AMOUNT",0,100,50));
        addParameter(tuneSpeed = new juce::AudioParameterFloat("tuneSpeed","2 SPOTON SPEED",0,100,50));
        addParameter(lockedKey = new juce::AudioParameterInt("lockedKey","2 KEY LOCKED",0,11,0));
        addParameter(hp = new juce::AudioParameterFloat("hp","3 EQ HIGH-PASS",20,500,80));
        addParameter(eqLow = new juce::AudioParameterFloat("eqLow","3 EQ LOW",-12,12,0));
        addParameter(eqLowMid = new juce::AudioParameterFloat("eqLowMid","3 EQ LOW-MID",-12,12,0));
        addParameter(eqMid = new juce::AudioParameterFloat("eqMid","3 EQ MID",-12,12,0));
        addParameter(eqHigh = new juce::AudioParameterFloat("eqHigh","3 EQ HIGH",-12,12,0));
        addParameter(lp = new juce::AudioParameterFloat("lp","3 EQ LOW-PASS",5000,20000,18000));
        addParameter(compThresh = new juce::AudioParameterFloat("compThresh","4 COMP THRESHOLD",-40,0,-18));
        addParameter(compRatio = new juce::AudioParameterFloat("compRatio","4 COMP RATIO",1,20,3));
        addParameter(compGain = new juce::AudioParameterFloat("compGain","4 COMP GAIN NORMAL",0,12,4));
        addParameter(softenAmt = new juce::AudioParameterFloat("softenAmt","5 SOFTEN AMOUNT",0,100,50));
        addParameter(softenSens = new juce::AudioParameterFloat("softenSens","5 SOFTEN SENSITIVITY",0,100,50));
        addParameter(doubMix = new juce::AudioParameterFloat("doubMix","6 DOUBLER MIX",0,100,60));
        addParameter(doubWidth = new juce::AudioParameterFloat("doubWidth","6 DOUBLER WIDTH",0,100,60));
        addParameter(delayTime = new juce::AudioParameterFloat("delayTime","7 DELAY TIME",10,1000,300));
        addParameter(delayEcho = new juce::AudioParameterFloat("delayEcho","7 DELAY ECHO",0,100,10));
        addParameter(revSize = new juce::AudioParameterFloat("revSize","8 REVERB SIZE",50,2000,300));
        addParameter(revRoom = new juce::AudioParameterFloat("revRoom","8 REVERB ROOM",0,100,20));
        for(int i=0;i<8;i++) addParameter(bypass[i] = new juce::AudioParameterBool("bypass"+juce::String(i),"BYPASS "+juce::String(i+1),false));
    }
    void prepareToPlay(double sr, int s) override {
        int chans = juce::jmax(1, getTotalNumInputChannels()); if(chans>2) chans=2;
        juce::dsp::ProcessSpec spec{sr,(juce::uint32)s,(juce::uint32)chans};
        gate.prepare(spec); hpF.prepare(spec); lowF.prepare(spec); lowMidF.prepare(spec); midF.prepare(spec); highF.prepare(spec); lpF.prepare(spec);
        compA.prepare(spec); compB.prepare(spec);
        delayLine.setMaximumDelayInSamples(96000); delayLine.prepare(spec); verb.prepare(spec);
        spoton[0].prepare(sr); spoton[1].prepare(sr);
        setLatencySamples(1024);
        smoothedCompGain = juce::Decibels::decibelsToGain((float)*compGain);
        smoothedDelay = (float)*delayTime * (float)sr / 1000.0f;
        lastHp = lastLp = lastLow = lastLowMid = lastMid = lastHigh = -9999.0f;
        reset();
    }
    void reset() override {
        gate.reset(); hpF.reset(); lowF.reset(); lowMidF.reset(); midF.reset(); highF.reset(); lpF.reset();
        compA.reset(); compB.reset(); delayLine.reset(); verb.reset(); spoton[0].reset(); spoton[1].reset();
        for(int c=0;c<2;c++){ lastLowFilt[c]=0; doubPos[c]=0; for(int i=0;i<800;i++) doubBuffer[c][i]=0; }
    }
    void releaseResources() override { reset(); }
    int getAutoKey(){ return spoton[0].getAutoKey(); }
    void resetAutoKey(){ spoton[0].resetHist(); spoton[1].resetHist(); }
// AJERRY V7.1 - PART 2/4
    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override {
        juce::ScopedNoDenormals noDenormals;
        for(int i=getTotalNumInputChannels();i<getTotalNumOutputChannels();++i) buf.clear(i,0,buf.getNumSamples());
        juce::dsp::AudioBlock<float> block(buf); juce::dsp::ProcessContextReplacing<float> ctx(block); auto sr=getSampleRate();
        if(!*bypass[0]){
            float gateRatio = juce::jmap((float)*gateRange, -60.0f, 0.0f, 20.0f, 2.0f);
            gate.setThreshold(*gateThresh); gate.setRatio(gateRatio); gate.setAttack(2.0f); gate.setRelease(100.0f); gate.process(ctx);
        }
        if(!*bypass[1]){ for(int ch=0;ch<buf.getNumChannels();++ch){ auto* d=buf.getWritePointer(ch); int idx = ch < 2 ? ch : 0; spoton[idx].process(d, buf.getNumSamples(), (float)*tuneAmount, (float)*tuneSpeed, (int)*lockedKey); } }
        if(!*bypass[2]){
            if (*hp != lastHp || *lp != lastLp || *eqLow != lastLow || *eqLowMid != lastLowMid || *eqMid != lastMid || *eqHigh != lastHigh)
            {
                *hpF.state=*juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,*hp);
                *lowF.state=*juce::dsp::IIR::Coefficients<float>::makeLowShelf(sr,120.0f,0.7f,juce::Decibels::decibelsToGain((float)*eqLow));
                *lowMidF.state=*juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr,400.0f,1.0f,juce::Decibels::decibelsToGain((float)*eqLowMid));
                *midF.state=*juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr,1000.0f,1.0f,juce::Decibels::decibelsToGain((float)*eqMid));
                *highF.state=*juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr,5000.0f,0.7f,juce::Decibels::decibelsToGain((float)*eqHigh));
                *lpF.state=*juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,*lp);
                lastHp=*hp; lastLp=*lp; lastLow=*eqLow; lastLowMid=*eqLowMid; lastMid=*eqMid; lastHigh=*eqHigh;
            }
            hpF.process(ctx); lowF.process(ctx); lowMidF.process(ctx); midF.process(ctx); highF.process(ctx); lpF.process(ctx);
        }
        if(!*bypass[3]){
            float userThresh = (float)*compThresh;
            float userRatio = (float)*compRatio;
            float splitRatio = 1.0f + (userRatio - 1.0f) * 0.5f;
            float inRMS = 0; for(int ch=0; ch<buf.getNumChannels(); ++ch){ auto* d=buf.getReadPointer(ch); for(int n=0;n<buf.getNumSamples();++n) inRMS += d[n]*d[n]; }
            inRMS = std::sqrt(inRMS / juce::jmax(1, buf.getNumSamples() * buf.getNumChannels()) + 1e-12f);
            compA.setThreshold(userThresh); compA.setRatio(splitRatio); compA.setAttack(30); compA.setRelease(200); compA.process(ctx);
            compB.setThreshold(userThresh); compB.setRatio(splitRatio); compB.setAttack(1); compB.setRelease(50); compB.process(ctx);
            float outRMS = 0; for(int ch=0; ch<buf.getNumChannels(); ++ch){ auto* d=buf.getReadPointer(ch); for(int n=0;n<buf.getNumSamples();++n) outRMS += d[n]*d[n]; }
            outRMS = std::sqrt(outRMS / juce::jmax(1, buf.getNumSamples() * buf.getNumChannels()) + 1e-12f);
            float targetGR = juce::jlimit(0.0f, 18.0f, 20.0f * std::log10((inRMS + 1e-6f) / (outRMS + 1e-6f)));
            lastCompGR += (targetGR - lastCompGR) * 0.2f;
            float targetGain = juce::Decibels::decibelsToGain((float)*compGain);
            for(int ch=0; ch<buf.getNumChannels(); ++ch){ auto* d = buf.getWritePointer(ch); for(int n=0;n<buf.getNumSamples();++n){ smoothedCompGain += (targetGain - smoothedCompGain) * 0.001f; d[n] *= smoothedCompGain; } }
        }
        if(!*bypass[4]){
            float amt=*softenAmt/100.0f; float sens=0.05f+(*softenSens/100.0f)*0.2f;
            float highEnergy=0, totalEnergy=0;
            for(int ch=0;ch<buf.getNumChannels();++ch){ auto* d=buf.getWritePointer(ch); for(int n=0;n<buf.getNumSamples();++n){ float high=d[n]-lastLowFilt[ch]; lastLowFilt[ch]+=(d[n]-lastLowFilt[ch])*sens; highEnergy += high*high; totalEnergy += d[n]*d[n]; d[n]=d[n]-high*amt*0.6f; } }
            float harsh = totalEnergy > 1e-9f ? highEnergy / totalEnergy : 0;
            float targetLearn = juce::jlimit(0.0f, 18.0f, harsh * 60.0f);
            lastLearn += (targetLearn - lastLearn) * 0.15f;
        }
        if(!*bypass[5]){
            float mix=*doubMix/100.0f*0.5f; float width=*doubWidth/100.0f;
            for(int ch=0;ch<buf.getNumChannels();++ch){ auto* d=buf.getWritePointer(ch); for(int n=0;n<buf.getNumSamples();++n){ int offset=550+(int)(width*100.0f*(ch==0?-1:1)); int readPos=(doubPos[ch]+800-offset)%800; if(readPos<0) readPos+=800; float delayed=doubBuffer[ch][readPos]; doubBuffer[ch][doubPos[ch]]=d[n]; doubPos[ch]=(doubPos[ch]+1)%800; d[n]=d[n]*(1.0f-mix)+delayed*mix; } }
        }
        if(!*bypass[6]){
            float targetDelay = juce::jlimit(1.0f,95000.0f,*delayTime*(float)sr/1000.0f);
            float feedback=*delayEcho/100.0f;
            for(int ch=0;ch<buf.getNumChannels();++ch){ auto* d=buf.getWritePointer(ch); for(int n=0;n<buf.getNumSamples();++n){ smoothedDelay += (targetDelay - smoothedDelay) * 0.001f; float read=delayLine.popSample(ch,(int)smoothedDelay,true); float out=d[n]+read*feedback*0.4f; delayLine.pushSample(ch,out); d[n]=out; } }
        }
        if(!*bypass[7]){ juce::dsp::Reverb::Parameters p; p.roomSize=*revSize/2000.0f; p.damping=0.5f; p.width=1.0f; p.wetLevel=*revRoom/100.0f*0.5f; p.dryLevel=1.0f; verb.setParameters(p); verb.process(ctx); }
    }
    juce::AudioProcessorEditor* createEditor() override;
    const juce::String getName() const override { return "AJERRY Channel Strip - Compact"; }
    bool hasEditor() const override { return true; } bool acceptsMidi() const override { return false; } bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; } int getNumPrograms() override { return 1; } int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {} const juce::String getProgramName(int) override { return {}; } void changeProgramName(int,const juce::String&) override {}
    bool isBusesLayoutSupported(const BusesLayout& l) const override { auto in=l.getMainInputChannelSet(); auto out=l.getMainOutputChannelSet(); if(in!=out) return false; return in==juce::AudioChannelSet::mono()||in==juce::AudioChannelSet::stereo(); }
    void getStateInformation(juce::MemoryBlock& dest) override { juce::MemoryOutputStream stream(dest,true); for(int i=0;i<getNumParameters();++i) stream.writeFloat(getParameter(i)->getValue()); }
    void setStateInformation(const void* data,int size) override { if(size<(int)(sizeof(float)*getNumParameters())) return; juce::MemoryInputStream stream(data,(size_t)size,false); for(int i=0;i<getNumParameters();++i) getParameter(i)->setValueNotifyingHost(stream.readFloat()); }
    juce::AudioParameterFloat *gateThresh,*gateRange,*tuneAmount,*tuneSpeed,*hp,*eqLow,*eqLowMid,*eqMid,*eqHigh,*lp,*compThresh,*compRatio,*compGain,*softenAmt,*softenSens,*doubMix,*doubWidth,*delayTime,*delayEcho,*revSize,*revRoom;
    juce::AudioParameterInt *lockedKey; juce::AudioParameterBool *bypass[8];
    float lastCompGR = 0.0f;
    float lastLearn = 6.0f;
private:
    juce::dsp::NoiseGate<float> gate; juce::dsp::IIR::Filter<float> hpF,lowF,lowMidF,midF,highF,lpF; juce::dsp::Compressor<float> compA,compB; juce::dsp::DelayLine<float,96000> delayLine; juce::dsp::Reverb verb;
    SpotOn spoton[2]; float lastLowFilt[2]={0,0}; float doubBuffer[2][800]={}; int doubPos[2]={0,0};
    float smoothedCompGain = 1.0f; float smoothedDelay = 13230.0f;
    float lastHp=-9999, lastLp=-9999, lastLow=-9999, lastLowMid=-9999, lastMid=-9999, lastHigh=-9999;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AjerryProcessor)
};
