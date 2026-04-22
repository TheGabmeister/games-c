#ifndef SOUNDS_H
#define SOUNDS_H

#include "common.h"

typedef enum {
    SND_JUMP,
    SND_COIN,
    SND_STOMP,
    SND_POWERUP,
    SND_BRICK_BREAK,
    SND_BUMP,
    SND_DEATH,
    SND_COUNT,
} SoundID;

void sounds_load(void);
void sounds_unload(void);
void sound_play(SoundID id);

#endif
