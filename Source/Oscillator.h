/*
  ==============================================================================

    Oscillator.h
    Created: 14 Apr 2024 12:17:03am
    Author:  freulaeuxx

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ADSR.h"

class Oscillator {
public:
    enum WaveType {
        Sine,
        Square,
        Triangle,
        Saw
    };

private:
    WaveType type;
    float frequency;
    float phase;
    float sampleRate;
    float phaseIncrement;
    float lastSample = 0;
    float level;

    double frequencyMultiplier = 1.0;
    double detuneFactor = 1.0;

    ADSR adsr;

public:
    Oscillator();

    void setWaveType(WaveType newType);
    WaveType getWaveType();
    void setFrequency(float newFrequency);
    float getFrequency();
    void setSampleRate(float newSampleRate);
    void updatePhaseIncrement();
    float nextSample();
    void reset();
    void setLevel(float newLevel);
    float getLevel();
    void setOctave(int octave);
    void setDetune(float cents);

    void noteOn();
    void noteOff();
    void setAttackTime(float time);
    void setDecayTime(float time);
    void setSustainLevel(float level);
    void setReleaseTime(float time);
};

