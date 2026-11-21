#include "PluginProcessor.h"
#include "PluginEditor.h"

AjerryVST3AudioProcessor::AjerryVST3AudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout AjerryVST3AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("gate", "Gate", -60.0f, 0.0f, -40.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("tune", "SpotOn Amount", 0.0f, 100.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("hp", "High Pass", 20.0f, 500.0f, 80.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("lp", "Low Pass", 2000.0f, 20000.0f, 16000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("threshold", "Comp Threshold", -60.0f, 0.0f, -18.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ratio", "Comp Ratio", 1.0f, 20.0f, 3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain", "Output Gain", -12.0f, 12.0f, 4.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("soften", "Soften", 0.0f, 100.0f, 50.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("doubler", "Doubler", 0.0f, 100.0f, 60.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("delayTime", "Delay Time", 1.0f, 1000.0f, 300.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("delayMix", "Delay Mix", 0.0f, 100.0f, 10.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("reverbTime", "Reverb Size", 50.0f, 2000.0f, 300.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("reverbMix", "Reverb Mix", 0.0f, 100.0f, 20.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
    return layout;
}

void AjerryVST3AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const auto channels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), channels };

    hpFilter.prepare(spec);
    lpFilter.prepare(spec);
    compressor.prepare(spec);
    delayLine.prepare(spec);
    doublerLine.prepare(spec);
    reverb.reset();

    delayLine.setMaximumDelayInSamples(juce::jmax(1, static_cast<int>(sampleRate) - 1));
    doublerLine.setMaximumDelayInSamples(juce::jmax(1, static_cast<int>(sampleRate * 0.05)));
    delayLine.reset();
    doublerLine.reset();
    hpFilter.reset();
    lpFilter.reset();
    compressor.reset();

    outputGainSmoothed.reset(sampleRate, 0.02);
    delayMixSmoothed.reset(sampleRate, 0.02);
    reverbMixSmoothed.reset(sampleRate, 0.05);
    softenSmoothed.reset(sampleRate, 0.02);
    doublerSmoothed.reset(sampleRate, 0.03);
    gateGainSmoothed.reset(sampleRate, 0.01);
    hpCutoffSmoothed.reset(sampleRate, 0.04);
    lpCutoffSmoothed.reset(sampleRate, 0.04);

    outputGainSmoothed.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(4.0f));
    delayMixSmoothed.setCurrentAndTargetValue(0.10f);
    reverbMixSmoothed.setCurrentAndTargetValue(0.20f);
    softenSmoothed.setCurrentAndTargetValue(0.50f);
    doublerSmoothed.setCurrentAndTargetValue(0.60f);
    gateGainSmoothed.setCurrentAndTargetValue(1.0f);
    hpCutoffSmoothed.setCurrentAndTargetValue(80.0f);
    lpCutoffSmoothed.setCurrentAndTargetValue(16000.0f);

    gateEnvelope = 0.0f;
    doublerPhase = 0.0f;
    gateIsOpen = false;

    hpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 80.0f);
    lpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 16000.0f);

    compressor.setThreshold(-18.0f);
    compressor.setRatio(3.0f);
    compressor.setAttack(10.0f);
    compressor.setRelease(100.0f);
}

void AjerryVST3AudioProcessor::releaseResources() {}

void AjerryVST3AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (parameters.getRawParameterValue("bypass")->load() > 0.5f)
        return;

    const float gateDb = parameters.getRawParameterValue("gate")->load();
    const float hpTarget = parameters.getRawParameterValue("hp")->load();
    const float lpTarget = parameters.getRawParameterValue("lp")->load();
    const float threshold = parameters.getRawParameterValue("threshold")->load();
    const float ratio = parameters.getRawParameterValue("ratio")->load();
    const float gainDb = parameters.getRawParameterValue("gain")->load();
    const float delayMs = parameters.getRawParameterValue("delayTime")->load();

    outputGainSmoothed.setTargetValue(juce::Decibels::decibelsToGain(gainDb));
    delayMixSmoothed.setTargetValue(parameters.getRawParameterValue("delayMix")->load() * 0.01f);
    reverbMixSmoothed.setTargetValue(parameters.getRawParameterValue("reverbMix")->load() * 0.01f);
    softenSmoothed.setTargetValue(parameters.getRawParameterValue("soften")->load() * 0.01f);
    doublerSmoothed.setTargetValue(parameters.getRawParameterValue("doubler")->load() * 0.01f);
    hpCutoffSmoothed.setTargetValue(hpTarget);
    lpCutoffSmoothed.setTargetValue(lpTarget);

    // Smooth cutoff changes at block boundaries to avoid zipper noise.
    const float hpNow = hpCutoffSmoothed.skip(numSamples);
    const float lpNow = lpCutoffSmoothed.skip(numSamples);
    hpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), hpNow);
    lpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), lpNow);

    compressor.setThreshold(threshold);
    compressor.setRatio(ratio);
    compressor.setAttack(10.0f);
    compressor.setRelease(100.0f);

    const int delaySamples = juce::jlimit(1, static_cast<int>(getSampleRate()) - 1,
                                          static_cast<int>(delayMs * getSampleRate() * 0.001));
    delayLine.setDelay(static_cast<float>(delaySamples));

    juce::dsp::Reverb::Parameters reverbParams;
    reverbParams.roomSize = juce::jlimit(0.05f, 1.0f,
        parameters.getRawParameterValue("reverbTime")->load() / 2000.0f);
    const float reverbMix = reverbMixSmoothed.skip(buffer.getNumSamples());
    reverbParams.wetLevel = reverbMix;
    reverbParams.dryLevel = 1.0f - reverbMix;
    reverbParams.width = 1.0f;
    reverbParams.damping = 0.45f;
    reverb.setParameters(reverbParams);

    juce::dsp::ProcessContextReplacing<float> context { juce::dsp::AudioBlock<float>(buffer) };
    hpFilter.process(context);
    lpFilter.process(context);
    compressor.process(context);

    const float gateThreshold = juce::Decibels::decibelsToGain(gateDb);
    const float gateOpenThreshold = gateThreshold;
    const float gateCloseThreshold = gateThreshold * 0.72f; // hysteresis prevents chatter.
    const float sampleRate = static_cast<float>(getSampleRate());

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float detector = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
            detector = juce::jmax(detector, std::abs(buffer.getSample(channel, sample)));

        const float envelopeCoefficient = detector > gateEnvelope
            ? std::exp(-1.0f / (0.008f * sampleRate))
            : std::exp(-1.0f / (0.090f * sampleRate));
        gateEnvelope = envelopeCoefficient * gateEnvelope + (1.0f - envelopeCoefficient) * detector;

        if (!gateIsOpen && gateEnvelope >= gateOpenThreshold)
            gateIsOpen = true;
        else if (gateIsOpen && gateEnvelope < gateCloseThreshold)
            gateIsOpen = false;

        gateGainSmoothed.setTargetValue(gateIsOpen ? 1.0f : 0.001f);
        const float gateGain = gateGainSmoothed.getNextValue();
        const float outputGain = outputGainSmoothed.getNextValue();
        const float delayWet = delayMixSmoothed.getNextValue();
        const float doublerAmount = doublerSmoothed.getNextValue();
        const float softenAmount = softenSmoothed.getNextValue();
        doublerPhase += 6.2831853f * 0.18f / sampleRate;
        if (doublerPhase >= 6.2831853f) doublerPhase -= 6.2831853f;

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float dry = buffer.getSample(channel, sample);
            const float echo = delayLine.popSample(channel);

            // Controlled feedback creates smooth, progressively quieter delay repeats.
            delayLine.pushSample(channel, dry + echo * 0.22f);

            // Independent, gently modulated channel delays create stereo thickening
            // without reading the same delay buffer twice or cross-feeding channels.
            const float channelOffset = (numChannels > 1 && channel == 1) ? 0.0022f : 0.0f;
            const float modulation = 0.00045f * std::sin(doublerPhase + channel * 1.5707963f);
            const float doublerDelaySeconds = juce::jlimit(0.012f, 0.045f,
                                                            0.020f + channelOffset + modulation);
            doublerLine.setDelay(doublerDelaySeconds * static_cast<float>(getSampleRate()));
            const float doubled = doublerLine.popSample(channel);
            doublerLine.pushSample(channel, dry);

            const float delayed = dry * (1.0f - delayWet) + echo * delayWet;
            const float wide = delayed * (1.0f - doublerAmount * 0.32f)
                             + doubled * (doublerAmount * 0.32f);
            const float drive = 1.0f + softenAmount * 0.65f;
            const float rounded = std::tanh(wide * drive) / drive;
            const float softened = wide + (rounded - wide) * (softenAmount * 0.35f);
            buffer.setSample(channel, sample, softened * gateGain * outputGain);
        }

    }

    // JUCE reverb runs in-place; its dry/wet balance is smoothed per block.
    reverb.process(context);
}

juce::AudioProcessorEditor* AjerryVST3AudioProcessor::createEditor()
{
    return new AjerryVST3AudioProcessorEditor(*this);
}

void AjerryVST3AudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destination);
}

void AjerryVST3AudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AjerryVST3AudioProcessor();
}
