#include "textures.h"
#include "game_config.h"

Texture2D textures[TEX_COUNT];

static const char *texture_paths[TEX_COUNT] = {
    [TEX_PLAYER] = "assets/sprites/player.png",
    [TEX_TILES]   = "assets/sprites/tiles.png",
    [TEX_ENEMIES] = "assets/sprites/enemies.png",
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
