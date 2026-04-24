#include "sounds.h"

static const char *sound_files[SOUND_COUNT] = {
    [SOUND_COIN] = "assets/coin.wav",
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
