#ifndef TEXTURES_H
#define TEXTURES_H

#include "raylib.h"

typedef enum TextureID {
    TEX_PLAYER = 0,
    TEX_GUARD,
    TEX_BRICK,
    TEX_SOLID,
    TEX_LADDER,
    TEX_ROPE,
    TEX_GOLD,
    TEX_EXIT_LADDER,
    TEX_TRAPDOOR,
    TEX_HOLE,
    TEX_COUNT
} TextureID;

void textures_load(Texture2D sprites[TEX_COUNT]);
void textures_unload(Texture2D sprites[TEX_COUNT]);

#endif
