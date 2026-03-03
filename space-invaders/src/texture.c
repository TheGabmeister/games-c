#include "texture.h"
#include <SDL3_image/SDL_image.h>

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path)
{
    SDL_Texture *tex = IMG_LoadTexture(renderer, path);
    if (!tex) {
        fprintf(stderr, "Failed to load texture '%s': %s\n", path, SDL_GetError());
        return NULL;
    }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}
