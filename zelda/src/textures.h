#ifndef TEXTURES_H
#define TEXTURES_H

#include "raylib.h"

typedef enum TextureID {
    TEX_PLAYER = 0,
    TEX_TILES,
    TEX_SLIME,
    TEX_BAT,
    TEX_SNAKE,
    TEX_ARROW,
    TEX_BOOMERANG,
    TEX_BOMB,
    TEX_ROCK,
    TEX_SPEAR,
    TEX_PICKUP_RUPEE,
    TEX_PICKUP_HEART,
    TEX_PICKUP_BOMB,
    TEX_PICKUP_ARROW,
    TEX_ITEM_KEY,
    TEX_ITEM_MAP,
    TEX_ITEM_COMPASS,
    TEX_ITEM_HEART_CONTAINER,
    TEX_ITEM_FRAGMENT,
    TEX_COUNT
} TextureID;

extern Texture2D textures[TEX_COUNT];

void textures_load(void);
void textures_unload(void);
Rectangle texture_frame_rect(int sheet_cols, int frame);

#endif
