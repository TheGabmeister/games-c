#include "sounds.h"

static Sound sounds[SND_COUNT];
static bool loaded = false;

static const char *sound_paths[SND_COUNT] = {
    [SND_JUMP]        = "resources/jump.wav",
    [SND_COIN]        = "resources/coin.wav",
    [SND_STOMP]       = "resources/stomp.wav",
    [SND_POWERUP]     = "resources/powerup.wav",
    [SND_BRICK_BREAK] = "resources/brick_break.wav",
    [SND_BUMP]        = "resources/bump.wav",
    [SND_DEATH]       = "resources/death.wav",
    [SND_KICK]        = "resources/kick.wav",
    [SND_FIREBALL]    = "resources/fireball.wav",
    [SND_FLAGPOLE]    = "resources/flagpole.wav",
    [SND_PIPE]        = "resources/pipe.wav",
    [SND_BOWSER_FALL] = "resources/bowser_fall.wav",
};

void sounds_load(void) {
    for (int i = 0; i < SND_COUNT; i++) {
        if (sound_paths[i]) {
            sounds[i] = LoadSound(sound_paths[i]);
        }
    }
    loaded = true;
}

void sounds_unload(void) {
    if (!loaded) return;
    for (int i = 0; i < SND_COUNT; i++) {
        if (sounds[i].frameCount > 0) {
            UnloadSound(sounds[i]);
        }
    }
    loaded = false;
}

void sound_play(SoundID id) {
    if (loaded && sounds[id].frameCount > 0) {
        PlaySound(sounds[id]);
    }
}
