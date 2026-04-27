#include "sounds.h"

static const char *sound_files[SOUND_COUNT] = {
    [SOUND_COIN] = "assets/coin.wav",
};

void sounds_load(Game *game) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (FileExists(sound_files[i])) {
            game->sounds[i] = LoadSound(sound_files[i]);
            game->sounds_loaded = true;
        }
    }

    const char *music_path = "assets/music/overworld.ogg";
    if (FileExists(music_path)) {
        game->overworld_music = LoadMusicStream(music_path);
        game->music_loaded = true;
        PlayMusicStream(game->overworld_music);
    }
}

void sounds_unload(Game *game) {
    if (game->sounds_loaded) {
        for (int i = 0; i < SOUND_COUNT; i++) {
            UnloadSound(game->sounds[i]);
        }
    }
    if (game->music_loaded) {
        UnloadMusicStream(game->overworld_music);
    }
}

void sound_play(Game *game, SoundID id) {
    if (game->sounds_loaded) {
        PlaySound(game->sounds[id]);
    }
}
