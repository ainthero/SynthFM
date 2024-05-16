/*
  ==============================================================================

    ADSR.h
    Created: 21 Apr 2024 3:15:04am
    Author:  freulaeuxx

  ==============================================================================
*/

#pragma once
#include <algorithm>

class ADSR {
public:
    enum class State {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    ADSR();

    float applyEnvelope(float inputSample);

    void noteOn();
    void noteOff();

    void setAttackTime(float attackTimeSeconds);
    void setDecayTime(float decayTimeSeconds);
    void setSustainLevel(float sustainLevelValue);
    void setReleaseTime(float releaseTimeSeconds);
    void setSampleRate(float newSampleRate);
    void reset();

private:
    State state;
    float envelopeLevel;
    float attackRate;
    float decayRate;
    float sustainLevel;
    float releaseRate;
    float sampleRate;
};
