#pragma once
#include "game.h"

/* Stub audio system — functions exist but do nothing until assets are added */
void audio_init(void);
void audio_shutdown(void);
void audio_update(const GameState *gs);
