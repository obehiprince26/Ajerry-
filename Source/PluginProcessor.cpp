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
    pitchLine.prepare(spec);
    reverb.reset();

    delayLine.setMaximumDelayInSamples(juce::jmax(1, static_cast<int>(sampleRate) - 1));
    doublerLine.setMaximumDelayInSamples(juce::jmax(1, static_cast<int>(sampleRate * 0.05)));
    pitchLine.setMaximumDelayInSamples(juce::jmax(2, static_cast<int>(sampleRate * 0.065)));
    delayLine.reset();
    doublerLine.reset();
    pitchLine.reset();
    hpFilter.reset();
    lpFilter.reset();
    compressor.reset();

    pitchRatioSmoothed.reset(sampleRate, 0.035);
    pitchRatioSmoothed.setCurrentAndTargetValue(1.0f);
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
    pitchAnalysisBuffer.fill(0.0f);
    pitchAnalysisWrite = pitchAnalysisCount = pitchAnalysisCountdown = 0;
    detectedPitchHz = 0.0f;
    pitchRatio = 1.0f;
    pitchShiftPhase = 0.0f;
    pitchHoldSamples = 0;
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
    const float spotOnAmount = parameters.getRawParameterValue("tune")->load() * 0.01f;

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
        float monoForPitch = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float inputSample = buffer.getSample(channel, sample);
            detector = juce::jmax(detector, std::abs(inputSample));
            monoForPitch += inputSample;
        }
        monoForPitch /= static_cast<float>(juce::jmax(1, numChannels));

        // SPOTON: rolling monophonic vocal analysis using a YIN-style difference function.
        pitchAnalysisBuffer[static_cast<size_t>(pitchAnalysisWrite)] = monoForPitch;
        pitchAnalysisWrite = (pitchAnalysisWrite + 1) % static_cast<int>(pitchAnalysisBuffer.size());
        pitchAnalysisCount = juce::jmin(pitchAnalysisCount + 1, static_cast<int>(pitchAnalysisBuffer.size()));
        if (++pitchAnalysisCountdown >= 1024 && pitchAnalysisCount >= 1024)
        {
            pitchAnalysisCountdown = 0;
            constexpr int windowSize = 1024;
            const int minLag = juce::jlimit(20, 80, static_cast<int>(sampleRate / 1500.0f));
            const int maxLag = juce::jlimit(minLag + 2, windowSize - 2,
                                            static_cast<int>(sampleRate / 75.0f));
            std::array<float, windowSize> frame{};
            float frameEnergy = 0.0f;
            for (int n = 0; n < windowSize; ++n)
            {
                const int index = (pitchAnalysisWrite - windowSize + n + static_cast<int>(pitchAnalysisBuffer.size()))
                                  % static_cast<int>(pitchAnalysisBuffer.size());
                const float value = pitchAnalysisBuffer[static_cast<size_t>(index)];
                frame[static_cast<size_t>(n)] = value;
                frameEnergy += value * value;
            }
            const float frameRms = std::sqrt(frameEnergy / static_cast<float>(windowSize));
            std::array<float, windowSize> cmnd{};
            float cumulativeDifference = 0.0f;
            int bestLag = 0;
            float bestValue = 1.0f;

            // YIN cumulative-mean-normalised difference: accumulate from lag 1,
            // not minLag, so the normalisation remains mathematically correct.
            for (int lag = 1; lag <= maxLag; ++lag)
            {
                float difference = 0.0f;
                for (int n = 0; n < windowSize - lag; ++n)
                {
                    const float delta = frame[static_cast<size_t>(n)] - frame[static_cast<size_t>(n + lag)];
                    difference += delta * delta;
                }
                cumulativeDifference += difference;
                if (lag >= minLag)
                {
                    const float normalized = cumulativeDifference > 1.0e-12f
                        ? difference * static_cast<float>(lag) / cumulativeDifference : 1.0f;
                    cmnd[static_cast<size_t>(lag)] = normalized;
                    if (normalized < bestValue)
                    {
                        bestValue = normalized;
                        bestLag = lag;
                    }
                }
            }

            // Prefer the first convincing YIN trough to avoid octave errors;
            // fall back to the global minimum only when it is still credible.
            if (frameRms >= 0.008f)
            {
                for (int lag = minLag + 1; lag < maxLag; ++lag)
                {
                    const float value = cmnd[static_cast<size_t>(lag)];
                    if (value < 0.15f
                        && value <= cmnd[static_cast<size_t>(lag - 1)]
                        && value <= cmnd[static_cast<size_t>(lag + 1)])
                    {
                        bestLag = lag;
                        bestValue = value;
                        break;
                    }
                }
            }
            else
            {
                bestLag = 0;
            }

            if (bestLag > 0 && bestValue < 0.25f && frameRms >= 0.008f)
            {
                const float left = cmnd[static_cast<size_t>(juce::jmax(minLag, bestLag - 1))];
                const float centre = cmnd[static_cast<size_t>(bestLag)];
                const float right = cmnd[static_cast<size_t>(juce::jmin(maxLag, bestLag + 1))];
                const float denominator = left - 2.0f * centre + right;
                const float offset = std::abs(denominator) > 1.0e-9f
                    ? 0.5f * (left - right) / denominator : 0.0f;
                const float refinedLag = static_cast<float>(bestLag) + juce::jlimit(-0.5f, 0.5f, offset);
                detectedPitchHz = sampleRate / refinedLag;
                const float midi = 69.0f + 12.0f * std::log2(detectedPitchHz / 440.0f);
                const float semitones = (std::round(midi) - midi) * spotOnAmount;
                pitchRatio = juce::jlimit(0.80f, 1.25f, std::pow(2.0f, semitones / 12.0f));
            }
                pitchHoldSamples = static_cast<int>(sampleRate * 0.12f);
            }
            else if (pitchHoldSamples > 0)
            {
                // Brief consonants and breath gaps should not pull the correction
                // back to unison between voiced frames.
                pitchHoldSamples = juce::jmax(0, pitchHoldSamples - windowSize);
            }
            else
            {
                detectedPitchHz = 0.0f;
                pitchRatio = 1.0f;
            }
            pitchRatioSmoothed.setTargetValue(pitchRatio);
        }

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
        const float correctionRatio = pitchRatioSmoothed.getNextValue();
        const float pitchRange = juce::jmax(2.0f, sampleRate * 0.030f);
        const float pitchMinDelay = sampleRate * 0.018f;
        const float phaseA = pitchShiftPhase;
        const float phaseB = std::fmod(pitchShiftPhase + 0.5f, 1.0f);
        const auto delayAtPhase = [correctionRatio, pitchRange, pitchMinDelay](float phase)
        {
            const float ramp = correctionRatio >= 1.0f ? 1.0f - phase : phase;
            return pitchMinDelay + ramp * pitchRange;
        };
        const float weightA = 0.5f - 0.5f * std::cos(6.2831853f * phaseA);
        const float weightB = 1.0f - weightA;
        pitchShiftPhase += std::abs(1.0f - correctionRatio) / pitchRange;
        if (pitchShiftPhase >= 1.0f) pitchShiftPhase -= std::floor(pitchShiftPhase);
        const float delayWet = delayMixSmoothed.getNextValue();
        const float doublerAmount = doublerSmoothed.getNextValue();
        const float softenAmount = softenSmoothed.getNextValue();
        doublerPhase += 6.2831853f * 0.18f / sampleRate;
        if (doublerPhase >= 6.2831853f) doublerPhase -= 6.2831853f;

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float dry = buffer.getSample(channel, sample);
            const float pitchA = pitchLine.popSample(channel, delayAtPhase(phaseA), false);
            const float pitchB = pitchLine.popSample(channel, delayAtPhase(phaseB), false);
            pitchLine.pushSample(channel, dry);
            const float shifted = pitchA * weightA + pitchB * weightB;
            const float pitchWet = (detectedPitchHz > 0.0f && spotOnAmount > 0.001f && pitchHoldSamples > 0) ? 1.0f : 0.0f;
            const float spotOnSignal = dry * (1.0f - pitchWet) + shifted * pitchWet;
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

            const float delayed = spotOnSignal * (1.0f - delayWet) + echo * delayWet;
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
