#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    using APF = juce::AudioParameterFloat;
    using API = juce::AudioParameterInt;
    using APC = juce::AudioParameterChoice;
    using APB = juce::AudioParameterBool;

    std::unique_ptr<juce::RangedAudioParameter> pf(const juce::String& id, const juce::String& name, float mn, float mx, float def, float skew = 1.0f)
    {
        return std::make_unique<APF>(id, name, juce::NormalisableRange<float>(mn, mx, 0.0f, skew), def);
    }
    std::unique_ptr<juce::RangedAudioParameter> pi(const juce::String& id, const juce::String& name, int mn, int mx, int def)
    {
        return std::make_unique<API>(id, name, mn, mx, def);
    }
    std::unique_ptr<juce::RangedAudioParameter> pc(const juce::String& id, const juce::String& name, const juce::StringArray& items, int def)
    {
        return std::make_unique<APC>(id, name, items, def);
    }
    std::unique_ptr<juce::RangedAudioParameter> pb(const juce::String& id, const juce::String& name, bool def)
    {
        return std::make_unique<APB>(id, name, def);
    }

    float getF(juce::AudioProcessorValueTreeState& s, const char* id) { return s.getRawParameterValue(id)->load(); }
    float getF(juce::AudioProcessorValueTreeState& s, const juce::String& id) { return s.getRawParameterValue(id)->load(); }
    int getI(juce::AudioProcessorValueTreeState& s, const char* id) { return (int) std::round(s.getRawParameterValue(id)->load()); }
    int getI(juce::AudioProcessorValueTreeState& s, const juce::String& id) { return (int) std::round(s.getRawParameterValue(id)->load()); }
    bool getB(juce::AudioProcessorValueTreeState& s, const char* id) { return s.getRawParameterValue(id)->load() > 0.5f; }
}

FiveParksVST3AudioProcessor::FiveParksVST3AudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameters())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout FiveParksVST3AudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(pc("play_mode", "Play Mode", { "Mono", "Legato", "Poly" }, 2));
    p.push_back(pf("glide_ms", "Glide", 0.0f, 500.0f, 35.0f, 0.35f));
    p.push_back(pc("polyphony", "Polyphony", { "1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16" }, 7));
    p.push_back(pf("master_gain", "Master Gain", 0.0f, 1.0f, 0.72f));

    p.push_back(pc("oscA_oct", "OSC A Oct", { "-4","-3","-2","-1","0","1","2","3","4" }, 4));
    p.push_back(pc("oscA_semi", "OSC A Semi", { "-12","-11","-10","-9","-8","-7","-6","-5","-4","-3","-2","-1","0","1","2","3","4","5","6","7","8","9","10","11","12" }, 12));
    p.push_back(pf("oscA_fine", "OSC A Fine", -100.0f, 100.0f, 0.0f));
    p.push_back(pc("oscA_unison", "OSC A Unison", { "1","2","3","4","5" }, 2));
    p.push_back(pf("oscA_detune", "OSC A Detune", 0.0f, 1.0f, 0.18f));
    p.push_back(pf("oscA_blend", "OSC A Blend", 0.0f, 1.0f, 0.76f));
    p.push_back(pf("oscA_phase", "OSC A Phase", 0.0f, 1.0f, 0.0f));
    p.push_back(pf("oscA_rand", "OSC A Rand", 0.0f, 1.0f, 0.0f));
    p.push_back(pf("oscA_pos", "OSC A WT Pos", 0.0f, 1.0f, 0.25f));
    p.push_back(pf("oscA_level", "OSC A Level", 0.0f, 1.0f, 0.85f));
    p.push_back(pf("oscA_pan", "OSC A Pan", 0.0f, 1.0f, 0.5f));

    p.push_back(pc("oscB_oct", "OSC B Oct", { "-4","-3","-2","-1","0","1","2","3","4" }, 4));
    p.push_back(pc("oscB_semi", "OSC B Semi", { "-12","-11","-10","-9","-8","-7","-6","-5","-4","-3","-2","-1","0","1","2","3","4","5","6","7","8","9","10","11","12" }, 19));
    p.push_back(pf("oscB_fine", "OSC B Fine", -100.0f, 100.0f, 0.0f));
    p.push_back(pc("oscB_unison", "OSC B Unison", { "1","2","3","4","5" }, 1));
    p.push_back(pf("oscB_detune", "OSC B Detune", 0.0f, 1.0f, 0.14f));
    p.push_back(pf("oscB_blend", "OSC B Blend", 0.0f, 1.0f, 0.68f));
    p.push_back(pf("oscB_phase", "OSC B Phase", 0.0f, 1.0f, 0.0f));
    p.push_back(pf("oscB_rand", "OSC B Rand", 0.0f, 1.0f, 0.0f));
    p.push_back(pf("oscB_pos", "OSC B WT Pos", 0.0f, 1.0f, 0.58f));
    p.push_back(pf("oscB_level", "OSC B Level", 0.0f, 1.0f, 0.70f));
    p.push_back(pf("oscB_pan", "OSC B Pan", 0.0f, 1.0f, 0.5f));

    p.push_back(pc("sub_wave", "Sub Wave", { "Sine","Tri","Saw","Square" }, 0));
    p.push_back(pf("sub_level", "Sub Level", 0.0f, 1.0f, 0.7f));
    p.push_back(pb("sub_direct", "Sub Direct", false));

    p.push_back(pf("noise_pitch", "Noise Pitch", 0.0f, 1.0f, 0.5f));
    p.push_back(pf("noise_phase", "Noise Phase", 0.0f, 1.0f, 0.0f));
    p.push_back(pf("noise_pan", "Noise Pan", 0.0f, 1.0f, 0.5f));
    p.push_back(pf("noise_level", "Noise Level", 0.0f, 1.0f, 0.0f));
    p.push_back(pb("noise_direct", "Noise Direct", false));

    p.push_back(pc("filter_mode", "Filter Mode", { "LP", "BP", "HP" }, 0));
    p.push_back(pf("filter_cutoff", "Filter Cutoff", 20.0f, 18000.0f, 1200.0f, 0.30f));
    p.push_back(pf("filter_res", "Filter Res", 0.1f, 1.15f, 0.25f));
    p.push_back(pf("filter_drive", "Filter Drive", 0.0f, 1.0f, 0.2f));
    p.push_back(pf("filter_mix", "Filter Mix", 0.0f, 1.0f, 1.0f));
    p.push_back(pf("filter_pan", "Filter Pan", 0.0f, 1.0f, 0.5f));

    for (int e = 1; e <= 3; ++e)
    {
        p.push_back(pf("env" + juce::String(e) + "_a", "Env A", 0.001f, 5.0f, e == 1 ? 0.005f : 0.001f, 0.35f));
        p.push_back(pf("env" + juce::String(e) + "_h", "Env H", 0.0f, 2.0f, 0.0f, 0.35f));
        p.push_back(pf("env" + juce::String(e) + "_d", "Env D", 0.001f, 5.0f, e == 1 ? 0.25f : (e == 2 ? 0.30f : 0.40f), 0.35f));
        p.push_back(pf("env" + juce::String(e) + "_s", "Env S", 0.0f, 1.0f, e == 1 ? 0.75f : 0.0f));
        p.push_back(pf("env" + juce::String(e) + "_r", "Env R", 0.001f, 5.0f, 0.2f, 0.35f));
    }

    for (int i = 1; i <= 4; ++i)
    {
        p.push_back(pf("lfo" + juce::String(i) + "_rate", "LFO Rate", 0.01f, 20.0f, i == 1 ? 1.2f : (i == 2 ? 0.35f : (i == 3 ? 3.0f : 0.12f)), 0.35f));
        p.push_back(pf("lfo" + juce::String(i) + "_amt", "LFO Amt", 0.0f, 1.0f, i == 1 ? 0.15f : 0.0f));
        p.push_back(pf("lfo" + juce::String(i) + "_shape", "LFO Shape", 0.0f, 1.0f, (float) (i - 1) / 3.0f));
        p.push_back(pf("lfo" + juce::String(i) + "_smooth", "LFO Smooth", 0.0f, 1.0f, 0.15f));
        p.push_back(pc("lfo" + juce::String(i) + "_sync", "LFO Sync", { "Free", "Sync" }, 0));
    }

    const juce::StringArray sources { "None", "ENV2", "ENV3", "LFO1", "LFO2", "LFO3", "LFO4", "Velocity" };
    const juce::StringArray dests { "None", "Filt Cutoff", "OSC A Pos", "OSC B Pos", "Pitch", "Level", "Pan", "Dist", "FX" };
    for (int i = 1; i <= 6; ++i)
    {
        p.push_back(pc("mod" + juce::String(i) + "_src", "Mod Src", sources, 0));
        p.push_back(pc("mod" + juce::String(i) + "_dst", "Mod Dst", dests, 0));
        p.push_back(pf("mod" + juce::String(i) + "_amt", "Mod Amt", -1.0f, 1.0f, 0.0f));
    }

    p.push_back(pf("dist_drive", "Dist Drive", 0.0f, 1.0f, 0.25f));
    p.push_back(pf("dist_mix", "Dist Mix", 0.0f, 1.0f, 0.40f));
    p.push_back(pf("chorus_mix", "Chorus Mix", 0.0f, 1.0f, 0.15f));
    p.push_back(pf("delay_mix", "Delay Mix", 0.0f, 1.0f, 0.12f));
    p.push_back(pf("reverb_mix", "Reverb Mix", 0.0f, 1.0f, 0.08f));
    p.push_back(pf("comp_amt", "Comp Amt", 0.0f, 1.0f, 0.20f));

    return { p.begin(), p.end() };
}

void FiveParksVST3AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    for (auto& v : voices)
        v.prepare(sampleRate, samplesPerBlock);

    chorusDelayL.assign((size_t) (sampleRate * 0.05), 0.0f);
    chorusDelayR.assign((size_t) (sampleRate * 0.05), 0.0f);
    delayLineL.assign((size_t) (sampleRate * 0.75), 0.0f);
    delayLineR.assign((size_t) (sampleRate * 0.75), 0.0f);
    wetBufferL.assign((size_t) juce::jmax(1, samplesPerBlock), 0.0f);
    wetBufferR.assign((size_t) juce::jmax(1, samplesPerBlock), 0.0f);
    chorusWrite = delayWrite = 0;
    chorusPhase = compEnv = 0.0f;

    reverb.reset();
    reverb.setSampleRate(sampleRate);
    reverbParams.roomSize = 0.62f;
    reverbParams.damping = 0.38f;
    reverbParams.width = 0.9f;
    reverbParams.freezeMode = 0.0f;
    reverbParams.wetLevel = 0.0f;
    reverbParams.dryLevel = 1.0f;
    reverb.setParameters(reverbParams);
}

void FiveParksVST3AudioProcessor::releaseResources() {}

bool FiveParksVST3AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void FiveParksVST3AudioProcessor::noteOn(int midiNote, float velocity)
{
    RuntimeParams p;
    p.env1A = getF(apvts, "env1_a"); p.env1D = getF(apvts, "env1_d"); p.env1S = getF(apvts, "env1_s"); p.env1R = getF(apvts, "env1_r");
    p.env2A = getF(apvts, "env2_a"); p.env2D = getF(apvts, "env2_d"); p.env2S = getF(apvts, "env2_s"); p.env2R = getF(apvts, "env2_r");
    p.env3A = getF(apvts, "env3_a"); p.env3D = getF(apvts, "env3_d"); p.env3S = getF(apvts, "env3_s"); p.env3R = getF(apvts, "env3_r");
    p.oscAPhase = getF(apvts, "oscA_phase"); p.oscARand = getF(apvts, "oscA_rand");
    p.oscBPhase = getF(apvts, "oscB_phase"); p.oscBRand = getF(apvts, "oscB_rand");
    p.playMode = getI(apvts, "play_mode");

    const int mode = p.playMode;
    if (mode != 2)
    {
        int idx = 0;
        for (int i = 0; i < maxVoices; ++i)
            if (voices[(size_t) i].isActive()) { idx = i; break; }
        voices[(size_t) idx].start(midiNote, velocity, p, ++voiceCounter);
        return;
    }

    int idx = findFreeVoice();
    if (idx < 0) idx = stealVoice();
    voices[(size_t) idx].start(midiNote, velocity, p, ++voiceCounter);
}

void FiveParksVST3AudioProcessor::noteOff(int midiNote)
{
    const int idx = findVoiceForNote(midiNote);
    if (idx >= 0)
        voices[(size_t) idx].stop();
}

int FiveParksVST3AudioProcessor::findVoiceForNote(int midiNote) const
{
    for (int i = 0; i < maxVoices; ++i)
        if (voices[(size_t) i].isActive() && voices[(size_t) i].getMidiNote() == midiNote)
            return i;
    return -1;
}

int FiveParksVST3AudioProcessor::findFreeVoice() const
{
    for (int i = 0; i < juce::jmin(maxVoices, getI(const_cast<juce::AudioProcessorValueTreeState&>(apvts), "polyphony") + 1); ++i)
        if (! voices[(size_t) i].isActive())
            return i;
    return -1;
}

int FiveParksVST3AudioProcessor::stealVoice() const
{
    uint64_t oldest = std::numeric_limits<uint64_t>::max();
    int idx = 0;
    for (int i = 0; i < juce::jmin(maxVoices, getI(const_cast<juce::AudioProcessorValueTreeState&>(apvts), "polyphony") + 1); ++i)
    {
        if (voices[(size_t) i].getAge() < oldest)
        {
            oldest = voices[(size_t) i].getAge();
            idx = i;
        }
    }
    return idx;
}

void FiveParksVST3AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    RuntimeParams p;
    p.playMode = getI(apvts, "play_mode");
    p.glideMs = getF(apvts, "glide_ms");
    p.polyphony = getI(apvts, "polyphony") + 1;
    p.masterGain = getF(apvts, "master_gain");

    p.oscAOct = getI(apvts, "oscA_oct") - 4; p.oscASemi = getI(apvts, "oscA_semi") - 12; p.oscAFine = getF(apvts, "oscA_fine");
    p.oscAUnison = (float) (getI(apvts, "oscA_unison") + 1); p.oscADetune = getF(apvts, "oscA_detune"); p.oscABlend = getF(apvts, "oscA_blend");
    p.oscAPhase = getF(apvts, "oscA_phase"); p.oscARand = getF(apvts, "oscA_rand"); p.oscAPos = getF(apvts, "oscA_pos"); p.oscALevel = getF(apvts, "oscA_level"); p.oscAPan = getF(apvts, "oscA_pan");
    p.oscBOct = getI(apvts, "oscB_oct") - 4; p.oscBSemi = getI(apvts, "oscB_semi") - 12; p.oscBFine = getF(apvts, "oscB_fine");
    p.oscBUnison = (float) (getI(apvts, "oscB_unison") + 1); p.oscBDetune = getF(apvts, "oscB_detune"); p.oscBBlend = getF(apvts, "oscB_blend");
    p.oscBPhase = getF(apvts, "oscB_phase"); p.oscBRand = getF(apvts, "oscB_rand"); p.oscBPos = getF(apvts, "oscB_pos"); p.oscBLevel = getF(apvts, "oscB_level"); p.oscBPan = getF(apvts, "oscB_pan");
    p.subWave = getI(apvts, "sub_wave"); p.subLevel = getF(apvts, "sub_level"); p.subDirect = getB(apvts, "sub_direct");
    p.noisePitch = getF(apvts, "noise_pitch"); p.noisePhase = getF(apvts, "noise_phase"); p.noisePan = getF(apvts, "noise_pan"); p.noiseLevel = getF(apvts, "noise_level"); p.noiseDirect = getB(apvts, "noise_direct");
    p.filterMode = getI(apvts, "filter_mode"); p.cutoff = getF(apvts, "filter_cutoff"); p.resonance = getF(apvts, "filter_res"); p.filterDrive = getF(apvts, "filter_drive"); p.filterMix = getF(apvts, "filter_mix"); p.filterPan = getF(apvts, "filter_pan");

    p.env1A = getF(apvts, "env1_a"); p.env1H = getF(apvts, "env1_h"); p.env1D = getF(apvts, "env1_d"); p.env1S = getF(apvts, "env1_s"); p.env1R = getF(apvts, "env1_r");
    p.env2A = getF(apvts, "env2_a"); p.env2H = getF(apvts, "env2_h"); p.env2D = getF(apvts, "env2_d"); p.env2S = getF(apvts, "env2_s"); p.env2R = getF(apvts, "env2_r");
    p.env3A = getF(apvts, "env3_a"); p.env3H = getF(apvts, "env3_h"); p.env3D = getF(apvts, "env3_d"); p.env3S = getF(apvts, "env3_s"); p.env3R = getF(apvts, "env3_r");

    for (int i = 1; i <= 4; ++i)
    {
        p.lfoRate[(size_t) (i - 1)] = getF(apvts, ("lfo" + juce::String(i) + "_rate"));
        p.lfoAmount[(size_t) (i - 1)] = getF(apvts, ("lfo" + juce::String(i) + "_amt"));
        p.lfoShape[(size_t) (i - 1)] = getF(apvts, ("lfo" + juce::String(i) + "_shape"));
        p.lfoSmooth[(size_t) (i - 1)] = getF(apvts, ("lfo" + juce::String(i) + "_smooth"));
        p.lfoSync[(size_t) (i - 1)] = getI(apvts, ("lfo" + juce::String(i) + "_sync"));
    }
    for (int i = 1; i <= 6; ++i)
    {
        p.matrix[(size_t) (i - 1)].source = (ModSource) getI(apvts, ("mod" + juce::String(i) + "_src"));
        p.matrix[(size_t) (i - 1)].destination = (ModDestination) getI(apvts, ("mod" + juce::String(i) + "_dst"));
        p.matrix[(size_t) (i - 1)].amount = getF(apvts, ("mod" + juce::String(i) + "_amt"));
    }
    p.distDrive = getF(apvts, "dist_drive"); p.distMix = getF(apvts, "dist_mix"); p.chorusMix = getF(apvts, "chorus_mix"); p.delayMix = getF(apvts, "delay_mix"); p.reverbMix = getF(apvts, "reverb_mix"); p.compAmt = getF(apvts, "comp_amt");

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff()) noteOff(msg.getNoteNumber());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff()) for (auto& v : voices) if (v.isActive()) v.stop();
    }

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    const int numSamples = buffer.getNumSamples();

    if ((int) wetBufferL.size() < numSamples)
    {
        wetBufferL.resize((size_t) numSamples, 0.0f);
        wetBufferR.resize((size_t) numSamples, 0.0f);
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float l = 0.0f, r = 0.0f;
        for (int i = 0; i < juce::jmin(maxVoices, p.polyphony); ++i)
        {
            const auto out = voices[(size_t) i].render(p);
            l += out.left;
            r += out.right;
        }

        chorusPhase += 0.18f / (float) currentSampleRate;
        if (chorusPhase >= 1.0f) chorusPhase -= 1.0f;
        const float chorusMod = 0.003f + 0.002f * std::sin(chorusPhase * juce::MathConstants<float>::twoPi);
        const int chorusDelaySamps = (int) juce::jlimit(1.0, (double) chorusDelayL.size() - 2.0, chorusMod * currentSampleRate);
        const int chorusRead = (chorusWrite - chorusDelaySamps + (int) chorusDelayL.size()) % (int) chorusDelayL.size();
        const float chorusL = chorusDelayL[(size_t) chorusRead];
        const float chorusR = chorusDelayR[(size_t) chorusRead];
        chorusDelayL[(size_t) chorusWrite] = l;
        chorusDelayR[(size_t) chorusWrite] = r;
        chorusWrite = (chorusWrite + 1) % (int) chorusDelayL.size();
        l = juce::jmap(p.chorusMix, l, 0.65f * l + 0.35f * chorusL);
        r = juce::jmap(p.chorusMix, r, 0.65f * r + 0.35f * chorusR);

        const int delaySamps = (int) (0.33 * currentSampleRate);
        const int delayRead = (delayWrite - delaySamps + (int) delayLineL.size()) % (int) delayLineL.size();
        const float dL = delayLineL[(size_t) delayRead];
        const float dR = delayLineR[(size_t) delayRead];
        delayLineL[(size_t) delayWrite] = l + dL * 0.35f;
        delayLineR[(size_t) delayWrite] = r + dR * 0.35f;
        delayWrite = (delayWrite + 1) % (int) delayLineL.size();
        l += dL * p.delayMix;
        r += dR * p.delayMix;

        const float peak = juce::jmax(std::abs(l), std::abs(r));
        compEnv = juce::jmax(peak, compEnv * 0.9994f);
        const float compGain = 1.0f / (1.0f + juce::jmax(0.0f, compEnv - 0.6f) * (2.5f * p.compAmt));
        l *= compGain * p.masterGain;
        r *= compGain * p.masterGain;
        l = std::tanh(l);
        r = std::tanh(r);

        left[sample] = l;
        if (right != nullptr) right[sample] = r;

        const float mono = 0.5f * (l + r);
        pushScopeSample(mono);
        pushAnalyzerSample(mono);
    }

    for (int i = 0; i < numSamples; ++i)
    {
        wetBufferL[(size_t) i] = left[i];
        wetBufferR[(size_t) i] = right != nullptr ? right[i] : left[i];
    }

    reverbParams.roomSize = 0.45f + 0.45f * p.reverbMix;
    reverbParams.damping = 0.22f + 0.38f * p.reverbMix;
    reverbParams.width = 0.7f + 0.3f * p.chorusMix;
    reverbParams.freezeMode = 0.0f;
    reverbParams.wetLevel = juce::jlimit(0.0f, 1.0f, p.reverbMix * 0.42f);
    reverbParams.dryLevel = 1.0f;
    reverb.setParameters(reverbParams);
    reverb.processStereo(wetBufferL.data(), wetBufferR.data(), numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        const float dryL = left[i];
        const float dryR = right != nullptr ? right[i] : left[i];
        const float wetAmt = juce::jlimit(0.0f, 1.0f, p.reverbMix);
        left[i] = juce::jmap(wetAmt, dryL, wetBufferL[(size_t) i]);
        if (right != nullptr)
            right[i] = juce::jmap(wetAmt, dryR, wetBufferR[(size_t) i]);
    }

    meterLevel.store(buffer.getMagnitude(0, numSamples));
    updatePreviewLfo();
}

juce::AudioProcessorEditor* FiveParksVST3AudioProcessor::createEditor()
{
    return new FiveParksVST3AudioProcessorEditor(*this);
}

void FiveParksVST3AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void FiveParksVST3AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

void FiveParksVST3AudioProcessor::pushScopeSample(float sample) noexcept
{
    const int pos = scopeWritePos.fetch_add(1) % (int) scopeRing.size();
    scopeRing[(size_t) pos] = sample;
}

void FiveParksVST3AudioProcessor::pushAnalyzerSample(float sample) noexcept
{
    fftFifo[(size_t) fftFifoIndex++] = sample;
    if (fftFifoIndex >= (int) fftFifo.size())
    {
        fftFifoIndex = 0;
        computeAnalyzerFrame();
    }
}

void FiveParksVST3AudioProcessor::computeAnalyzerFrame() noexcept
{
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    std::copy(fftFifo.begin(), fftFifo.end(), fftData.begin());
    window.multiplyWithWindowingTable(fftData.data(), fftFifo.size());
    fft.performFrequencyOnlyForwardTransform(fftData.data());
    for (size_t i = 0; i < analyzerBins.size(); ++i)
    {
        const size_t src = juce::jmin(fftFifo.size() / 2 - 1, 1 + i * 4);
        analyzerBins[i] = juce::Decibels::gainToDecibels(fftData[src] / (float) fftFifo.size(), -100.0f);
    }
}

void FiveParksVST3AudioProcessor::copyScopeData(std::array<float, 512>& dest) const noexcept
{
    const int wp = scopeWritePos.load();
    for (size_t i = 0; i < dest.size(); ++i)
        dest[i] = scopeRing[(size_t) ((wp + (int) i) % (int) scopeRing.size())];
}

void FiveParksVST3AudioProcessor::copyAnalyzerData(std::array<float, 128>& dest) const noexcept
{
    dest = analyzerBins;
}

void FiveParksVST3AudioProcessor::updatePreviewLfo() noexcept
{
    const float shape = getF(apvts, "lfo1_shape");
    for (size_t i = 0; i < previewLfoShape.size(); ++i)
    {
        const float p = (float) i / (float) (previewLfoShape.size() - 1);
        const float sine = std::sin(juce::MathConstants<float>::twoPi * p);
        const float tri = 1.0f - 4.0f * std::abs(p - 0.5f);
        const float saw = 2.0f * p - 1.0f;
        const float step = p < 0.5f ? -1.0f : 1.0f;
        float v = sine;
        if (shape < 0.33f) v = juce::jmap(shape / 0.33f, sine, tri);
        else if (shape < 0.66f) v = juce::jmap((shape - 0.33f) / 0.33f, tri, saw);
        else v = juce::jmap((shape - 0.66f) / 0.34f, saw, step);
        previewLfoShape[i] = v;
    }
}

void FiveParksVST3AudioProcessor::copyLfoShape(std::array<float, 128>& dest) const noexcept
{
    dest = previewLfoShape;
}

juce::String FiveParksVST3AudioProcessor::getMatrixSummary() const
{
    static const char* srcs[] = { "-", "ENV2", "ENV3", "LFO1", "LFO2", "LFO3", "LFO4", "VEL" };
    static const char* dsts[] = { "-", "Cutoff", "OSC A Pos", "OSC B Pos", "Pitch", "Level", "Pan", "Dist", "FX" };
    juce::StringArray lines;
    for (int i = 1; i <= 6; ++i)
    {
        const int s = getI(const_cast<juce::AudioProcessorValueTreeState&>(apvts), ("mod" + juce::String(i) + "_src"));
        const int d = getI(const_cast<juce::AudioProcessorValueTreeState&>(apvts), ("mod" + juce::String(i) + "_dst"));
        const float a = getF(const_cast<juce::AudioProcessorValueTreeState&>(apvts), ("mod" + juce::String(i) + "_amt"));
        lines.add(juce::String(i) + ". " + srcs[juce::jlimit(0, 7, s)] + " -> " + dsts[juce::jlimit(0, 8, d)] + "  " + juce::String(a, 2));
    }
    return lines.joinIntoString("\n");
}
