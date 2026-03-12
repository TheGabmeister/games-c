#include "../defines.h"

#include "../components/transform.h"
#include "../components/sprite.h"

#include "../managers/texture.h"
#include "../managers/system.h"

#include <SDL3/SDL.h>
#include "../platform.h"

#include "render.h"

//==============================================================================
// Helpers
//==============================================================================

static inline rectangle _sdl_rect(rectangle r)
{
  return (rectangle){r.x, r.y, r.w, r.h};
}

static inline void _set_tint(SDL_Texture *tex, color c)
{
    SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(tex, c.a);
}


void render_sprites(ecs_iter_t *it)
{
    Sprite *sprite = ecs_field(it, Sprite, 0);
    Transform *spatial = ecs_field(it, Transform, 1);
    SDL_Renderer *renderer = get_renderer();
    SDL_Texture *playfield = texture_manager_playfield();
    SDL_SetRenderTarget(renderer, playfield);
    for (int i = 0; i < it->count; ++i)
    {
      rectangle src = _sdl_rect(sprite[i].src);
      float w = sprite[i].scale * sprite[i].src.w;
      float h = sprite[i].scale * sprite[i].src.h;
      rectangle dst = {spatial[i].position.x - w * 0.5f, spatial[i].position.y - h * 0.5f, w, h};
      SDL_RenderTextureRotated(renderer, sprite[i].texture, &src, &dst, spatial[i].rotation, NULL, SDL_FLIP_NONE);
    }
    SDL_SetRenderTarget(renderer, NULL);
}
