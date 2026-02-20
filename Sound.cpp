#include "Sound.h"
#include "raylib.h"
#include <cmath>
#include <cstdlib>

#define PI 3.14159265358979323846f

// Audio stream for real-time synthesis
static AudioStream audioStream;
static bool audioInitialized = false;

SoundManager::SoundManager() 
    : frequency(0), duration(0), elapsed(0), playing(false),
      attack(0), decay(0), sustain(15), release(0),
      volume(15), waveType(0), phase(0) {
}

SoundManager::~SoundManager() {
    shutdown();
}

bool SoundManager::init() {
    InitAudioDevice();

    SetAudioStreamBufferSizeDefault(4096);
    audioStream = LoadAudioStream(SAMPLE_RATE, SAMPLE_SIZE, CHANNELS);
    PlayAudioStream(audioStream);
    
    audioInitialized = IsAudioDeviceReady();
    return audioInitialized;
}

void SoundManager::shutdown() {
    if (audioInitialized) {
        StopAudioStream(audioStream);
        UnloadAudioStream(audioStream);
        CloseAudioDevice();
        audioInitialized = false;
    }
}

void SoundManager::playTone(float freq, float durationMs) {
    frequency = freq;
    duration = durationMs / 1000.0f;  // Convert to seconds
    elapsed = 0.0f;
    phase = 0.0f;
    playing = true;
}

void SoundManager::stop() {
    playing = false;
    frequency = 0;
    elapsed = 0;
}

void SoundManager::setEnvelope(int atk, int dec, int sus, int rel, int vol, int wave) {
    attack = atk & 0x0F;
    decay = dec & 0x0F;
    sustain = sus & 0x0F;
    release = rel & 0x0F;
    volume = vol & 0x0F;
    waveType = wave & 0x03;
}

float SoundManager::getEnvelopeVolume() {
    if (!playing) return 0.0f;

    // Convert ADSR values to time (each unit = ~15ms)
    float attackTime = attack * 0.015f;
    float decayTime = decay * 0.015f;
    float releaseTime = release * 0.015f;
    float sustainLevel = sustain / 15.0f;
    float maxVolume = volume / 15.0f;

    float t = elapsed;
    float totalTime = duration;
    float releaseStart = totalTime - releaseTime;

    float env = 0.0f;

    if (t < attackTime) {
        // Attack phase
        env = (t / attackTime) * maxVolume;
    }
    else if (t < attackTime + decayTime) {
        // Decay phase
        float decayProgress = (t - attackTime) / decayTime;
        env = maxVolume - (maxVolume - sustainLevel * maxVolume) * decayProgress;
    }
    else if (t < releaseStart) {
        // Sustain phase
        env = sustainLevel * maxVolume;
    }
    else if (t < totalTime) {
        // Release phase
        float releaseProgress = (t - releaseStart) / releaseTime;
        env = sustainLevel * maxVolume * (1.0f - releaseProgress);
    }
    else {
        env = 0.0f;
    }

    return env;
}

float SoundManager::generateSample() {
    float sample = 0.0f;

    switch (waveType) {
    case 0:  // Triangle wave
        sample = 2.0f * fabsf(2.0f * (phase - floorf(phase + 0.5f))) - 1.0f;
        break;
    case 1:  // Sawtooth wave
        sample = 2.0f * (phase - floorf(phase + 0.5f));
        break;
    case 2:  // Pulse wave (square)
        sample = (phase - floorf(phase) < 0.5f) ? 1.0f : -1.0f;
        break;
    case 3:  // Noise
        sample = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        break;
    }

    return sample;
}

void SoundManager::update(float deltaTime) {
    if (!playing) return;

    // Check if sound duration has expired
    if (elapsed >= duration) {
        stop();
        return;
    }

    // Fill audio buffer if needed
    if (IsAudioStreamProcessed(audioStream)) {
        float buffer[4096];
        int samplesToWrite = 4096;

        for (int i = 0; i < samplesToWrite; i++) {
            if (!playing || elapsed >= duration) {
                buffer[i] = 0.0f;
            }
            else {
                // Generate sample
                float sample = generateSample();

                // Apply envelope
                float env = getEnvelopeVolume();
                buffer[i] = sample * env * 0.3f;  // 0.3 = master volume

                // Advance phase
                phase += frequency / SAMPLE_RATE;
                if (phase >= 1.0f) phase -= 1.0f;

                // Advance time
                elapsed += 1.0f / SAMPLE_RATE;
            }
        }

        UpdateAudioStream(audioStream, buffer, samplesToWrite);
    }

    // Also advance elapsed time based on delta
    // (This ensures timing works even if audio buffer isn't being processed)
    // elapsed += deltaTime;  // Commented out - handled in buffer fill
}
