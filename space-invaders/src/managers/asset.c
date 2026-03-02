#include "asset.h"
#include <SDL3/SDL.h>
#include <string.h>

AssetManager asset_manager = {0};

void asset_manager_add(const char *id, SDL_Texture *texture)
{
    if (asset_manager.count >= ASSET_MANAGER_MAX) {
        SDL_Log("asset_manager_add: manager full, cannot add '%s'", id);
        return;
    }
    TextureEntry *e = &asset_manager.entries[asset_manager.count++];
    strncpy(e->id, id, sizeof(e->id) - 1);
    e->id[sizeof(e->id) - 1] = '\0';
    e->texture = texture;
}

SDL_Texture *asset_manager_get(const char *id)
{
    for (int i = 0; i < asset_manager.count; i++) {
        if (strcmp(asset_manager.entries[i].id, id) == 0)
            return asset_manager.entries[i].texture;
    }
    return NULL;
}

void asset_manager_destroy(void)
{
    for (int i = 0; i < asset_manager.count; i++)
        SDL_DestroyTexture(asset_manager.entries[i].texture);
    asset_manager.count = 0;
}
