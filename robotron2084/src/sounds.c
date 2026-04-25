#include "sounds.h"

static const char *sound_files[SOUND_COUNT] = {
    [SOUND_START] = "assets/start.wav",
    [SOUND_PLAYER_SHOOT] = "assets/player_shoot.wav",
    [SOUND_ENEMY_EXPLODE] = "assets/enemy_explode.wav",
    [SOUND_PLAYER_DIE] = "assets/player_die.wav",
    [SOUND_HUMAN_RESCUE] = "assets/human_rescue.wav",
    [SOUND_EXTRA_LIFE] = "assets/extra_life.wav",
    [SOUND_WAVE_CLEAR] = "assets/wave_clear.wav",
    [SOUND_ENEMY_SHOOT] = "assets/enemy_shoot.wav",
    [SOUND_BRAIN_MISSILE] = "assets/brain_missile.wav",
    [SOUND_HULK_HIT] = "assets/hulk_hit.wav",
};

void sounds_load(Game *game) {
    game->sounds_loaded = false;
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (FileExists(sound_files[i])) {
            game->sounds[i] = LoadSound(sound_files[i]);
            if (IsSoundValid(game->sounds[i])) {
                game->sounds_loaded = true;
            }
        }
    }

    if (IsSoundValid(game->sounds[SOUND_PLAYER_SHOOT])) {
        SetSoundVolume(game->sounds[SOUND_PLAYER_SHOOT], PLAYER_SHOOT_VOLUME);
    }
    if (IsSoundValid(game->sounds[SOUND_ENEMY_SHOOT])) {
        SetSoundVolume(game->sounds[SOUND_ENEMY_SHOOT], ENEMY_SHOOT_VOLUME);
    }
    if (IsSoundValid(game->sounds[SOUND_BRAIN_MISSILE])) {
        SetSoundVolume(game->sounds[SOUND_BRAIN_MISSILE], BRAIN_MISSILE_VOLUME);
    }
    if (IsSoundValid(game->sounds[SOUND_HULK_HIT])) {
        SetSoundVolume(game->sounds[SOUND_HULK_HIT], HULK_HIT_VOLUME);
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
