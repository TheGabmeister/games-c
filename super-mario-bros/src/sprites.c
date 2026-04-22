#include "sprites.h"

static Texture2D textures[SPR_COUNT];
static bool loaded = false;

static const char *sprite_paths[SPR_COUNT] = {
    [SPR_MARIO_STAND]   = "resources/mario_small_stand.png",
    [SPR_MARIO_WALK1]   = "resources/mario_small_walk1.png",
    [SPR_MARIO_JUMP]    = "resources/mario_small_jump.png",
    [SPR_TILE_GROUND]   = "resources/tile_ground.png",
    [SPR_TILE_BRICK]    = "resources/tile_brick.png",
    [SPR_TILE_QUESTION] = "resources/tile_question.png",
    [SPR_TILE_USED]     = "resources/tile_used.png",
    [SPR_TILE_HARD]     = "resources/tile_hard.png",
    [SPR_TILE_PIPE_TL]  = "resources/tile_pipe_tl.png",
    [SPR_TILE_PIPE_TR]  = "resources/tile_pipe_tr.png",
    [SPR_TILE_PIPE_BL]  = "resources/tile_pipe_bl.png",
    [SPR_TILE_PIPE_BR]  = "resources/tile_pipe_br.png",
    [SPR_TILE_FLAGPOLE] = "resources/tile_flagpole.png",
};

void sprites_load(void) {
    for (int i = 0; i < SPR_COUNT; i++) {
        if (sprite_paths[i]) {
            textures[i] = LoadTexture(sprite_paths[i]);
        }
    }
    loaded = true;
}

void sprites_unload(void) {
    if (!loaded) return;
    for (int i = 0; i < SPR_COUNT; i++) {
        if (textures[i].id > 0) {
            UnloadTexture(textures[i]);
        }
    }
    loaded = false;
}

Texture2D sprites_get(SpriteID id) {
    return textures[id];
}
