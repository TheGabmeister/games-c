#ifndef SOUNDS_H
#define SOUNDS_H

#include "game.h"

void sounds_load(Resources *res);
void sounds_unload(Resources *res);
void sound_play(Resources *res, SoundID id);

#endif
