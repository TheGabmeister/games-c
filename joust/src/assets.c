#include "assets.h"

static const char *texture_files[TEXTURE_COUNT] = {
    [TEXTURE_PLAYER_IDLE] = "assets/player_idle.png",
    [TEXTURE_PLAYER_FLAP_1] = "assets/player_flap_1.png",
    [TEXTURE_PLAYER_FLAP_2] = "assets/player_flap_2.png",
    [TEXTURE_ENEMY_GRUNT] = "assets/enemy_grunt.png",
    [TEXTURE_ENEMY_HUNTER] = "assets/enemy_hunter.png",
    [TEXTURE_ENEMY_CHAMPION] = "assets/enemy_champion.png",
    [TEXTURE_EGG] = "assets/egg.png",
    [TEXTURE_EGG_HATCHING] = "assets/egg_hatching.png",
    [TEXTURE_PLATFORM] = "assets/platform.png",
    [TEXTURE_LAVA] = "assets/lava.png",
    [TEXTURE_SPARK] = "assets/spark.png",
};

void assets_load(Game *game) {
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        game->textures[i].loaded = false;
        if (FileExists(texture_files[i])) {
            game->textures[i].texture = LoadTexture(texture_files[i]);
            game->textures[i].loaded = IsTextureValid(game->textures[i].texture);
            if (game->textures[i].loaded) {
                SetTextureFilter(game->textures[i].texture, TEXTURE_FILTER_POINT);
            }
        }
    }
}

void assets_unload(Game *game) {
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        if (game->textures[i].loaded && IsTextureValid(game->textures[i].texture)) {
            UnloadTexture(game->textures[i].texture);
        }
        game->textures[i].loaded = false;
    }
}
