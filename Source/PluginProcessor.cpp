#include "PluginProcessor.h"
#include "PluginEditor.h"

AJERRYAudioProcessor::AJERRYAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Params", createParams()) {}

AJERRYAudioProcessor::~AJERRYAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout AJERRYAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 2.0f, 1.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 1.0f, 10.0f, 1.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("tone", "Tone", 200.0f, 10000.0f, 5000.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 1.0f));
    return { p.begin(), p.end() };
}

const juce::String AJERRYAudioProcessor::getName() const { return "AJERRY"; }
bool AJERRYAudioProcessor::acceptsMidi() const { return false; }
bool AJERRYAudioProcessor::producesMidi() const { return false; }
bool AJERRYAudioProcessor::isMidiEffect() const { return false; }
double AJERRYAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int AJERRYAudioProcessor::getNumPrograms() { return 1; }
int AJERRYAudioProcessor::getCurrentProgram() { return 0; }
void AJERRYAudioProcessor::setCurrentProgram(int) {}
const juce::String AJERRYAudioProcessor::getProgramName(int) { return {}; }
void AJERRYAudioProcessor::changeProgramName(int, const juce::String&) {}
void AJERRYAudioProcessor::prepareToPlay(double, int) {}
void AJERRYAudioProcessor::releaseResources() {}

bool AJERRYAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}

void AJERRYAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float gain = *apvts.getRawParameterValue("gain");
    float drive = *apvts.getRawParameterValue("drive");
    float tone = *apvts.getRawParameterValue("tone");
    float mix = *apvts.getRawParameterValue("mix");

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* data = buffer.getWritePointer(channel);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float clean = data[s];
            float driven = std::tanh(clean * gain * drive);
            float alpha = tone / (tone + 1000.0f);
            driven = driven * alpha + clean * (1.0f - alpha) * 0.1f;
            data[s] = clean * (1.0f - mix) + driven * mix;
        }
    }
}

bool AJERRYAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* AJERRYAudioProcessor::createEditor() { return new AJERRYAudioProcessorEditor(*this); }
void AJERRYAudioProcessor::getStateInformation(juce::MemoryBlock& destData) { auto state = apvts.copyState(); std::unique_ptr<juce::XmlElement> xml(state.createXml()); copyXmlToBinary(*xml, destData); }
void AJERRYAudioProcessor::setStateInformation(const void* data, int sizeInBytes) { std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes)); if (xmlState.get() != nullptr) if (xmlState->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(*xmlState)); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AJERRYAudioProcessor(); }
