#include "sounds.h"

static const char *sound_files[SOUND_COUNT] = {
    [SOUND_COIN] = "assets/coin.wav",
};

void sounds_load(Game *game) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (FileExists(sound_files[i])) {
            game->sounds[i] = LoadSound(sound_files[i]);
        }
    }
    game->sounds_loaded = true;
}

void sounds_unload(Game *game) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (game->sounds[i].frameCount > 0) {
            UnloadSound(game->sounds[i]);
        }
    }
    game->sounds_loaded = false;
}

void sound_play(Game *game, SoundID id) {
    if (!game->sounds_loaded || id < 0 || id >= SOUND_COUNT) {
        return;
    }
    if (game->sounds[id].frameCount > 0) {
        PlaySound(game->sounds[id]);
    }
}
