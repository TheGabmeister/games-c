#include "sounds.h"

static const char *sound_files[SOUND_COUNT] = {
    "resources/shoot.wav",
    "resources/segment_hit.wav",
    "resources/extra_life.wav",
    "resources/death.wav",
    "resources/level_complete.wav",
    "resources/scorpion.wav",
    "resources/death.wav",
    "resources/level_complete.wav",
    "resources/extra_life.wav",
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
    if (IsSoundValid(game->sounds[id])) {
        PlaySound(game->sounds[id]);
    }
}
