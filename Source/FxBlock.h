/*
  ==============================================================================

    FxBlock.h
    Created: 28 Apr 2024 9:32:36pm
    Author:  freulaeuxx

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <variant>
#include <cmath>

class Overdrive {
public:
    float drive;
    float tone;
    juce::IIRFilter filter;

    Overdrive(float drive = 0.5f, float tone = 0.5f)
        : drive(drive), tone(tone) {}

    void processBlock(juce::AudioBuffer<float>& buffer);
    void updateFilter();
};

class Reverb {
public:
    float roomSize;
    float damping;

    Reverb(float roomSize = 0.7f, float damping = 0.6f)
        : roomSize(roomSize), damping(damping) {}

    void processBlock(juce::AudioBuffer<float>& buffer);
};

class Delay {
public:
    float time;
    float feedback;
    std::vector<float> delayBuffer;
    int delayBufferPos;

    Delay(float time = 0.5f, float feedback = 0.5f)
        : time(time), feedback(feedback), delayBufferPos(0) {
        delayBuffer.resize(48000.0);
    }

    void processBlock(juce::AudioBuffer<float>& buffer);
};

class Flanger {
public:
    float rate;
    float depth;
    float sampleRate;
    float phase;

    std::vector<float> delayBuffer;
    int delayBufferPos;

    Flanger(float sr = 48000.0, float rate = 0.25f, float depth = 0.5f)
        : rate(rate * 5), depth(depth * 15), sampleRate(sr), phase(0.0f), delayBufferPos(0) {
        int delayBufferSize = static_cast<int>(this->depth / 1000.0f * sampleRate);
        delayBuffer.resize(delayBufferSize, 0.0f);
    }

    void processBlock(juce::AudioBuffer<float>& buffer);
};

class Chorus {
public:
    float rate; 
    float depth;
    std::vector<float> delayBuffer;
    int delayBufferPos;
    float phase;
    float sampleRate;

    Chorus(float rate = 0.25f, float depth = 0.5f, float sampleRate = 48000.0f)
        : rate(rate), depth(15 * depth), sampleRate(sampleRate), delayBufferPos(0), phase(0) {
        int maxDelaySamples = static_cast<int>(depth * sampleRate);
        delayBuffer.resize(maxDelaySamples, 0.0f);
    }

    void processBlock(juce::AudioBuffer<float>& buffer);
};

class Filter {
public:
    float highCut;
    float lowCut;
    float resonance;
    juce::IIRFilter highPassFilter;
    juce::IIRFilter lowPassFilter;

    Filter(float highCut = 1.0f, float lowCut = 0.01f, float resonance = 1.0f)
        : highCut(highCut), lowCut(lowCut), resonance(resonance) {
        updateFilter();
    }

    void updateFilter();
    void processBlock(juce::AudioBuffer<float>& buffer);
};


struct FxBlock {
    std::string name;
    bool isActive;
    std::map<std::string, float> parameters;
    std::unique_ptr <std::variant<Overdrive, Reverb, Delay, Flanger, Chorus, Filter>> effect;

    FxBlock(const std::string& name);
    void processBlock(juce::AudioBuffer<float>& buffer);
    std::map<std::string, float> getDefaultParameters(const std::string& name);
};

class FxBlockComponent : public juce::Component {
public:
    juce::Label nameLabel;
    juce::TextButton upButton{ "^" };
    juce::TextButton downButton{ "v" };
    juce::ToggleButton enableButton{ "Enable" };
    juce::Slider param1Slider;
    juce::Slider param2Slider;
    juce::Label param1Label;
    juce::Label param2Label;
    int indexInList;
    bool isMouseOver;


    FxBlockComponent();

    void resized() override;

    void setFxBlock(const FxBlock& fxBlock);

    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
};


class FxList : public juce::ListBox, public juce::ListBoxModel {

public:
    FxList();

    int getNumRows() override;

    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate);

    void toggleEffectEnabled(int index);

    void moveEffectUp(int index);

    void moveEffectDown(int index);

    void setEffect1(int index, float value);
    void setEffect2(int index, float value);

    std::vector<FxBlock> effects;
};


