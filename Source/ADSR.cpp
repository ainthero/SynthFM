/*
  ==============================================================================

    ADSR.cpp
    Created: 21 Apr 2024 3:15:04am
    Author:  freulaeuxx

  ==============================================================================
*/

#include "ADSR.h"

ADSR::ADSR()
    : state(State::Idle), envelopeLevel(0.0f), attackRate(0.001f),
    decayRate(0.001f), sustainLevel(0.7f), releaseRate(0.001f), sampleRate(48000.0) {}

float ADSR::applyEnvelope(float inputSample) {
    switch (state) {
    case State::Attack:
        envelopeLevel += attackRate;
        if (envelopeLevel >= 1.0f) {
            envelopeLevel = 1.0f;
            state = State::Decay;
        }
        break;
    case State::Decay:
        envelopeLevel -= decayRate;
        if (envelopeLevel <= sustainLevel) {
            envelopeLevel = sustainLevel;
            state = State::Sustain;
        }
        break;
    case State::Sustain:
        // Ïîääåðæàíèå óðîâíÿ sustain, èçìåíåíèé íåò
        break;
    case State::Release:
        envelopeLevel -= releaseRate;
        if (envelopeLevel <= 0.0f) {
            envelopeLevel = 0.0f;
            state = State::Idle;
        }
        break;
    case State::Idle:
        break;
    }
    return inputSample * envelopeLevel;
}

void ADSR::noteOn() {
    state = State::Attack;
}

void ADSR::noteOff() {
    state = State::Release;
}

void ADSR::setAttackTime(float attackTimeSeconds) {
    attackRate = 1.0f / (sampleRate * attackTimeSeconds);
}

void ADSR::setDecayTime(float decayTimeSeconds) {
    decayRate = (1.0f - sustainLevel) / (sampleRate * decayTimeSeconds);
}

void ADSR::setSustainLevel(float sustainLevelValue) {
    sustainLevel = sustainLevelValue;
}

void ADSR::setReleaseTime(float releaseTimeSeconds) {
    releaseRate = sustainLevel / (sampleRate * releaseTimeSeconds);
}

void ADSR::setSampleRate(float newSampleRate) {
    sampleRate = newSampleRate;
}

void ADSR::reset() {
    state = State::Idle;
    envelopeLevel = 0.0f;
}
