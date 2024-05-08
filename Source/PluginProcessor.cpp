/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SynthFMAudioProcessor::SynthFMAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    oscillators.resize(4);
    for (auto& osc : oscillators) {
        osc = new Oscillator();
    }
    modulationMatrix = ModulationMatrix(oscillators);
}

SynthFMAudioProcessor::~SynthFMAudioProcessor()
{
}

//==============================================================================
const juce::String SynthFMAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SynthFMAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SynthFMAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SynthFMAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SynthFMAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SynthFMAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SynthFMAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SynthFMAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SynthFMAudioProcessor::getProgramName (int index)
{
    return {};
}

void SynthFMAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SynthFMAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for (auto& osc : oscillators) {
        osc->setSampleRate(sampleRate);
    }
}

void SynthFMAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void SynthFMAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    buffer.clear();
    juce::MidiMessage m;
    int time;

    juce::MidiBuffer::Iterator it(midiMessages);
    while (it.getNextEvent(m, time)) {
        if (m.isNoteOn()) {
            currentFrequency = juce::MidiMessage::getMidiNoteInHertz(m.getNoteNumber() - 12);
            for (auto& osc : oscillators) {
                osc->reset();
                osc->setFrequency(currentFrequency);
                osc->noteOn();
            }
            noteCurrentlyOn = true;
        }
        else if (m.isNoteOff()) {
            noteCurrentlyOn = false;
        }
    }

    if (noteCurrentlyOn) {
        float* channelData0 = buffer.getWritePointer(0);
        float* channelData1 = buffer.getWritePointer(1);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            auto nextSample = modulationMatrix.process();
            channelData0[sample] = nextSample;
            channelData1[sample] = nextSample;
        }
        for (auto& effect : fxList.effects) {
            if (effect.isActive) {
                effect.processBlock(buffer);
            }
        }
    }
    else {
        for (auto& osc : oscillators) {
            //osc->setFrequency(0.0);
            osc->noteOff();
        }
        float* channelData0 = buffer.getWritePointer(0);
        float* channelData1 = buffer.getWritePointer(1);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            auto nextSample = modulationMatrix.process();
            channelData0[sample] = nextSample;
            channelData1[sample] = nextSample;
        }
        for (auto& effect : fxList.effects) {
            if (effect.isActive) {
                effect.processBlock(buffer);
            }
        }
    }
}



//==============================================================================
bool SynthFMAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SynthFMAudioProcessor::createEditor()
{
    return new SynthFMAudioProcessorEditor (*this);
}

//==============================================================================
void SynthFMAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SynthFMAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

Oscillator* SynthFMAudioProcessor::getOscillator(int index) {
    return oscillators[index];
}

bool SynthFMAudioProcessor::setModulationDepth(int carrierIdx, int modulatorIdx, float modulationDepth) {
    if (modulationDepth < 0.00) {
        return modulationMatrix.removeModulation(carrierIdx, modulatorIdx);
    } else {
        return modulationMatrix.setModulation(carrierIdx, modulatorIdx, modulationDepth);
    }
}

void SynthFMAudioProcessor::setOscillatorLevel(int index, float level) {
    oscillators[index]->setLevel(level);
}

void SynthFMAudioProcessor::setLevel(float level) {
    modulationMatrix.setLevel(level);
}

void SynthFMAudioProcessor::setOscillatorOctave(int index, int octave) {
    if (index >= 0 && index < oscillators.size()) {
        oscillators[index]->setOctave(octave);
        // Äîïîëíèòåëüíûå äåéñòâèÿ ïîñëå èçìåíåíèÿ îêòàâû, åñëè íåîáõîäèìî
    }
}

void SynthFMAudioProcessor::setOscillatorDetune(int index, float detune) {
    if (index >= 0 && index < oscillators.size()) {
        oscillators[index]->setDetune(detune);
        // Äîïîëíèòåëüíûå äåéñòâèÿ ïîñëå èçìåíåíèÿ äåòþíà, åñëè íåîáõîäèìî
    }
}

void SynthFMAudioProcessor::setOscillatorAttack(int index, float time) {
    oscillators[index]->setAttackTime(time);
}

void SynthFMAudioProcessor::setOscillatorDecay(int index, float time) {
    oscillators[index]->setDecayTime(time);
}

void SynthFMAudioProcessor::setOscillatorSustain(int index, float level) {
    oscillators[index]->setSustainLevel(level);
}

void SynthFMAudioProcessor::setOscillatorRelease(int index, float time) {
    oscillators[index]->setReleaseTime(time);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthFMAudioProcessor();
}
