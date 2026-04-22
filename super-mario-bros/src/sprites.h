#ifndef SPRITES_H
#define SPRITES_H

#include "common.h"

typedef enum {
    SPR_MARIO_STAND,
    SPR_MARIO_WALK1,
    SPR_MARIO_JUMP,
    SPR_TILE_GROUND,
    SPR_TILE_BRICK,
    SPR_TILE_QUESTION,
    SPR_TILE_USED,
    SPR_TILE_HARD,
    SPR_TILE_PIPE_TL,
    SPR_TILE_PIPE_TR,
    SPR_TILE_PIPE_BL,
    SPR_TILE_PIPE_BR,
    SPR_TILE_FLAGPOLE,
    SPR_COUNT,
} SpriteID;

void sprites_load(void);
void sprites_unload(void);
Texture2D sprites_get(SpriteID id);

#endif
