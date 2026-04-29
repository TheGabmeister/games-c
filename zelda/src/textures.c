#include "textures.h"
#include "game_config.h"

Texture2D textures[TEX_COUNT];

static const char *texture_paths[TEX_COUNT] = {
    [TEX_PLAYER] = "assets/sprites/player.png",
    [TEX_TILES]   = "assets/sprites/tiles.png",
    [TEX_SLIME] = "assets/sprites/slime.png",
    [TEX_BAT]   = "assets/sprites/bat.png",
    [TEX_SNAKE]       = "assets/sprites/snake.png",
    [TEX_ARROW]        = "assets/sprites/arrow.png",
    [TEX_BOOMERANG]    = "assets/sprites/boomerang.png",
    [TEX_BOMB]         = "assets/sprites/bomb.png",
    [TEX_ROCK]         = "assets/sprites/rock.png",
    [TEX_SPEAR]        = "assets/sprites/spear.png",
    [TEX_PICKUP_RUPEE] = "assets/sprites/pickup_rupee.png",
    [TEX_PICKUP_HEART] = "assets/sprites/pickup_heart.png",
    [TEX_PICKUP_BOMB]  = "assets/sprites/pickup_bomb.png",
    [TEX_PICKUP_ARROW] = "assets/sprites/pickup_arrow.png",
};

void textures_load(void) {
    for (int i = 0; i < TEX_COUNT; i++) {
        textures[i] = LoadTexture(texture_paths[i]);
        if (!IsTextureValid(textures[i])) {
            TraceLog(LOG_WARNING, "TEXTURES: Failed to load %s", texture_paths[i]);
        }
    }
}

void textures_unload(void) {
    for (int i = 0; i < TEX_COUNT; i++) {
        if (IsTextureValid(textures[i])) {
            UnloadTexture(textures[i]);
        }
    }
}

Rectangle texture_frame_rect(int sheet_cols, int frame) {
    int col = frame % sheet_cols;
    int row = frame / sheet_cols;
    return (Rectangle){ (float)(col * TILE_SIZE), (float)(row * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE };
}
