#include "sounds.h"

static const char *sound_files[SOUND_COUNT] = {
    [SOUND_COIN] = "assets/coin.wav",
    [SOUND_DIG] = "assets/dig.wav",
    [SOUND_REFILL] = "assets/refill.wav",
    [SOUND_PLAYER_DIE] = "assets/player_die.wav",
    [SOUND_GUARD_FALL] = "assets/guard_fall.wav",
    [SOUND_GUARD_DIE] = "assets/guard_die.wav",
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
