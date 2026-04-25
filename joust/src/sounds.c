#include "sounds.h"

static const char *sound_files[SOUND_COUNT] = {
    [SOUND_FLAP] = "assets/flap.wav",
    [SOUND_JOUST_WIN] = "assets/joust_win.wav",
    [SOUND_JOUST_BOUNCE] = "assets/joust_bounce.wav",
    [SOUND_EGG_COLLECT] = "assets/egg_collect.wav",
    [SOUND_EGG_HATCH] = "assets/egg_hatch.wav",
    [SOUND_PLAYER_DIE] = "assets/player_die.wav",
    [SOUND_WAVE_CLEAR] = "assets/wave_clear.wav",
    [SOUND_START] = "assets/start.wav",
    [SOUND_LAVA] = "assets/lava.wav",
    [SOUND_PREDATOR] = "assets/predator.wav",
    [SOUND_PAUSE] = "assets/pause.wav",
};

void sounds_load(Resources *res) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        res->sounds[i].loaded = false;
        if (FileExists(sound_files[i])) {
            res->sounds[i].sound = LoadSound(sound_files[i]);
            res->sounds[i].loaded = IsSoundValid(res->sounds[i].sound);
        }
    }
}

void sounds_unload(Resources *res) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (res->sounds[i].loaded && IsSoundValid(res->sounds[i].sound)) {
            UnloadSound(res->sounds[i].sound);
        }
        res->sounds[i].loaded = false;
    }
}

void sound_play(Resources *res, SoundID id) {
    if (id < 0 || id >= SOUND_COUNT) {
        return;
    }
    if (res->sounds[id].loaded) {
        PlaySound(res->sounds[id].sound);
    }
}
