#ifndef TEXTURES_H
#define TEXTURES_H

#include "game.h"

void textures_load(Game *game);
void textures_unload(Game *game);
bool texture_is_ready(Game *game, TextureID id);

#endif
