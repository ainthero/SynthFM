/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SynthFMAudioProcessorEditor::SynthFMAudioProcessorEditor(SynthFMAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), keyboardComponent(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    keyboardComponent.setKeyWidth(24);
    addAndMakeVisible(keyboardComponent);

    for (int i = 0; i < 4; ++i) {
        waveTypeSelector[i].addItem("Sine", 1);
        waveTypeSelector[i].addItem("Square", 2);
        waveTypeSelector[i].addItem("Triangle", 3);
        waveTypeSelector[i].addItem("Saw", 4);
        waveTypeSelector[i].onChange = [this, i] {
            processor.getOscillator(i)->setWaveType(static_cast<Oscillator::WaveType>(waveTypeSelector[i].getSelectedId() - 1));
        };
        waveTypeSelector[i].setSelectedId(1);
        addAndMakeVisible(waveTypeSelector[i]);

        octaveDials[i].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        octaveDials[i].setRange(-4, 4, 1);
        octaveDials[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
        octaveDials[i].onValueChange = [this, i] {
            processor.setOscillatorOctave(i, octaveDials[i].getValue());
        };
        addAndMakeVisible(octaveDials[i]);

        // Detune Dial
        detuneDials[i].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        detuneDials[i].setRange(-50, 50, 1);
        detuneDials[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
        detuneDials[i].onValueChange = [this, i] {
            processor.setOscillatorDetune(i, detuneDials[i].getValue());
        };
        addAndMakeVisible(detuneDials[i]);

        for (int j = 0; j < 4; ++j) {
            if (i == j) continue;
            enableModulation[i][j].onClick = [this, i, j] {
                bool isEnabled = enableModulation[i][j].getToggleState();
                if (!isEnabled) {
                    processor.setModulationDepth(i, j, -1);
                }
                else {
                    isEnabled = processor.setModulationDepth(i, j, modulationDepthDials[i][j].getValue());
                }
                enableModulation[i][j].setToggleState(isEnabled, juce::dontSendNotification);
                modulationDepthDials[i][j].setEnabled(isEnabled);
            };
            addAndMakeVisible(enableModulation[i][j]);

            modulationDepthDials[i][j].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
            modulationDepthDials[i][j].setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
            modulationDepthDials[i][j].setRange(0, 10000, 1);
            modulationDepthDials[i][j].setNumDecimalPlacesToDisplay(0);
            modulationDepthDials[i][j].setTextValueSuffix("");
            modulationDepthDials[i][j].setEnabled(false);


            modulationDepthDials[i][j].onValueChange = [this, i, j] {
                if (enableModulation[i][j].getToggleState()) {
                    processor.setModulationDepth(i, j, modulationDepthDials[i][j].getValue());
                }
                };
            addAndMakeVisible(modulationDepthDials[i][j]);
        }
    }

    for (int i = 0; i < 4; ++i) {
        attackSliders[i].setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        decaySliders[i].setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        sustainSliders[i].setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        releaseSliders[i].setSliderStyle(juce::Slider::SliderStyle::LinearVertical);

        attackSliders[i].setRange(0.01, 5.0);
        decaySliders[i].setRange(0.01, 5.0);
        sustainSliders[i].setRange(0.0, 1.0);
        releaseSliders[i].setRange(0.01, 5.0);

        attackSliders[i].onValueChange = [this, i] {
            processor.setOscillatorAttack(i, attackSliders[i].getValue());
        };
        decaySliders[i].onValueChange = [this, i] {
            processor.setOscillatorDecay(i, decaySliders[i].getValue());
        };
        sustainSliders[i].onValueChange = [this, i] {
            processor.setOscillatorSustain(i, sustainSliders[i].getValue());
        };
        releaseSliders[i].onValueChange = [this, i] {
            processor.setOscillatorRelease(i, releaseSliders[i].getValue());
        };

        addAndMakeVisible(attackSliders[i]);
        addAndMakeVisible(decaySliders[i]);
        addAndMakeVisible(sustainSliders[i]);
        addAndMakeVisible(releaseSliders[i]);
    }

    levelSlider.setSliderStyle(juce::Slider::LinearVertical);
    levelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    levelSlider.setRange(0.0, 1.0);
    levelSlider.setNumDecimalPlacesToDisplay(2);
    levelSlider.setTextValueSuffix("");
    levelSlider.onValueChange = [this] {
        processor.setLevel(levelSlider.getValue());
    };

    addAndMakeVisible(levelSlider);

    levelLabel.setText("Level", juce::dontSendNotification);
    levelLabel.attachToComponent(&levelSlider, false);
    addAndMakeVisible(levelLabel);

    for (int i = 0; i < 4; ++i) {
        attackLabels[i].setText("Attack", juce::dontSendNotification);
        decayLabels[i].setText("Decay", juce::dontSendNotification);
        sustainLabels[i].setText("Sustain", juce::dontSendNotification);
        releaseLabels[i].setText("Release", juce::dontSendNotification);

        addAndMakeVisible(attackLabels[i]);
        addAndMakeVisible(decayLabels[i]);
        addAndMakeVisible(sustainLabels[i]);
        addAndMakeVisible(releaseLabels[i]);
    }

    for (int i = 0; i < 4; ++i) {
        levelDials[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
        levelDials[i].setRange(0.0, 1.0, 0.01);
        levelDials[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        levelDials[i].setNumDecimalPlacesToDisplay(2);
        levelDials[i].setTextValueSuffix("");
        addAndMakeVisible(levelDials[i]);

        levelDials[i].onValueChange = [this, i] {
            processor.setOscillatorLevel(i, levelDials[i].getValue());
        };
    }

    addAndMakeVisible(synthButton);
    addAndMakeVisible(fxButton);
    addAndMakeVisible(processor.fxList);

    synthButton.onClick = [this] {showSynthInterface(); };
    fxButton.onClick = [this] {showFxInterface(); };
    showSynthInterface();

    setSize(1000, 600);
}

void SynthFMAudioProcessorEditor::showSynthInterface() {
    isSynth = true;
    for (auto& selector : waveTypeSelector) selector.setVisible(true);
    for (auto& dial : octaveDials) dial.setVisible(true);
    for (auto& dial : detuneDials) dial.setVisible(true);
    for (auto& slider : attackSliders) slider.setVisible(true);
    for (auto& slider : decaySliders) slider.setVisible(true);
    for (auto& slider : sustainSliders) slider.setVisible(true);
    for (auto& slider : releaseSliders) slider.setVisible(true);
    for (auto& label : attackLabels) label.setVisible(true);
    for (auto& label : decayLabels) label.setVisible(true);
    for (auto& label : sustainLabels) label.setVisible(true);
    for (auto& label : releaseLabels) label.setVisible(true);
    for (auto& dial : levelDials) dial.setVisible(true);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            modulationDepthDials[i][j].setVisible(true);
            enableModulation[i][j].setVisible(true);
        }
    }
    levelLabel.setVisible(true);
    levelSlider.setVisible(true);
    processor.fxList.setVisible(false);

    repaint();
}

void SynthFMAudioProcessorEditor::showFxInterface() {
    isSynth = false;
    for (auto& selector : waveTypeSelector) selector.setVisible(false);
    for (auto& dial : octaveDials) dial.setVisible(false);
    for (auto& dial : detuneDials) dial.setVisible(false);
    for (auto& slider : attackSliders) slider.setVisible(false);
    for (auto& slider : decaySliders) slider.setVisible(false);
    for (auto& slider : sustainSliders) slider.setVisible(false);
    for (auto& slider : releaseSliders) slider.setVisible(false);
    for (auto& label : attackLabels) label.setVisible(false);
    for (auto& label : decayLabels) label.setVisible(false);
    for (auto& label : sustainLabels) label.setVisible(false);
    for (auto& label : releaseLabels) label.setVisible(false);
    for (auto& dial : levelDials) dial.setVisible(false);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            modulationDepthDials[i][j].setVisible(false);
            enableModulation[i][j].setVisible(false);
        }
    }
    levelLabel.setVisible(false);
    levelSlider.setVisible(false);
    processor.fxList.setVisible(true);

    repaint();
}

void SynthFMAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    for (int i = 0; i < 4; ++i)
    {
        if (comboBoxThatHasChanged == &waveTypeSelector[i])
        {
            processor.getOscillator(i)->setWaveType(static_cast<Oscillator::WaveType>(waveTypeSelector[i].getSelectedId() - 1));
            return;
        }
    }
}


SynthFMAudioProcessorEditor::~SynthFMAudioProcessorEditor()
{
}

//==============================================================================
void SynthFMAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    if (isSynth) {
        g.setColour(juce::Colours::lightgrey);

        int xOffset = 10;
        int yOffset = 40;
        int width = (getWidth() - xOffset * 2 - 50) / 6;
        int cellHeight = (getHeight() - yOffset - 100) / 4;
        int dialDiameter = juce::jmin(width, cellHeight) - 70;

        for (int i = 0; i <= 6; ++i) {
            if (i == 1) continue;
            int xPosition = xOffset + i * width;
            g.drawLine(xPosition, yOffset, xOffset + width * i, cellHeight * 4 + yOffset, 1);
        }

        for (int j = 0; j <= 4; ++j) {
            int yPosition = yOffset + j * cellHeight;
            if (j == 0) {
                g.drawLine(xOffset, yPosition, getWidth(), yPosition, 1);
            }
            else {
                g.drawLine(xOffset, yPosition, getWidth() - xOffset - 50, yPosition, 1);
            }
        }

        g.setColour(juce::Colours::white);

        juce::Font labelFont(12.0f);
        g.setFont(labelFont);

        int labelXOffset = 15;
        int labelYOffset = 20;

        for (int i = 0; i < 4; ++i) {
            int octaveX = labelXOffset;
            int octaveY = yOffset + i * cellHeight + labelYOffset;
            int detuneX = 85;
            int detuneY = yOffset + i * cellHeight + labelYOffset;
            g.drawFittedText("Octave", octaveX, octaveY, dialDiameter, labelYOffset, juce::Justification::centred, 1);
            g.drawFittedText("Detune", detuneX, detuneY, dialDiameter, labelYOffset, juce::Justification::centred, 1);
        }


        for (int i = 2; i < 6; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                if (i - 2 != j)
                {
                    int textX = xOffset + i * width + 20;
                    int textY = yOffset + j * cellHeight;


                    juce::String labelString = juce::String(i - 1) + " -> " + juce::String(j + 1);
                    g.drawFittedText(labelString, textX, textY, dialDiameter, 20, juce::Justification::centred, 1);
                }
                else {
                    int textX = xOffset + i * width + 2;
                    int textY = yOffset + j * cellHeight + 12;

                    juce::String labelString = "Operator " + juce::String(j + 1) + " level";

                    g.drawSingleLineText(labelString, textX, textY);
                }
            }
        }
    }
    else {
    
    }
}





void SynthFMAudioProcessorEditor::resized()
{
    int xOffset = 10;
    int yOffset = 40;
    int width = (getWidth() - xOffset * 2 - 50) / 6;
    int comboBoxHeight = 30;
    int cellHeight = (getHeight() - yOffset - 100) / 4;
    int dialDiameter = juce::jmin(width, cellHeight) - 50;

    int textBoxWidth = 40;
    int textBoxHeight = 15;

    keyboardComponent.setBounds(0, getHeight() - 100, getWidth(), 100);

    int levelSliderWidth = 60;
    int levelSliderX = getWidth() - levelSliderWidth;
    int levelSliderY = yOffset + 40;
    int levelSliderHeight = getHeight() - yOffset - 100 - 40;

    levelSlider.setBounds(levelSliderX, levelSliderY, levelSliderWidth, levelSliderHeight);
    levelLabel.setBounds(levelSliderX + 5, levelSliderY - 30, levelSliderWidth, 20);

    int leftColumnWidth = width * 2;
    for (int i = 0; i < 4; ++i) {
        int yPosition = yOffset + i * cellHeight;
        octaveDials[i].setBounds(10, yPosition + 30, dialDiameter - 10, dialDiameter + 10);
        octaveDials[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
        detuneDials[i].setBounds(80, yPosition + 30, dialDiameter - 10, dialDiameter + 10);
        detuneDials[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
        waveTypeSelector[i].setBounds(10, yPosition, leftColumnWidth, 20);

        int sliderWidth = 20;
        int sliderHeight = cellHeight - 35;
        attackSliders[i].setBounds(150, yPosition + 30, sliderWidth, sliderHeight);
        decaySliders[i].setBounds(150 + (sliderWidth + 25), yPosition + 30, sliderWidth, sliderHeight);
        sustainSliders[i].setBounds(150 + 2 * (sliderWidth + 25), yPosition + 30, sliderWidth, sliderHeight);
        releaseSliders[i].setBounds(150 + 3 * (sliderWidth + 25), yPosition + 30, sliderWidth, sliderHeight);

        int labelWidth = 50;
        int labelHeight = 20;
        juce::Font labelFont(12.0f);
        attackLabels[i].setFont(labelFont);
        decayLabels[i].setFont(labelFont);
        sustainLabels[i].setFont(labelFont);
        releaseLabels[i].setFont(labelFont);

        attackLabels[i].setBounds(140, yPosition + 20, labelWidth, labelHeight);
        decayLabels[i].setBounds(140 + (sliderWidth + 25), yPosition + 20, labelWidth, labelHeight);
        sustainLabels[i].setBounds(140 + 2 * (sliderWidth + 25), yPosition + 20, labelWidth, labelHeight);
        releaseLabels[i].setBounds(140 + 3 * (sliderWidth + 25), yPosition + 20, labelWidth, labelHeight);
    }

    for (int i = 2; i < 6; ++i) {
        int elementYOffset = yOffset + 30 + 10;

        for (int j = 0; j < 4; ++j) {
            int cellYPosition = yOffset + j * cellHeight;
            if (i - 2 == j) {
                int levelDialX = xOffset + i * width + (width - dialDiameter) / 2;
                int levelDialY = cellYPosition + (cellHeight - dialDiameter - 20) / 2;
                levelDials[j].setBounds(levelDialX, levelDialY, dialDiameter, dialDiameter + 20);
                levelDials[j].setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
            }
            
            enableModulation[i - 2][j].setBounds(xOffset + i * width, cellYPosition, 50, 20);

            int dialXPosition = xOffset + i * width + (width - dialDiameter) / 2;
            int dialYPosition = cellYPosition + (cellHeight - dialDiameter - 20) / 2;
            modulationDepthDials[i - 2][j].setBounds(dialXPosition, dialYPosition, dialDiameter, dialDiameter + 20);

            modulationDepthDials[i - 2][j].setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);

        }
    }

    int buttonWidth = getWidth() / 2;
    synthButton.setBounds(0, 5, buttonWidth, 30);
    fxButton.setBounds(buttonWidth, 5, buttonWidth, 30);
    processor.fxList.setBounds(0, 43, getWidth(), getHeight() - 144);
}

