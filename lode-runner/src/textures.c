#include "textures.h"

static const char *sprite_paths[TEX_COUNT] = {
    [TEX_PLAYER]      = "assets/sprites/player.png",
    [TEX_GUARD]       = "assets/sprites/guard.png",
    [TEX_BRICK]       = "assets/sprites/brick.png",
    [TEX_SOLID]       = "assets/sprites/solid.png",
    [TEX_LADDER]      = "assets/sprites/ladder.png",
    [TEX_ROPE]        = "assets/sprites/rope.png",
    [TEX_GOLD]        = "assets/sprites/gold.png",
    [TEX_EXIT_LADDER] = "assets/sprites/exit_ladder.png",
    [TEX_TRAPDOOR]    = "assets/sprites/trapdoor.png",
    [TEX_HOLE]        = "assets/sprites/hole.png",
};

void textures_load(Texture2D sprites[TEX_COUNT]) {
    for (int i = 0; i < TEX_COUNT; i++) {
        if (FileExists(sprite_paths[i])) {
            sprites[i] = LoadTexture(sprite_paths[i]);
        } else {
            TraceLog(LOG_WARNING, "Sprite not found: %s", sprite_paths[i]);
            sprites[i] = (Texture2D){ 0 };
        }
    }
}

void textures_unload(Texture2D sprites[TEX_COUNT]) {
    for (int i = 0; i < TEX_COUNT; i++) {
        if (sprites[i].id != 0) {
            UnloadTexture(sprites[i]);
        }
    }
}
