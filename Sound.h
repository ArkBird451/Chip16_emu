#pragma once

#include <cstdint>

// Audio constants
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE 16
#define CHANNELS 1

class SoundManager {
private:
    // Tone state
    float frequency;
    float duration;       // Total duration in seconds
    float elapsed;        // Time elapsed in seconds
    bool playing;

    // ADSR envelope
    int attack;
    int decay;
    int sustain;
    int release;
    int volume;
    int waveType;         // 0=triangle, 1=sawtooth, 2=pulse, 3=noise

    // Phase for wave generation
    float phase;

    // Calculate envelope volume at current time
    float getEnvelopeVolume();

    // Generate wave sample
    float generateSample();

public:
    SoundManager();
    ~SoundManager();

    // Initialize audio system
    bool init();
    void shutdown();

    // Play tone at frequency for duration
    void playTone(float freq, float durationMs);

    // Stop sound
    void stop();

    // Set ADSR envelope (from SNG instruction)
    void setEnvelope(int atk, int dec, int sus, int rel, int vol, int wave);

    // Update sound state (call each frame)
    void update(float deltaTime);

    // Check if playing
    bool isPlaying() const { return playing; }
};
