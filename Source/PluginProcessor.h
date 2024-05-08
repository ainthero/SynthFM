#pragma once

#include <JuceHeader.h>
#include "Oscillator.h"
#include "ModulationMatrix.h"
#include "FxBlock.h"

class SynthFMAudioProcessor : public juce::AudioProcessor {
public:
    SynthFMAudioProcessor();
    ~SynthFMAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    Oscillator* getOscillator(int index);

    bool setModulationDepth(int carrierIdx, int modulatorIdx, float modulationDepth);
    void setOscillatorLevel(int index, float level);
    void setLevel(float level);
    void setOscillatorOctave(int index, int octave);
    void setOscillatorDetune(int index, float detune);
    void setOscillatorAttack(int index, float time);
    void setOscillatorDecay(int index, float time);
    void setOscillatorSustain(int index, float level);
    void setOscillatorRelease(int index, float time);

    juce::MidiKeyboardState keyboardState;
    FxList fxList;
private:
    bool noteCurrentlyOn = false;
    double currentFrequency = 0.0;
    double currentSampleRate = 48000.0;

    std::vector<Oscillator*> oscillators;
    ModulationMatrix modulationMatrix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthFMAudioProcessor)
};
