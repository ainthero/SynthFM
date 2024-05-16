/*
  ==============================================================================

    Oscillator.cpp
    Created: 14 Apr 2024 12:17:03am
    Author:  freulaeuxx

  ==============================================================================
*/

#include "Oscillator.h" 

Oscillator::Oscillator()
    : type(Sine), frequency(0.0), phase(0.0), sampleRate(48000.0), phaseIncrement(0.0), level(0.0) {
    updatePhaseIncrement();
}

void Oscillator::setWaveType(WaveType newType) {
    type = newType;
}

Oscillator::WaveType Oscillator::getWaveType() {
    return type;
}

float Oscillator::getFrequency() {
    return frequency;
}

void Oscillator::setFrequency(float newFrequency) {
    frequency = newFrequency;
    updatePhaseIncrement();
}

void Oscillator::setSampleRate(float newSampleRate) {
    sampleRate = newSampleRate;
    updatePhaseIncrement();
}

void Oscillator::updatePhaseIncrement() {
    phaseIncrement = frequency * frequencyMultiplier * detuneFactor * juce::MathConstants<float>::twoPi / sampleRate;
}

void Oscillator::setLevel(float newLevel) {
    level = newLevel;
}

float Oscillator::getLevel() {
    return level;
}

float Oscillator::nextSample() {
    float sample = 0.0;
    switch (type) {
    case Sine:
        sample = std::sin(phase);
        break;
    case Square:
        sample = (std::sin(phase) >= 0.0) ? 1.0 : -1.0;
        break;
    case Triangle:
        sample = 2.0 * std::asin(std::sin(phase)) / juce::MathConstants<float>::pi;
        break;
    case Saw:
        sample = 2.0 * (phase / juce::MathConstants<float>::twoPi) - 1.0;
        break;
    }
    float filteredSample = sample - lastSample + 0.995 * lastSample;
    lastSample = filteredSample;

    phase += phaseIncrement;
    if (phase >= juce::MathConstants<float>::twoPi)
        phase -= juce::MathConstants<float>::twoPi;

    return adsr.applyEnvelope(filteredSample);
}

void Oscillator::reset() {
    phase = 0.0;
    phaseIncrement = 0.0;
    adsr.reset();
}

void Oscillator::setOctave(int octave) {
    frequencyMultiplier = std::pow(2.0, octave);
}

void Oscillator::setDetune(float cents) {
    detuneFactor = std::pow(2.0, cents / 1200.0);
}

void Oscillator::noteOn() {
    adsr.noteOn();
}

void Oscillator::noteOff() {
    adsr.noteOff();
}

void Oscillator::setAttackTime(float time) {
    adsr.setAttackTime(time);
}

void Oscillator::setDecayTime(float time) {
    adsr.setDecayTime(time);
}

void Oscillator::setSustainLevel(float level) {
    adsr.setSustainLevel(level);
}

void Oscillator::setReleaseTime(float time) {
    adsr.setReleaseTime(time);
}
