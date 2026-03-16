#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>

struct StereoSample
{
    float left = 0.0f;
    float right = 0.0f;
};

enum class ModSource : int
{
    none = 0,
    env2,
    env3,
    lfo1,
    lfo2,
    lfo3,
    lfo4,
    velocity
};

enum class ModDestination : int
{
    none = 0,
    filterCutoff,
    oscAPos,
    oscBPos,
    pitch,
    level,
    pan,
    distortion,
    fxMix
};

struct ModSlot
{
    ModSource source = ModSource::none;
    ModDestination destination = ModDestination::none;
    float amount = 0.0f;
};

struct RuntimeParams
{
    int playMode = 2;
    float glideMs = 35.0f;

    int oscAOct = 0, oscASemi = 0;
    float oscAFine = 0.0f, oscAUnison = 3.0f, oscADetune = 0.18f, oscABlend = 0.76f, oscAPhase = 0.0f, oscARand = 0.0f, oscAPos = 0.25f, oscALevel = 0.85f, oscAPan = 0.5f;
    int oscBOct = 0, oscBSemi = 7;
    float oscBFine = 0.0f, oscBUnison = 2.0f, oscBDetune = 0.14f, oscBBlend = 0.68f, oscBPhase = 0.0f, oscBRand = 0.0f, oscBPos = 0.58f, oscBLevel = 0.70f, oscBPan = 0.5f;

    float subLevel = 0.7f; int subWave = 0; bool subDirect = false;
    float noisePitch = 0.5f, noisePhase = 0.0f, noisePan = 0.5f, noiseLevel = 0.0f; bool noiseDirect = false;

    int filterMode = 0; float cutoff = 1200.0f, resonance = 0.25f, filterDrive = 0.2f, filterMix = 1.0f, filterPan = 0.5f;

    float env1A = 0.005f, env1H = 0.0f, env1D = 0.25f, env1S = 0.75f, env1R = 0.2f;
    float env2A = 0.001f, env2H = 0.0f, env2D = 0.3f, env2S = 0.0f, env2R = 0.2f;
    float env3A = 0.001f, env3H = 0.0f, env3D = 0.4f, env3S = 0.0f, env3R = 0.2f;

    std::array<float, 4> lfoRate { 1.0f, 0.3f, 3.0f, 0.11f };
    std::array<float, 4> lfoAmount { 0.0f, 0.0f, 0.0f, 0.0f };
    std::array<float, 4> lfoShape { 0.0f, 0.3f, 0.7f, 1.0f };
    std::array<float, 4> lfoSmooth { 0.1f, 0.1f, 0.1f, 0.1f };
    std::array<int, 4> lfoSync { 0, 0, 0, 0 };

    std::array<ModSlot, 6> matrix {};

    float distDrive = 0.25f, distMix = 0.4f;
    float chorusMix = 0.15f, delayMix = 0.12f, reverbMix = 0.08f, compAmt = 0.2f;

    int polyphony = 8;
    float masterGain = 0.7f;
};

struct WavetableOscillator
{
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float wavePos = 0.25f;
    float frequency = 110.0f;

    void prepare(double sr) { sampleRate = sr; phase = 0.0f; }
    void setFrequency(float hz) { frequency = juce::jmax(0.0f, hz); }
    void setPosition(float p) { wavePos = juce::jlimit(0.0f, 1.0f, p); }
    void setPhase(float p) { phase = p - std::floor(p); }

    float frame(float p, float pos) const
    {
        const float s = std::sin(juce::MathConstants<float>::twoPi * p);
        const float tri = 2.0f * std::abs(2.0f * p - 1.0f) - 1.0f;
        const float saw = 2.0f * p - 1.0f;
        const float square = p < 0.5f ? 1.0f : -1.0f;
        const float vocal = std::sin(juce::MathConstants<float>::twoPi * p + 2.2f * std::sin(juce::MathConstants<float>::twoPi * p));
        if (pos < 0.25f) return juce::jmap(pos / 0.25f, s, tri);
        if (pos < 0.5f)  return juce::jmap((pos - 0.25f) / 0.25f, tri, saw);
        if (pos < 0.75f) return juce::jmap((pos - 0.5f) / 0.25f, saw, square);
        return juce::jmap((pos - 0.75f) / 0.25f, square, vocal);
    }

    float process(float phaseOffset = 0.0f)
    {
        const float p = juce::jlimit(0.0f, 1.0f, phase + phaseOffset - std::floor(phase + phaseOffset));
        const float y = frame(p, wavePos);
        phase += frequency / (float) sampleRate;
        phase -= std::floor(phase);
        return y;
    }
};

struct ModEnvelope
{
    juce::ADSR adsr;
    juce::ADSR::Parameters params;

    void prepare(double sr) { adsr.setSampleRate(sr); }
    void set(float a, float d, float s, float r) { params.attack = a; params.decay = d; params.sustain = s; params.release = r; adsr.setParameters(params); }
    void noteOn() { adsr.noteOn(); }
    void noteOff() { adsr.noteOff(); }
    float next() { return adsr.getNextSample(); }
    bool isActive() const { return adsr.isActive(); }
};

struct ShapedLfo
{
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float out = 0.0f;
    void prepare(double sr) { sampleRate = sr; phase = 0.0f; out = 0.0f; }

    float shapeValue(float p, float shape) const
    {
        const float sine = std::sin(juce::MathConstants<float>::twoPi * p);
        const float tri = 1.0f - 4.0f * std::abs(p - 0.5f);
        const float saw = 2.0f * p - 1.0f;
        const float step = p < 0.5f ? -1.0f : 1.0f;
        if (shape < 0.33f) return juce::jmap(shape / 0.33f, sine, tri);
        if (shape < 0.66f) return juce::jmap((shape - 0.33f) / 0.33f, tri, saw);
        return juce::jmap((shape - 0.66f) / 0.34f, saw, step);
    }

    float process(float rate, float shape, float smooth)
    {
        phase += rate / (float) sampleRate;
        phase -= std::floor(phase);
        const float target = shapeValue(phase, juce::jlimit(0.0f, 1.0f, shape));
        const float alpha = juce::jlimit(0.001f, 1.0f, 1.0f - smooth * 0.95f);
        out += (target - out) * alpha;
        return out;
    }
};

struct FilterBlock
{
    juce::dsp::StateVariableTPTFilter<float> left, right;

    void prepare(double sr, int blockSize)
    {
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) blockSize, 1 };
        left.prepare(spec);
        right.prepare(spec);
        left.reset();
        right.reset();
    }

    void update(int mode, float cutoff, float resonance)
    {
        auto t = juce::dsp::StateVariableTPTFilterType::lowpass;
        if (mode == 1) t = juce::dsp::StateVariableTPTFilterType::bandpass;
        else if (mode == 2) t = juce::dsp::StateVariableTPTFilterType::highpass;
        left.setType(t); right.setType(t);
        left.setCutoffFrequency(cutoff); right.setCutoffFrequency(cutoff);
        left.setResonance(resonance); right.setResonance(resonance);
    }

    StereoSample process(float l, float r) { return { left.processSample(0, l), right.processSample(0, r) }; }
};

class FiveParksVoice
{
public:
    void prepare(double sr, int blockSize)
    {
        sampleRate = sr;
        oscA.prepare(sr); oscB.prepare(sr); sub.prepare(sr); noise.prepare(sr);
        for (auto& o : extraA) o.prepare(sr);
        for (auto& o : extraB) o.prepare(sr);
        env1.prepare(sr); env2.prepare(sr); env3.prepare(sr);
        for (auto& l : lfos) l.prepare(sr);
        filter.prepare(sr, blockSize);
        reset();
    }

    void reset()
    {
        active = false; held = false; velocity = 0.0f; currentHz = targetHz = 110.0f; midiNote = -1;
        age = 0; basePan = 0.5f;
    }

    bool isActive() const { return active; }
    int getMidiNote() const { return midiNote; }
    uint64_t getAge() const { return age; }
    float getVelocity() const { return velocity; }

    void start(int note, float vel, const RuntimeParams& p, uint64_t newAge)
    {
        active = true; held = true; midiNote = note; velocity = vel; age = newAge;
        targetHz = currentHz = (float) juce::MidiMessage::getMidiNoteInHertz(note);
        env1.set(p.env1A, p.env1D, p.env1S, p.env1R);
        env2.set(p.env2A, p.env2D, p.env2S, p.env2R);
        env3.set(p.env3A, p.env3D, p.env3S, p.env3R);
        env1.noteOn(); env2.noteOn(); env3.noteOn();
        const float seed = std::sin((float) note * 12.345f) * 43758.5453f;
        const float rnd = seed - std::floor(seed);
        oscA.setPhase(p.oscAPhase + rnd * p.oscARand);
        oscB.setPhase(p.oscBPhase + rnd * p.oscBRand);
        sub.setPhase(0.0f);
        noise.setPhase(p.noisePhase);
        for (size_t i = 0; i < extraA.size(); ++i)
            extraA[i].setPhase(p.oscAPhase + 0.07f * (float) i + rnd * p.oscARand);
        for (size_t i = 0; i < extraB.size(); ++i)
            extraB[i].setPhase(p.oscBPhase + 0.05f * (float) i + rnd * p.oscBRand);
        basePan = juce::jlimit(0.0f, 1.0f, 0.5f + (rnd - 0.5f) * 0.25f);
    }

    void stop() { held = false; env1.noteOff(); env2.noteOff(); env3.noteOff(); }

    StereoSample render(const RuntimeParams& p)
    {
        if (! active) return {};

        const float glideCoeff = p.glideMs <= 0.0f ? 1.0f : (1.0f - std::exp(-1.0f / (0.001f * p.glideMs * (float) sampleRate)));
        currentHz += (targetHz - currentHz) * glideCoeff;

        const float env2v = env2.next();
        const float env3v = env3.next();
        std::array<float, 4> lfoValues {};
        for (size_t i = 0; i < 4; ++i)
            lfoValues[i] = lfos[i].process(p.lfoRate[i], p.lfoShape[i], p.lfoSmooth[i]) * p.lfoAmount[i];

        auto modValue = [&](ModSource src)
        {
            switch (src)
            {
                case ModSource::env2: return env2v;
                case ModSource::env3: return env3v;
                case ModSource::lfo1: return lfoValues[0];
                case ModSource::lfo2: return lfoValues[1];
                case ModSource::lfo3: return lfoValues[2];
                case ModSource::lfo4: return lfoValues[3];
                case ModSource::velocity: return velocity;
                default: return 0.0f;
            }
        };

        float modCutoff = 0.0f, modAPos = 0.0f, modBPos = 0.0f, modPitch = 0.0f, modLevel = 0.0f, modPan = 0.0f, modDist = 0.0f, modFx = 0.0f;
        for (const auto& slot : p.matrix)
        {
            const float v = modValue(slot.source) * slot.amount;
            switch (slot.destination)
            {
                case ModDestination::filterCutoff: modCutoff += v; break;
                case ModDestination::oscAPos:      modAPos += v; break;
                case ModDestination::oscBPos:      modBPos += v; break;
                case ModDestination::pitch:        modPitch += v; break;
                case ModDestination::level:        modLevel += v; break;
                case ModDestination::pan:          modPan += v; break;
                case ModDestination::distortion:   modDist += v; break;
                case ModDestination::fxMix:        modFx += v; break;
                default: break;
            }
        }

        const float pitchRatio = std::pow(2.0f, modPitch * 0.25f);
        const float hzA = currentHz * std::pow(2.0f, (float) (12 * p.oscAOct + p.oscASemi) / 12.0f) * std::pow(2.0f, p.oscAFine / 1200.0f) * pitchRatio;
        const float hzB = currentHz * std::pow(2.0f, (float) (12 * p.oscBOct + p.oscBSemi) / 12.0f) * std::pow(2.0f, p.oscBFine / 1200.0f) * pitchRatio;
        const float hzSub = currentHz * 0.5f * pitchRatio;

        oscA.setFrequency(hzA); oscB.setFrequency(hzB); sub.setFrequency(hzSub);
        oscA.setPosition(p.oscAPos + modAPos * 0.35f);
        oscB.setPosition(p.oscBPos + modBPos * 0.35f);
        noise.setFrequency(100.0f + p.noisePitch * 8000.0f);
        noise.setPosition(0.85f);

        auto renderStack = [&](WavetableOscillator& core, std::array<WavetableOscillator, 4>& extras, float unison, float detune, float blend, float pan, bool isA)
        {
            const int voices = juce::jlimit(1, 5, (int) std::round(unison));
            float l = 0.0f, r = 0.0f;
            for (int i = 0; i < voices; ++i)
            {
                const float spread = voices == 1 ? 0.0f : juce::jmap((float) i / (float) (voices - 1), -1.0f, 1.0f) * detune * 0.05f;
                WavetableOscillator* osc = (i == 0 ? &core : &extras[(size_t) (i - 1)]);
                const float baseFreq = isA ? hzA : hzB;
                osc->setFrequency(baseFreq * (1.0f + spread));
                const float s = osc->process(spread * 0.1f);
                const float panPos = juce::jlimit(0.0f, 1.0f, pan + spread * 1.2f);
                const float gain = (i == 0 ? blend : (1.0f - blend) / juce::jmax(1, voices - 1));
                l += s * std::cos(juce::MathConstants<float>::halfPi * panPos) * gain;
                r += s * std::sin(juce::MathConstants<float>::halfPi * panPos) * gain;
            }
            return StereoSample { l, r };
        };

        const auto oscOutA = renderStack(oscA, extraA, p.oscAUnison, p.oscADetune, p.oscABlend, juce::jlimit(0.0f, 1.0f, p.oscAPan + modPan * 0.2f), true);
        const auto oscOutB = renderStack(oscB, extraB, p.oscBUnison, p.oscBDetune, p.oscBBlend, juce::jlimit(0.0f, 1.0f, p.oscBPan - modPan * 0.2f), false);

        float subSample = 0.0f;
        switch (p.subWave)
        {
            case 1: sub.setPosition(0.35f); break;
            case 2: sub.setPosition(0.55f); break;
            case 3: sub.setPosition(0.95f); break;
            default: sub.setPosition(0.05f); break;
        }
        subSample = sub.process() * p.subLevel;

        const float noiseRaw = noise.process() * p.noiseLevel;
        const float noisePanL = std::cos(juce::MathConstants<float>::halfPi * p.noisePan);
        const float noisePanR = std::sin(juce::MathConstants<float>::halfPi * p.noisePan);

        float l = oscOutA.left * p.oscALevel + oscOutB.left * p.oscBLevel;
        float r = oscOutA.right * p.oscALevel + oscOutB.right * p.oscBLevel;

        if (! p.subDirect) { l += subSample; r += subSample; }
        if (! p.noiseDirect) { l += noiseRaw * noisePanL; r += noiseRaw * noisePanR; }

        const float cutoff = juce::jlimit(20.0f, 18000.0f, p.cutoff * std::pow(2.0f, modCutoff * 4.0f));
        const float resonance = juce::jlimit(0.1f, 1.15f, p.resonance + std::abs(modCutoff) * 0.2f);
        filter.update(p.filterMode, cutoff, resonance);

        l = std::tanh(l * (1.0f + p.filterDrive * 4.0f));
        r = std::tanh(r * (1.0f + p.filterDrive * 4.0f));
        const auto filtered = filter.process(l, r);
        l = juce::jmap(p.filterMix, l, filtered.left);
        r = juce::jmap(p.filterMix, r, filtered.right);

        if (p.subDirect) { l += subSample; r += subSample; }
        if (p.noiseDirect) { l += noiseRaw * noisePanL; r += noiseRaw * noisePanR; }

        const float distDrive = p.distDrive + modDist * 0.35f;
        const float dirtyL = std::tanh(l * (1.0f + distDrive * 8.0f));
        const float dirtyR = std::tanh(r * (1.0f + distDrive * 8.0f));
        l = juce::jmap(p.distMix, l, dirtyL);
        r = juce::jmap(p.distMix, r, dirtyR);

        const float amp = env1.next() * velocity * juce::jlimit(0.0f, 2.0f, 1.0f + modLevel * 0.5f);
        l *= amp;
        r *= amp;

        if (! env1.isActive())
            reset();

        return { l, r };
    }

private:
    double sampleRate = 44100.0;
    bool active = false, held = false;
    int midiNote = -1;
    float velocity = 0.0f;
    float currentHz = 110.0f, targetHz = 110.0f;
    float basePan = 0.5f;
    uint64_t age = 0;

    WavetableOscillator oscA, oscB, sub, noise;
    std::array<WavetableOscillator, 4> extraA {}, extraB {};
    ModEnvelope env1, env2, env3;
    std::array<ShapedLfo, 4> lfos {};
    FilterBlock filter;
};
