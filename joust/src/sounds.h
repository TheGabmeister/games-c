#ifndef SOUNDS_H
#define SOUNDS_H

#include "game.h"

void sounds_load(Game *game);
void sounds_unload(Game *game);
void sound_play(Game *game, SoundID id);

#endif
