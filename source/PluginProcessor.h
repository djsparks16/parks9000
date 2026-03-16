#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

#include "SynthEngine.h"

class FiveParksVST3AudioProcessor : public juce::AudioProcessor
{
public:
    FiveParksVST3AudioProcessor();
    ~FiveParksVST3AudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    float getMeterLevel() const noexcept { return meterLevel.load(); }
    void copyScopeData(std::array<float, 512>& dest) const noexcept;
    void copyAnalyzerData(std::array<float, 128>& dest) const noexcept;
    void copyLfoShape(std::array<float, 128>& dest) const noexcept;
    juce::String getMatrixSummary() const;

private:
    static constexpr int maxVoices = 16;
    std::array<FiveParksVoice, maxVoices> voices {};
    uint64_t voiceCounter = 0;

    std::atomic<float> meterLevel { 0.0f };
    std::array<float, 2048> scopeRing {};
    std::atomic<int> scopeWritePos { 0 };

    std::array<float, 1024> fftFifo {};
    int fftFifoIndex = 0;
    juce::dsp::FFT fft { 10 };
    juce::dsp::WindowingFunction<float> window { 1024, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, 2048> fftData {};
    std::array<float, 128> analyzerBins {};
    std::array<float, 128> previewLfoShape {};

    std::vector<float> chorusDelayL, chorusDelayR, delayLineL, delayLineR;
    std::vector<float> wetBufferL, wetBufferR;
    int chorusWrite = 0, delayWrite = 0;
    float chorusPhase = 0.0f;
    float compEnv = 0.0f;
    double currentSampleRate = 44100.0;

    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams {};

    void noteOn(int midiNote, float velocity);
    void noteOff(int midiNote);
    int findVoiceForNote(int midiNote) const;
    int findFreeVoice() const;
    int stealVoice() const;

    void pushScopeSample(float sample) noexcept;
    void pushAnalyzerSample(float sample) noexcept;
    void computeAnalyzerFrame() noexcept;
    void updatePreviewLfo() noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FiveParksVST3AudioProcessor)
};
