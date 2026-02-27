#include "asset_registry.h"
#include <string.h>

AssetRegistry asset_registry = {0};

void asset_registry_add(const char *id, SDL_Texture *texture)
{
    if (asset_registry.count >= ASSET_REGISTRY_MAX) {
        SDL_Log("asset_registry_add: registry full, cannot add '%s'", id);
        return;
    }
    TextureEntry *e = &asset_registry.entries[asset_registry.count++];
    strncpy(e->id, id, sizeof(e->id) - 1);
    e->id[sizeof(e->id) - 1] = '\0';
    e->texture = texture;
}

SDL_Texture *asset_registry_get(const char *id)
{
    for (int i = 0; i < asset_registry.count; i++) {
        if (strcmp(asset_registry.entries[i].id, id) == 0)
            return asset_registry.entries[i].texture;
    }
    return NULL;
}

void asset_registry_destroy(void)
{
    for (int i = 0; i < asset_registry.count; i++)
        SDL_DestroyTexture(asset_registry.entries[i].texture);
    asset_registry.count = 0;
}
