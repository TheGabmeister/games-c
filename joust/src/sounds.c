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

void sounds_load(Game *game) {
    game->sounds_loaded = false;
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (FileExists(sound_files[i])) {
            game->sounds[i] = LoadSound(sound_files[i]);
            game->sounds_loaded = true;
        }
    }
}

void sounds_unload(Game *game) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (IsSoundValid(game->sounds[i])) {
            UnloadSound(game->sounds[i]);
        }
    }
}

void sound_play(Game *game, SoundID id) {
    if (id < 0 || id >= SOUND_COUNT) {
        return;
    }

    if (IsSoundValid(game->sounds[id])) {
        PlaySound(game->sounds[id]);
    }
}
