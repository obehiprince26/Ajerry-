#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class AjerryVST3AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
 explicit AjerryVST3AudioProcessorEditor(AjerryVST3AudioProcessor&);
 void paint(juce::Graphics&) override; void resized() override;
private:
 void changeTab(int);
 AjerryVST3AudioProcessor& proc; juce::TextButton tabs[8]; juce::Slider knobs[3]; juce::Label labels[3],vals[3],title,footer;
 using Attachment=juce::AudioProcessorValueTreeState::SliderAttachment;
 std::unique_ptr<Attachment> attachments[3]; int page=0;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AjerryVST3AudioProcessorEditor)
};
