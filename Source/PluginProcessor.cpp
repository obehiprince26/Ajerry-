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
    pitchConfidenceSmoothed.reset(sampleRate, 0.018);
    pitchConfidenceSmoothed.setCurrentAndTargetValue(0.0f);
    outputGainSmoothed.reset(sampleRate, 0.02);
    delayTimeSmoothed.reset(sampleRate, 0.05);
    delayMixSmoothed.reset(sampleRate, 0.02);
    reverbMixSmoothed.reset(sampleRate, 0.05);
    softenSmoothed.reset(sampleRate, 0.02);
    doublerSmoothed.reset(sampleRate, 0.03);
    gateGainSmoothed.reset(sampleRate, 0.01);
    hpCutoffSmoothed.reset(sampleRate, 0.04);
    lpCutoffSmoothed.reset(sampleRate, 0.04);

    outputGainSmoothed.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(4.0f));
    delayTimeSmoothed.setCurrentAndTargetValue(300.0f);
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
    keyFFTData.fill(0.0f);
    keyAnalysisBuffer.fill(0.0f);
    keyChromaHistory.fill(0.0f);
    keyAnalysisWrite = keyAnalysisCount = keyAnalysisCountdown = keyAnalysisFrames = 0;
    detectedKeyRoot = -1;
    detectedKeyMinor = false;
    detectedKeyConfidence = 0.0f;
    pitchFrameScratch.fill(0.0f);
    pitchCmndScratch.fill(0.0f);
    pitchAnalysisWrite = pitchAnalysisCount = pitchAnalysisCountdown = 0;
    pitchAnalysisDecimation = juce::jmax(1, static_cast<int>(std::ceil(sampleRate / 24000.0)));
    pitchDecimationCounter = 0;
    pitchAnalysisSampleRate = static_cast<float>(sampleRate) / static_cast<float>(pitchAnalysisDecimation);
    detectedPitchHz = 0.0f;
    pitchRatio = 1.0f;
    pitchShiftPhase = 0.0f;
    pitchHoldSamples = 0;
    stableMidiNote = candidateMidiNote = -1;
    candidateNoteFrames = 0;
    pitchChannelEnergySmoothed.fill(0.0f);
    pitchAnalysisChannel = 0;
    pitchConfidence = 0.0f;
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
    const float safeSampleRate = static_cast<float>(getSampleRate());
    hpCutoffSmoothed.setTargetValue(juce::jlimit(20.0f, safeSampleRate * 0.40f, hpTarget));
    lpCutoffSmoothed.setTargetValue(juce::jlimit(2000.0f, safeSampleRate * 0.45f, lpTarget));

    // Smooth cutoff changes at block boundaries to avoid zipper noise.
    const float hpNow = hpCutoffSmoothed.skip(numSamples);
    const float lpNow = lpCutoffSmoothed.skip(numSamples);
    hpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), hpNow);
    lpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), lpNow);

    compressor.setThreshold(threshold);
    compressor.setRatio(ratio);
    // A firmer ratio gets a quicker grab and shorter recovery; gentler settings
    // retain more transient shape and use a longer, smoother release.
    compressor.setAttack(juce::jmap(ratio, 1.0f, 20.0f, 18.0f, 5.0f));
    compressor.setRelease(juce::jmap(ratio, 1.0f, 20.0f, 150.0f, 75.0f));

    delayTimeSmoothed.setTargetValue(delayMs);

    juce::dsp::Reverb::Parameters reverbParams;
    const float reverbTimeNorm = juce::jlimit(0.0f, 1.0f,
        (parameters.getRawParameterValue("reverbTime")->load() - 50.0f) / 1950.0f);
    reverbParams.roomSize = juce::jmap(reverbTimeNorm, 0.12f, 0.95f);
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

    // Track channel energy over time and switch analysis channels only when
    // another channel is clearly stronger. This avoids block-to-block channel
    // hopping on stereo vocals while still recovering from a silent channel.
    int strongestChannel = pitchAnalysisChannel;
    float strongestChannelEnergy = -1.0f;
    for (int channel = 0; channel < juce::jmin(numChannels, 2); ++channel)
    {
        float channelEnergy = 0.0f;
        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float value = buffer.getSample(channel, sample);
            channelEnergy += value * value;
        }
        channelEnergy /= static_cast<float>(juce::jmax(1, numSamples));
        auto& trackedEnergy = pitchChannelEnergySmoothed[static_cast<size_t>(channel)];
        trackedEnergy += 0.20f * (channelEnergy - trackedEnergy);
        if (trackedEnergy > strongestChannelEnergy)
        {
            strongestChannelEnergy = trackedEnergy;
            strongestChannel = channel;
        }
    }
    const float currentChannelEnergy = pitchChannelEnergySmoothed[
        static_cast<size_t>(juce::jlimit(0, 1, pitchAnalysisChannel))];
    if (strongestChannel != pitchAnalysisChannel
        && (currentChannelEnergy < 1.0e-8f
            || strongestChannelEnergy > currentChannelEnergy * 1.6f))
        pitchAnalysisChannel = strongestChannel;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float detector = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
            detector = juce::jmax(detector, std::abs(buffer.getSample(channel, sample)));
        const float monoForPitch = buffer.getSample(pitchAnalysisChannel, sample);

        // SPOTON: analyse at no more than 48 kHz. Decimation after the channel filters
        // preserves the vocal band while keeping detector cost predictable at 88.2/96/192 kHz.
        if (++pitchDecimationCounter >= pitchAnalysisDecimation)
        {
            pitchDecimationCounter = 0;
            pitchAnalysisBuffer[static_cast<size_t>(pitchAnalysisWrite)] = monoForPitch;
        pitchAnalysisWrite = (pitchAnalysisWrite + 1) % static_cast<int>(pitchAnalysisBuffer.size());
        pitchAnalysisCount = juce::jmin(pitchAnalysisCount + 1, static_cast<int>(pitchAnalysisBuffer.size()));
            // Feed the same stable analysis channel into a separate chroma window.
            keyAnalysisBuffer[static_cast<size_t>(keyAnalysisWrite)] = monoForPitch;
            keyAnalysisWrite = (keyAnalysisWrite + 1) % static_cast<int>(keyAnalysisBuffer.size());
            keyAnalysisCount = juce::jmin(keyAnalysisCount + 1, static_cast<int>(keyAnalysisBuffer.size()));
            ++keyAnalysisCountdown;
            if (keyAnalysisCount >= 2048 && keyAnalysisCountdown >= 1024)
            {
                keyAnalysisCountdown = 0;
                keyFFTData.fill(0.0f);
                for (int n = 0; n < 2048; ++n)
                {
                    const int index = (keyAnalysisWrite + n) % 2048;
                    const float window = 0.5f - 0.5f * std::cos(
                        6.28318530718f * static_cast<float>(n) / 2047.0f);
                    keyFFTData[static_cast<size_t>(n)] = keyAnalysisBuffer[static_cast<size_t>(index)] * window;
                }
                keyFFT.performFrequencyOnlyForwardTransform(keyFFTData.data());
                // Convert spectral peaks into a pitch-class energy profile. Restrict
                // the range to musical fundamentals/harmonics and discount sub-bass rumble.
                for (auto& energy : keyChromaHistory)
                    energy *= 0.985f;
                const float fftRate = pitchAnalysisSampleRate;
                for (int bin = 5; bin < 170; ++bin)
                {
                    const float frequency = static_cast<float>(bin) * fftRate / 2048.0f;
                    if (frequency < 55.0f || frequency > 2000.0f)
                        continue;
                    const float magnitude = keyFFTData[static_cast<size_t>(bin)];
                    if (magnitude <= 1.0e-5f)
                        continue;
                    const int midiNote = static_cast<int>(std::round(
                        69.0f + 12.0f * std::log2(frequency / 440.0f)));
                    const int pitchClass = (midiNote % 12 + 12) % 12;
                    keyChromaHistory[static_cast<size_t>(pitchClass)] += std::sqrt(magnitude);
                }
                ++keyAnalysisFrames;
                if (keyAnalysisFrames >= 32)
                {
                    static constexpr std::array<float, 12> majorProfile {
                        6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f,
                        2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f
                    };
                    static constexpr std::array<float, 12> minorProfile {
                        6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f,
                        2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f
                    };
                    float profileMean = 0.0f;
                    float chromaMean = 0.0f;
                    for (int pc = 0; pc < 12; ++pc)
                    {
                        profileMean += majorProfile[static_cast<size_t>(pc)];
                        chromaMean += keyChromaHistory[static_cast<size_t>(pc)];
                    }
                    profileMean /= 12.0f;
                    chromaMean /= 12.0f;
                    float bestScore = -2.0f;
                    float secondScore = -2.0f;
                    int bestRoot = -1;
                    bool bestMinor = false;
                    for (int root = 0; root < 12; ++root)
                    {
                        for (int mode = 0; mode < 2; ++mode)
                        {
                            const auto& profile = mode == 0 ? majorProfile : minorProfile;
                            float dot = 0.0f, profileNorm = 0.0f, chromaNorm = 0.0f;
                            for (int pc = 0; pc < 12; ++pc)
                            {
                                const float x = keyChromaHistory[static_cast<size_t>(pc)] - chromaMean;
                                const float y = profile[static_cast<size_t>((pc - root + 12) % 12)] - profileMean;
                                dot += x * y;
                                chromaNorm += x * x;
                                profileNorm += y * y;
                            }
                            const float score = dot / std::sqrt(chromaNorm * profileNorm + 1.0e-12f);
                            if (score > bestScore)
                            {
                                secondScore = bestScore;
                                bestScore = score;
                                bestRoot = root;
                                bestMinor = mode == 1;
                            }
                            else if (score > secondScore)
                                secondScore = score;
                        }
                    }
                    if (bestRoot >= 0)
                    {
                        detectedKeyRoot = bestRoot;
                        detectedKeyMinor = bestMinor;
                        detectedKeyConfidence = juce::jlimit(0.0f, 1.0f,
                            (bestScore - secondScore) * 5.0f);
                    }
                }
            }
        if (++pitchAnalysisCountdown >= 512 && pitchAnalysisCount >= 1024)
        {
            pitchAnalysisCountdown = 0;
            constexpr int windowSize = 1024;
            const int minLag = juce::jlimit(12, 80, static_cast<int>(pitchAnalysisSampleRate / 1500.0f));
            const int maxLag = juce::jlimit(minLag + 2, windowSize - 2,
                                            static_cast<int>(pitchAnalysisSampleRate / 55.0f));
            auto& frame = pitchFrameScratch;
            auto& cmnd = pitchCmndScratch;
            cmnd.fill(0.0f);
            float frameMean = 0.0f;
            for (int n = 0; n < windowSize; ++n)
            {
                const int index = (pitchAnalysisWrite - windowSize + n + static_cast<int>(pitchAnalysisBuffer.size()))
                                  % static_cast<int>(pitchAnalysisBuffer.size());
                frameMean += pitchAnalysisBuffer[static_cast<size_t>(index)];
            }
            frameMean /= static_cast<float>(windowSize);

            // Remove DC offset before the YIN difference calculation. This helps
            // prevent low-frequency bias from mic/interface offset.
            float frameEnergy = 0.0f;
            for (int n = 0; n < windowSize; ++n)
            {
                const int index = (pitchAnalysisWrite - windowSize + n + static_cast<int>(pitchAnalysisBuffer.size()))
                                  % static_cast<int>(pitchAnalysisBuffer.size());
                const float value = pitchAnalysisBuffer[static_cast<size_t>(index)] - frameMean;
                frame[static_cast<size_t>(n)] = value;
                frameEnergy += value * value;
            }
            const float frameRms = std::sqrt(frameEnergy / static_cast<float>(windowSize));
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
                if (lag >= minLag - 1)
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
            if (frameRms >= 0.003f)
            {
                for (int lag = minLag + 1; lag < maxLag; ++lag)
                {
                    const float value = cmnd[static_cast<size_t>(lag)];
                    if (value < 0.14f
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

            if (bestLag > 0 && bestValue < 0.20f && frameRms >= 0.003f)
            {
                const float left = cmnd[static_cast<size_t>(juce::jmax(minLag, bestLag - 1))];
                const float centre = cmnd[static_cast<size_t>(bestLag)];
                const float right = cmnd[static_cast<size_t>(juce::jmin(maxLag, bestLag + 1))];
                const float denominator = left - 2.0f * centre + right;
                const float offset = std::abs(denominator) > 1.0e-9f
                    ? 0.5f * (left - right) / denominator : 0.0f;
                const float refinedLag = static_cast<float>(bestLag) + juce::jlimit(-0.5f, 0.5f, offset);
                detectedPitchHz = pitchAnalysisSampleRate / refinedLag;
                const float midi = 69.0f + 12.0f * std::log2(detectedPitchHz / 440.0f);
                const int measuredNote = static_cast<int>(std::round(midi));

                // Do not calculate a correction against the previous note while a
                // new note is still being verified. That avoids a brief wrong-way
                // pitch pull during note transitions and octave-error rejection.
                bool targetNoteConfirmed = false;
                if (stableMidiNote < 0)
                {
                    stableMidiNote = measuredNote;
                    candidateMidiNote = -1;
                    candidateNoteFrames = 0;
                    targetNoteConfirmed = true;
                }
                else if (measuredNote == stableMidiNote)
                {
                    candidateMidiNote = -1;
                    candidateNoteFrames = 0;
                    targetNoteConfirmed = true;
                }
                else
                {
                    if (candidateMidiNote == measuredNote)
                        ++candidateNoteFrames;
                    else
                    {
                        candidateMidiNote = measuredNote;
                        candidateNoteFrames = 1;
                    }

                    // Large note jumps need stronger evidence than ordinary melodic
                    // steps; only a confirmed transition updates the correction target.
                    const int noteJump = std::abs(measuredNote - stableMidiNote);
                    const int requiredFrames = noteJump >= 7 ? 4 : 2;
                    if (candidateNoteFrames >= requiredFrames)
                    {
                        stableMidiNote = candidateMidiNote;
                        candidateMidiNote = -1;
                        candidateNoteFrames = 0;
                        targetNoteConfirmed = true;
                    }
                }

                if (targetNoteConfirmed)
                {
                    float semitones = (static_cast<float>(stableMidiNote) - midi) * spotOnAmount;
                    // A small cents deadband prevents the shifter chasing tiny detector
                    // fluctuations around the target note and reduces modulation artifacts.
                    if (std::abs(semitones) < 0.04f)
                        semitones = 0.0f;
                    pitchRatio = juce::jlimit(0.80f, 1.25f, std::pow(2.0f, semitones / 12.0f));
                }
                pitchHoldSamples = static_cast<int>(pitchAnalysisSampleRate * 0.12f);

                // Confidence now considers both periodicity (YIN score) and the
                // shape of the selected trough. A clear, well-defined minimum is
                // more trustworthy than a broad ambiguous valley with a similar
                // raw score, reducing correction on breathy/noisy material.
                const float neighbourMean = 0.5f * (left + right);
                const float troughShape = juce::jlimit(0.35f, 1.0f,
                    (neighbourMean - centre) / (neighbourMean + 1.0e-6f) * 4.0f);
                const float periodicityConfidence =
                    juce::jlimit(0.0f, 1.0f, (0.20f - bestValue) * 6.25f);
                const float measuredConfidence = periodicityConfidence * troughShape;
                // Fade correction authority while a new note is being confirmed;
                // never apply the previous note's target strongly to a new syllable.
                pitchConfidence = targetNoteConfirmed ? measuredConfidence : pitchConfidence * 0.65f;
            }
            else if (pitchHoldSamples > 0)
            {
                // Hold the last stable target briefly through consonants, while
                // fading confidence on each missed frame. This avoids the abrupt
                // full-wet-to-dry transition that can sound like a pitch-shift glitch.
                pitchHoldSamples = juce::jmax(0, pitchHoldSamples - 512);
                pitchConfidence *= 0.72f;
            }
            else
            {
                detectedPitchHz = 0.0f;
                pitchRatio = 1.0f;
                pitchConfidence = 0.0f;
                candidateMidiNote = -1;
                candidateNoteFrames = 0;
            }
            pitchRatioSmoothed.setTargetValue(pitchRatio);
            pitchConfidenceSmoothed.setTargetValue(pitchConfidence);
        }
        }

        const float envelopeCoefficient = detector > gateEnvelope
            ? std::exp(-1.0f / (0.006f * sampleRate))
            : std::exp(-1.0f / (0.120f * sampleRate));
        gateEnvelope = envelopeCoefficient * gateEnvelope + (1.0f - envelopeCoefficient) * detector;

        if (!gateIsOpen && gateEnvelope >= gateOpenThreshold)
            gateIsOpen = true;
        else if (gateIsOpen && gateEnvelope < gateCloseThreshold)
            gateIsOpen = false;

        gateGainSmoothed.setTargetValue(gateIsOpen ? 1.0f : 0.001f);
        const float gateGain = gateGainSmoothed.getNextValue();
        const float outputGain = outputGainSmoothed.getNextValue();
        const float correctionRatio = pitchRatioSmoothed.getNextValue();
        const float pitchWet = pitchConfidenceSmoothed.getNextValue()
            * (spotOnAmount > 0.001f ? 1.0f : 0.0f);
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
        const float currentDelayMs = delayTimeSmoothed.getNextValue();
        const int delaySamples = juce::jlimit(1, static_cast<int>(getSampleRate()) - 1,
            static_cast<int>(currentDelayMs * getSampleRate() * 0.001f));
        delayLine.setDelay(static_cast<float>(delaySamples));
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
            const float spotOnSignal = dry * (1.0f - pitchWet) + shifted * pitchWet;
            const float echo = delayLine.popSample(channel);

            // Controlled feedback creates smooth, progressively quieter delay repeats.
            delayLine.pushSample(channel, dry + echo * 0.22f);

            // Independent, gently modulated channel delays create stereo thickening
            // without reading the same delay buffer twice or cross-feeding channels.
            const float channelOffset = (numChannels > 1 && channel == 1) ? 0.0045f : 0.0f;
            const float modulation = 0.00065f * std::sin(doublerPhase + channel * 1.5707963f);
            const float doublerDelaySeconds = juce::jlimit(0.012f, 0.045f,
                                                            0.018f + channelOffset + modulation);
            doublerLine.setDelay(doublerDelaySeconds * static_cast<float>(getSampleRate()));
            const float doubled = doublerLine.popSample(channel);
            doublerLine.pushSample(channel, dry);

            const float delayed = spotOnSignal * (1.0f - delayWet) + echo * delayWet;
            const float wide = delayed * (1.0f - doublerAmount * 0.32f)
                             + doubled * (doublerAmount * 0.32f);
            const float drive = 1.0f + softenAmount * 0.65f;
            // Unity-slope soft saturation: preserve low-level vocal detail while gently
            // rounding peaks as the Soften control increases.
            const float rounded = std::tanh(wide * drive) / drive;
            const float softened = wide + (rounded - wide) * (softenAmount * 0.35f);
            // Final safety stage catches extreme sample peaks without
            // imposing a hard clip; normal programme levels remain effectively linear.
            const float postGain = softened * gateGain * outputGain;
            const float safeOutput = std::abs(postGain) > 0.98f
                ? std::copysign(0.98f + 0.02f * std::tanh((std::abs(postGain) - 0.98f) * 8.0f), postGain)
                : postGain;
            buffer.setSample(channel, sample, safeOutput);
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
