#ifndef SOUNDS_H
#define SOUNDS_H

#include "raylib.h"
#include <stdbool.h>

typedef enum SoundID {
    SOUND_COIN = 0,
    SOUND_DIG,
    SOUND_REFILL,
    SOUND_PLAYER_DIE,
    SOUND_GUARD_FALL,
    SOUND_GUARD_DIE,
    SOUND_GOLD_COMPLETE,
    SOUND_LEVEL_CLEAR,
    SOUND_COUNT
} SoundID;

void sounds_load(Sound sounds[SOUND_COUNT]);
void sounds_unload(Sound sounds[SOUND_COUNT]);
void sound_play(const Sound sounds[SOUND_COUNT], SoundID id);

#endif
