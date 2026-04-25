#include "textures.h"

static const char *texture_files[TEXTURE_COUNT] = {
    [TEXTURE_PLAYER] = "assets/player.png",
    [TEXTURE_GRUNT] = "assets/grunt.png",
    [TEXTURE_HULK] = "assets/hulk.png",
    [TEXTURE_HUMAN_MOMMY] = "assets/human_mommy.png",
    [TEXTURE_HUMAN_DADDY] = "assets/human_daddy.png",
    [TEXTURE_HUMAN_MIKEY] = "assets/human_mikey.png",
    [TEXTURE_ELECTRODE] = "assets/electrode.png",
    [TEXTURE_SPHEROID] = "assets/spheroid.png",
    [TEXTURE_ENFORCER] = "assets/enforcer.png",
    [TEXTURE_QUARK] = "assets/quark.png",
    [TEXTURE_TANK] = "assets/tank.png",
    [TEXTURE_BRAIN] = "assets/brain.png",
    [TEXTURE_PROG] = "assets/prog.png",
};

void textures_load(Game *game) {
    game->textures_loaded = false;

    for (int i = 0; i < TEXTURE_COUNT; i++) {
        if (FileExists(texture_files[i])) {
            game->textures[i] = LoadTexture(texture_files[i]);
            if (IsTextureValid(game->textures[i])) {
                SetTextureFilter(game->textures[i], TEXTURE_FILTER_POINT);
                game->textures_loaded = true;
            }
        }
    }
}

void textures_unload(Game *game) {
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        if (IsTextureValid(game->textures[i])) {
            UnloadTexture(game->textures[i]);
        }
    }
}

bool texture_is_ready(Game *game, TextureID id) {
    if (id < 0 || id >= TEXTURE_COUNT) {
        return false;
    }

    return IsTextureValid(game->textures[id]);
}
