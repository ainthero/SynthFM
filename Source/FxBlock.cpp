/*
  ==============================================================================

    FxBlock.cpp
    Created: 28 Apr 2024 9:32:36pm
    Author:  freulaeuxx

  ==============================================================================
*/

#include "FxBlock.h"

void Overdrive::processBlock(juce::AudioBuffer<float>& buffer) {
    updateFilter();
    float* channelData = buffer.getWritePointer(0);
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        channelData[i] = std::tanh(channelData[i] * drive * 100);
    }

    filter.processSamples(channelData, buffer.getNumSamples());

    for (int channel = 1; channel < buffer.getNumChannels(); ++channel) {
        std::copy(channelData, channelData + buffer.getNumSamples(), buffer.getWritePointer(channel));
    }
}

void Overdrive::updateFilter() {
    float maxCutoff = 5000.0f;
    float minCutoff = 500.0f;
    float cutoff = minCutoff + (maxCutoff - minCutoff) * tone;
    filter.setCoefficients(juce::IIRCoefficients::makeLowPass(48000.0, cutoff));
}

void Reverb::processBlock(juce::AudioBuffer<float>& buffer) {
    static juce::Reverb reverb;
    static juce::Reverb::Parameters params;
    params.roomSize = roomSize;
    params.damping = damping;
    params.wetLevel = 0.8;
    params.dryLevel = 0.2;
    reverb.setParameters(params);

    reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
}

void Delay::processBlock(juce::AudioBuffer<float>& buffer) {
    int delaySamples = static_cast<int>(48000.0 * time);
    float feedbackGain = std::min(feedback, 0.95f);

    float* channelData0 = buffer.getWritePointer(0);
    float* channelData1 = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        float delayedSample = delayBuffer[delayBufferPos];

        delayBuffer[delayBufferPos] = channelData0[i] + delayedSample * feedbackGain;

        channelData0[i] = channelData0[i] * 0.5 + delayedSample * 0.5;
        channelData1[i] = channelData0[i];

        delayBufferPos = (delayBufferPos + 1) % delaySamples;
    }
}


void Flanger::processBlock(juce::AudioBuffer<float>& buffer) {
    int numSamples = buffer.getNumSamples();
    float sampleRate = 48000.0;
    float depthInSamples = depth * sampleRate / 1000.0;

    int maxDelaySamples = static_cast<int>(depthInSamples);
    if (delayBuffer.size() < maxDelaySamples) {
        delayBuffer.resize(maxDelaySamples, 0.0);
    }
    auto* channelData = buffer.getWritePointer(0);
    for (int i = 0; i < numSamples; ++i) {
        float modulatedDelay = depthInSamples * (0.5f + 0.4f * std::sin(phase));
        int intDelay = static_cast<int>(modulatedDelay);
        float fracDelay = modulatedDelay - intDelay;
            
        phase += rate * juce::MathConstants<float>::twoPi / sampleRate;
        if (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;

            float currentSample = channelData[i];

            int readPos = (delayBufferPos - intDelay + maxDelaySamples) % maxDelaySamples;
            float delayedSample1 = delayBuffer[readPos];

            channelData[i] = currentSample * 0.7 + delayedSample1 * 0.3;

            delayBuffer[delayBufferPos] = currentSample;

        delayBufferPos = (delayBufferPos + 1) % maxDelaySamples;
    }
    for (int channel = 1; channel < buffer.getNumChannels(); ++channel) {
        std::copy(channelData, channelData + buffer.getNumSamples(), buffer.getWritePointer(channel));
    }
}



void Chorus::processBlock(juce::AudioBuffer<float>& buffer) {
    int numSamples = buffer.getNumSamples();
    float sampleRate = 48000.0;

    auto* channelData = buffer.getWritePointer(0);

    for (int i = 0; i < numSamples; ++i) {
        float pitchModulation1 = 1.0 + 0.001 * std::sin(phase + 0.5);
        float pitchModulation2 = 1.0 + 0.001 * std::sin(phase - 0.5);

        float modulatedDelay1 = 20 + depth * sampleRate * (0.5f + 0.4f * std::sin(phase)) / 1000 * pitchModulation1;
        int intDelay1 = static_cast<int>(modulatedDelay1);
        float fracDelay1 = modulatedDelay1 - intDelay1;

        float modulatedDelay2 = 20 + depth * sampleRate * (0.5f + 0.4f * std::sin(phase + juce::MathConstants<float>::pi)) / 1000 * pitchModulation2;
        int intDelay2 = static_cast<int>(modulatedDelay2);
        float fracDelay2 = modulatedDelay2 - intDelay2;

        phase += rate * juce::MathConstants<float>::twoPi / sampleRate;
        if (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;

        int readPos1 = (delayBufferPos - intDelay1 + delayBuffer.size()) % delayBuffer.size();
        float delayedSample1_1 = delayBuffer[readPos1];
        float delayedSample1_2 = delayBuffer[(readPos1 + 1) % delayBuffer.size()];
        float interpolatedSample1 = delayedSample1_1 + fracDelay1 * (delayedSample1_2 - delayedSample1_1);

        int readPos2 = (delayBufferPos - intDelay2 + delayBuffer.size()) % delayBuffer.size();
        float delayedSample2_1 = delayBuffer[readPos2];
        float delayedSample2_2 = delayBuffer[(readPos2 + 1) % delayBuffer.size()];
        float interpolatedSample2 = delayedSample2_1 + fracDelay2 * (delayedSample2_2 - delayedSample2_1);

        channelData[i] = channelData[i] * 0.3 + interpolatedSample1 * 0.3 + interpolatedSample2 * 0.4;

        delayBuffer[delayBufferPos] = channelData[i];

        delayBufferPos = (delayBufferPos + 1) % delayBuffer.size();
    }
    for (int channel = 1; channel < buffer.getNumChannels(); ++channel) {
        std::copy(channelData, channelData + buffer.getNumSamples(), buffer.getWritePointer(channel));
    }
}



void Filter::updateFilter() {
    auto highPassCoeffs = juce::IIRCoefficients::makeHighPass(48000.0, 21000 * lowCut, resonance);
    highPassFilter.setCoefficients(highPassCoeffs);

    auto lowPassCoeffs = juce::IIRCoefficients::makeLowPass(48000.0, 21000 * highCut, resonance);
    lowPassFilter.setCoefficients(lowPassCoeffs);
}

void Filter::processBlock(juce::AudioBuffer<float>& buffer) {
    updateFilter();
    float* channelData = buffer.getWritePointer(0);
    highPassFilter.processSamples(channelData, buffer.getNumSamples());
    lowPassFilter.processSamples(channelData, buffer.getNumSamples());

    for (int channel = 1; channel < buffer.getNumChannels(); ++channel) {
        std::copy(channelData, channelData + buffer.getNumSamples(), buffer.getWritePointer(channel));
    }
}


FxBlock::FxBlock(const std::string& name)
    : name(name), isActive(false), parameters(getDefaultParameters(name)) {
    if (name == "Overdrive") {
        effect = std::make_unique<std::variant<Overdrive, Reverb, Delay, Flanger, Chorus, Filter>>(Overdrive());
    }
    else if (name == "Reverb") {
        effect = std::make_unique<std::variant<Overdrive, Reverb, Delay, Flanger, Chorus, Filter>>(Reverb());
    }
    else if (name == "Delay") {
        effect = std::make_unique<std::variant<Overdrive, Reverb, Delay, Flanger, Chorus, Filter>>(Delay());
    }
    else if (name == "Flanger") {
        effect = std::make_unique<std::variant<Overdrive, Reverb, Delay, Flanger, Chorus, Filter>>(Flanger());
    }
    else if (name == "Chorus") {
        effect = std::make_unique<std::variant<Overdrive, Reverb, Delay, Flanger, Chorus, Filter>>(Chorus());
    }
    else if (name == "Filter") {
        effect = std::make_unique<std::variant<Overdrive, Reverb, Delay, Flanger, Chorus, Filter>>(Filter());
    }
}

void FxBlock::processBlock(juce::AudioBuffer<float>& buffer) {
    std::visit([&](auto& eff) {
        eff.processBlock(buffer);
        }, *effect);
}

std::map<std::string, float> FxBlock::getDefaultParameters(const std::string& name) {
    if (name == "Overdrive") {
        return { {"Drive", 0.5f}, {"Tone", 0.5f} };
    }
    else if (name == "Reverb") {
        return { {"Damping", 0.7f}, {"Room Size", 0.6f} };
    }
    else if (name == "Delay") {
        return { {"Feedback", 0.5f}, {"Time", 0.5f} };
    }
    else if (name == "Flanger") {
        return { {"Depth", 0.5f}, {"Rate", 0.25f} };
    }
    else if (name == "Chorus") {
        return { {"Depth", 0.5f}, {"Rate", 0.25f} };
    }
    else if (name == "Filter") {
        return { {"HighCut", 1.0f}, {"LowCut", 0.01f} };
    }
    return {};
}

FxBlockComponent::FxBlockComponent() {
    addAndMakeVisible(nameLabel);
    addAndMakeVisible(upButton);
    addAndMakeVisible(downButton);
    addAndMakeVisible(enableButton);
    addAndMakeVisible(param1Slider);
    addAndMakeVisible(param2Slider);
    addAndMakeVisible(param1Label);
    addAndMakeVisible(param2Label);

    param1Slider.onValueChange = [this] {
        if (auto* list = findParentComponentOfClass<FxList>()) {
            list->setEffect1(indexInList, param1Slider.getValue());
        }
        };

    param2Slider.onValueChange = [this] {
        if (auto* list = findParentComponentOfClass<FxList>()) {
            list->setEffect2(indexInList, param2Slider.getValue());
        }
        };

    enableButton.onClick = [this] {
        if (auto* list = findParentComponentOfClass<FxList>()) {
            list->toggleEffectEnabled(indexInList);
        }
        };

    upButton.onClick = [this] {
        if (auto* list = findParentComponentOfClass<FxList>()) {
            list->moveEffectUp(indexInList);
        }
        };

    downButton.onClick = [this] {
        if (auto* list = findParentComponentOfClass<FxList>()) {
            list->moveEffectDown(indexInList);
        }
        };
}

void FxBlockComponent::resized() {
    int width = getWidth();
    int height = getHeight();

    nameLabel.setBounds(10, 10, width - 20, 20);

    enableButton.setBounds(10, 40, 70, 20);
    upButton.setBounds(90, 20, 70, 20);
    downButton.setBounds(90, 40, 70, 20);

    int sliderWidth = 70;
    int labelHeight = 20;
    int sliderHeight = 70;

    int param1X = 250;
    param1Label.setBounds(param1X - 55, 20, sliderWidth, labelHeight);
    param1Slider.setBounds(param1X, 0, sliderWidth, sliderHeight);
    param1Slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 15);

    int param2X = param1X + sliderWidth + 150;
    param2Label.setBounds(param2X - 55, 20, sliderWidth, labelHeight);
    param2Slider.setBounds(param2X, 0, sliderWidth, sliderHeight);
    param2Slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 15);
}


void FxBlockComponent::setFxBlock(const FxBlock& fxBlock) {
    nameLabel.setText(fxBlock.name, juce::dontSendNotification);
    enableButton.setToggleState(fxBlock.isActive, juce::dontSendNotification);
    auto it = fxBlock.parameters.begin();

    param1Slider.setValue(it->second);
    param1Slider.setName(it->first);
    param1Slider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    param1Slider.setRange(0.01, 1.0, 0.01);
    param1Slider.setNumDecimalPlacesToDisplay(2);
    param1Slider.setTextValueSuffix("");
    param1Label.setText(it->first, juce::dontSendNotification);

    if (++it != fxBlock.parameters.end()) {
        param2Slider.setValue(it->second);
        param2Slider.setName(it->first);
        param2Slider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        param2Slider.setRange(0.01, 1.0, 0.01);
        param2Slider.setNumDecimalPlacesToDisplay(2);
        param2Slider.setTextValueSuffix("");
        param2Label.setText(it->first, juce::dontSendNotification);
    }
}

void FxBlockComponent::mouseEnter(const juce::MouseEvent&) {
    isMouseOver = true;
    repaint();
}

void FxBlockComponent::mouseExit(const juce::MouseEvent&) {
    isMouseOver = false;
    repaint();
}

FxList::FxList() {
    setModel(this);
    effects.emplace_back("Overdrive");
    effects.emplace_back("Reverb");
    effects.emplace_back("Delay");
    effects.emplace_back("Flanger");
    effects.emplace_back("Chorus");
    effects.emplace_back("Filter");
    setRowHeight(75);
}

int FxList::getNumRows() {
    return static_cast<int>(effects.size());
}

void FxList::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) {
    if (rowNumber < 0 || rowNumber >= effects.size()) {
        return;
    }
    if (rowIsSelected) {
        g.fillAll(juce::Colours::lightblue);
    }

    g.setColour(juce::Colours::black);

    g.setColour(juce::Colours::grey);
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.drawLine(0, height - 1, width, height - 1);
}



juce::Component* FxList::refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) {
    if (rowNumber < 0 || rowNumber >= effects.size())
        return nullptr;

    auto* component = static_cast<FxBlockComponent*>(existingComponentToUpdate);
    if (component == nullptr)
        component = new FxBlockComponent();

    component->setFxBlock(effects[rowNumber]);
    component->indexInList = rowNumber;
    return component;
}

void FxList::toggleEffectEnabled(int index) {
    if (index >= 0 && index < effects.size()) {
        effects[index].isActive = !effects[index].isActive;
        updateContent();
    }
}

void FxList::setEffect1(int index, float value) {
    if (effects[index].name == "Overdrive") {
        effects[index].parameters["Drive"] = value;
        std::get<Overdrive>(*effects[index].effect).drive = value;
    }
    else if (effects[index].name == "Reverb") {
        effects[index].parameters["Damping"] = value;
        std::get<Reverb>(*effects[index].effect).damping = value;
    }
    else if (effects[index].name == "Delay") {
        effects[index].parameters["Feedback"] = value;
        std::get<Delay>(*effects[index].effect).feedback = value;
    }
    else if (effects[index].name == "Flanger") {
        effects[index].parameters["Depth"] = value;
        std::get<Flanger>(*effects[index].effect).depth = value * 15;
    }
    else if (effects[index].name == "Chorus") {
        effects[index].parameters["Depth"] = value;
        std::get<Chorus>(*effects[index].effect).depth = 15 * value;
    }
    else if (effects[index].name == "Filter") {
        effects[index].parameters["HighCut"] = value;
        std::get<Filter>(*effects[index].effect).highCut = value;
    }
}

void FxList::setEffect2(int index, float value) {
    if (effects[index].name == "Overdrive") {
        effects[index].parameters["Tone"] = value;
        std::get<Overdrive>(*effects[index].effect).tone = value;
    }
    else if (effects[index].name == "Reverb") {
        effects[index].parameters["Room Size"] = value;
        std::get<Reverb>(*effects[index].effect).roomSize = value;
    }
    else if (effects[index].name == "Delay") {
        effects[index].parameters["Time"] = value;
        std::get<Delay>(*effects[index].effect).time = value;
    }
    else if (effects[index].name == "Flanger") {
        effects[index].parameters["Rate"] = value;
        std::get<Flanger>(*effects[index].effect).rate = value * 5;
    }
    else if (effects[index].name == "Chorus") {
        effects[index].parameters["Rate"] = value;
        std::get<Chorus>(*effects[index].effect).rate = value;
    }
    else if (effects[index].name == "Filter") {
        effects[index].parameters["LowCut"] = value;
        std::get<Filter>(*effects[index].effect).lowCut = value;
    }
}

void FxList::moveEffectUp(int index) {
    if (index > 0 && index < effects.size()) {
        std::swap(effects[index], effects[index - 1]);
        updateContent();
        repaint();
    }
}

void FxList::moveEffectDown(int index) {
    if (index >= 0 && index < effects.size() - 1) {
        std::swap(effects[index], effects[index + 1]);
        updateContent();
        repaint();
    }
}