#define STB_IMAGE_IMPLEMENTATION
#include "sarsa.h"

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path) 
{
    int w, h;
    unsigned char *pixels = stbi_load(path, &w, &h, NULL, 4);
    if (!pixels) {
        fprintf(stderr, "Failed to load texture '%s': %s\n", path, stbi_failure_reason());
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                         SDL_TEXTUREACCESS_STATIC, w, h);
    SDL_UpdateTexture(tex, NULL, pixels, w * 4);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    stbi_image_free(pixels);
    return tex;
}