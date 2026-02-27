#ifndef ASSET_REGISTRY_H
#define ASSET_REGISTRY_H

#include <SDL3/SDL.h>

#define ASSET_REGISTRY_MAX 64

typedef struct {
    char         id[64];
    SDL_Texture *texture;
} TextureEntry;

typedef struct {
    TextureEntry entries[ASSET_REGISTRY_MAX];
    int          count;
} AssetRegistry;

extern AssetRegistry asset_registry;

void         asset_registry_add(const char *id, SDL_Texture *texture);
SDL_Texture *asset_registry_get(const char *id);
void         asset_registry_destroy(void);

#endif
