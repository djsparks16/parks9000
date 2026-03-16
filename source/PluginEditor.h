#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <vector>

#include "PluginProcessor.h"

class ScopeDisplay : public juce::Component, private juce::Timer
{
public:
    explicit ScopeDisplay(FiveParksVST3AudioProcessor& p) : processor(p) { startTimerHz(30); }
    void paint(juce::Graphics& g) override;
private:
    void timerCallback() override;
    FiveParksVST3AudioProcessor& processor;
    std::array<float, 512> data {};
};

class SpectrumDisplay : public juce::Component, private juce::Timer
{
public:
    explicit SpectrumDisplay(FiveParksVST3AudioProcessor& p) : processor(p) { startTimerHz(20); }
    void paint(juce::Graphics& g) override;
private:
    void timerCallback() override;
    FiveParksVST3AudioProcessor& processor;
    std::array<float, 128> data {};
};

class LfoDisplay : public juce::Component, private juce::Timer
{
public:
    explicit LfoDisplay(FiveParksVST3AudioProcessor& p) : processor(p) { startTimerHz(20); }
    void paint(juce::Graphics& g) override;
private:
    void timerCallback() override;
    FiveParksVST3AudioProcessor& processor;
    std::array<float, 128> data {};
};

class MatrixDisplay : public juce::Component, private juce::Timer
{
public:
    explicit MatrixDisplay(FiveParksVST3AudioProcessor& p) : processor(p) { startTimerHz(8); }
    void paint(juce::Graphics& g) override;
private:
    void timerCallback() override;
    FiveParksVST3AudioProcessor& processor;
    juce::String summary;
};

class SerumKnob : public juce::Component
{
public:
    SerumKnob();
    void resized() override;
    juce::Slider slider;
    juce::Label title;
};

class FiveParksVST3AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit FiveParksVST3AudioProcessorEditor(FiveParksVST3AudioProcessor&);
    ~FiveParksVST3AudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void configureKnob(SerumKnob& knob, const juce::String& title);
    void attach(juce::Component& parent, SerumKnob& knob, const juce::String& id, const juce::String& title);
    void styleCombo(juce::ComboBox& box);
    void addMatrixSlot(int row, juce::ComboBox& src, juce::ComboBox& dst, SerumKnob& amt, const juce::String& srcId, const juce::String& dstId, const juce::String& amtId);

    FiveParksVST3AudioProcessor& audioProcessor;

    juce::Label logo, subtitle;
    juce::Component oscABox, oscBBox, subBox, noiseBox, filterBox, envBox, lfoBox, matrixBox, fxBox, perfBox, displayBox;

    ScopeDisplay scope;
    SpectrumDisplay spectrum;
    LfoDisplay lfoShape;
    MatrixDisplay matrixView;

    SerumKnob oscAPos, oscALevel, oscADetune, oscABlend, oscAPan;
    SerumKnob oscBPos, oscBLevel, oscBDetune, oscBBlend, oscBPan;
    SerumKnob subLevel, noiseLevel, filterCutoff, filterRes, filterDrive, filterMix;
    SerumKnob env1A, env1D, env1S, env1R;
    SerumKnob lfo1Rate, lfo1Amt, lfo1Shape, lfo1Smooth;
    SerumKnob distDrive, distMix, chorusMix, delayMix, reverbMix, compAmt;
    SerumKnob glide, masterGain;
    SerumKnob mod1Amt, mod2Amt, mod3Amt, mod4Amt, mod5Amt, mod6Amt;

    juce::ComboBox oscAOct, oscASemi, oscAUnison;
    juce::ComboBox oscBOct, oscBSemi, oscBUnison;
    juce::ComboBox subWave, filterMode, playMode, polyphony;
    juce::ComboBox mod1Src, mod1Dst, mod2Src, mod2Dst, mod3Src, mod3Dst, mod4Src, mod4Dst, mod5Src, mod5Dst, mod6Src, mod6Dst;
    juce::ToggleButton subDirect, noiseDirect;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAtts;
    std::vector<std::unique_ptr<ComboAttachment>> comboAtts;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAtts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FiveParksVST3AudioProcessorEditor)
};
