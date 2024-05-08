/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "FxBlock.h"

//==============================================================================
/**
*/
class SynthFMAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SynthFMAudioProcessorEditor(SynthFMAudioProcessor&);
    ~SynthFMAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged);
    void showFxInterface();
    void showSynthInterface();

private:
    SynthFMAudioProcessor& processor;
    juce::MidiKeyboardComponent keyboardComponent;
    juce::ComboBox waveTypeSelector[4];
    juce::Slider modulationDepthDials[4][4];
    juce::ToggleButton enableModulation[4][4];
    juce::Slider octaveDials[4];
    juce::Slider detuneDials[4];
    juce::Slider attackSliders[4];
    juce::Slider decaySliders[4];
    juce::Slider sustainSliders[4];
    juce::Slider releaseSliders[4];
    juce::Slider levelDials[4];

    juce::Slider levelSlider;
    juce::Label levelLabel;

    juce::Label attackLabels[4];
    juce::Label decayLabels[4];
    juce::Label sustainLabels[4];
    juce::Label releaseLabels[4];

    juce::TextButton synthButton{ "Synth" };
    juce::TextButton fxButton{ "FX" };

    bool isSynth = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthFMAudioProcessorEditor)
};
