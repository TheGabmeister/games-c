#include "sounds.h"

#include <math.h>
#include <stdlib.h>

static const char *sound_files[SOUND_COUNT] = {
    [SOUND_COIN]          = "assets/coin.wav",
    [SOUND_DIG]           = "assets/dig.wav",
    [SOUND_REFILL]        = "assets/refill.wav",
    [SOUND_PLAYER_DIE]    = "assets/player_die.wav",
    [SOUND_GUARD_FALL]    = "assets/guard_fall.wav",
    [SOUND_GUARD_DIE]     = "assets/guard_die.wav",
    [SOUND_GOLD_COMPLETE] = "assets/gold_complete.wav",
    [SOUND_LEVEL_CLEAR]   = "assets/level_clear.wav",
};

static Sound generate_tone(float duration, float freq_start, float freq_end,
                           float volume, bool noise) {
    int rate = 44100;
    int frames = (int)(duration * (float)rate);
    if (frames < 1) frames = 1;

    short *data = (short *)malloc((size_t)frames * sizeof(short));
    if (!data) return (Sound){ 0 };

    for (int i = 0; i < frames; i++) {
        float t = (float)i / (float)rate;
        float p = t / duration;
        float freq = freq_start + (freq_end - freq_start) * p;
        float env = (1.0f - p);
        env *= env;
        float sample = sinf(6.28318530f * freq * t) * env * volume;

        if (noise) {
            unsigned int h = (unsigned int)i * 1103515245u + 12345u;
            float n = ((float)(h % 65536u) / 32768.0f) - 1.0f;
            sample += n * env * volume * 0.3f;
        }

        int val = (int)(sample * 30000.0f);
        if (val > 32767) val = 32767;
        if (val < -32768) val = -32768;
        data[i] = (short)val;
    }

    Wave wave = { 0 };
    wave.frameCount = (unsigned int)frames;
    wave.sampleRate = (unsigned int)rate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = data;

    Sound sound = LoadSoundFromWave(wave);
    free(data);
    return sound;
}

typedef struct { float dur; float f0; float f1; float vol; bool noise; } SoundDef;

static const SoundDef defs[SOUND_COUNT] = {
    [SOUND_COIN]          = { 0.12f, 900.0f,  1400.0f, 0.35f, false },
    [SOUND_DIG]           = { 0.18f, 350.0f,  80.0f,   0.45f, true  },
    [SOUND_REFILL]        = { 0.10f, 200.0f,  700.0f,  0.30f, false },
    [SOUND_PLAYER_DIE]    = { 0.40f, 500.0f,  80.0f,   0.50f, true  },
    [SOUND_GUARD_FALL]    = { 0.15f, 500.0f,  200.0f,  0.30f, false },
    [SOUND_GUARD_DIE]     = { 0.25f, 350.0f,  50.0f,   0.40f, true  },
    [SOUND_GOLD_COMPLETE] = { 0.50f, 500.0f,  1400.0f, 0.40f, false },
    [SOUND_LEVEL_CLEAR]   = { 0.70f, 400.0f,  1600.0f, 0.40f, false },
};

void sounds_load(Sound sounds[SOUND_COUNT]) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (FileExists(sound_files[i])) {
            sounds[i] = LoadSound(sound_files[i]);
        } else {
            sounds[i] = generate_tone(
                defs[i].dur, defs[i].f0, defs[i].f1, defs[i].vol, defs[i].noise);
        }
    }
}

void sounds_unload(Sound sounds[SOUND_COUNT]) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (sounds[i].frameCount > 0) {
            UnloadSound(sounds[i]);
        }
    }
}

void sound_play(const Sound sounds[SOUND_COUNT], SoundID id) {
    if (id < 0 || id >= SOUND_COUNT) return;
    if (sounds[id].frameCount > 0) PlaySound(sounds[id]);
}
