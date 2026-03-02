#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#define ASSET_MANAGER_MAX 64
typedef struct SDL_Texture SDL_Texture;


typedef struct {
    char         id[64];
    SDL_Texture *texture;
} TextureEntry;

typedef struct {
    TextureEntry entries[ASSET_MANAGER_MAX];
    int          count;
} AssetManager;

extern AssetManager asset_manager;

void         asset_manager_add(const char *id, SDL_Texture *texture);
SDL_Texture *asset_manager_get(const char *id);
void         asset_manager_destroy(void);

#endif
