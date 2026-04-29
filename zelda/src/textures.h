#ifndef TEXTURES_H
#define TEXTURES_H

#include "raylib.h"

typedef enum TextureID {
    TEX_PLAYER = 0,
    TEX_TILES,
    TEX_SLIME,
    TEX_BAT,
    TEX_SNAKE,
    TEX_PROJECTILES,
    TEX_COUNT
} TextureID;

extern Texture2D textures[TEX_COUNT];

void textures_load(void);
void textures_unload(void);
Rectangle texture_frame_rect(int sheet_cols, int frame);

#endif
