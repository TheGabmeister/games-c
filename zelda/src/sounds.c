#include "sounds.h"
#include "game.h"

static const char *sound_files[SOUND_COUNT] = {
    [SOUND_COIN] = "assets/coin.wav",
};

void sounds_load(Game *game) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (FileExists(sound_files[i])) {
            game->sounds[i] = LoadSound(sound_files[i]);
            game->sound_loaded[i] = IsSoundValid(game->sounds[i]);
        }
    }

    const char *music_path = "assets/music/overworld.ogg";
    if (FileExists(music_path)) {
        game->overworld_music = LoadMusicStream(music_path);
        game->music_loaded = IsMusicValid(game->overworld_music);
        if (game->music_loaded) PlayMusicStream(game->overworld_music);
    }
}

void sounds_unload(Game *game) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (game->sound_loaded[i]) UnloadSound(game->sounds[i]);
    }
    if (game->music_loaded) {
        UnloadMusicStream(game->overworld_music);
    }
}

void sound_play(Game *game, SoundID id) {
    if (game->sound_loaded[id]) {
        PlaySound(game->sounds[id]);
    }
}
