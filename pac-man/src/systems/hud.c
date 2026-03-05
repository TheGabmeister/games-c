#include <SDL3/SDL.h>
#include <engine.h>

#include "../components/hud_rect.h"

#include "../managers/texture.h"

#include "hud.h"

//==============================================================================

void render_hud(ecs_iter_t *it)
{
  HudRect *hud_rect = ecs_field(it, HudRect, 0);
  SDL_Renderer *renderer = get_renderer();
  SDL_Texture *playfield = texture_manager_playfield();
  SDL_SetRenderTarget(renderer, playfield);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  for (int i = 0; i < it->count; ++i)
  {
    SDL_SetRenderDrawColor(renderer, hud_rect[i].tint.r, hud_rect[i].tint.g, hud_rect[i].tint.b, hud_rect[i].tint.a);
    SDL_RenderFillRect(renderer, &hud_rect[i].rect);
  }
  SDL_SetRenderTarget(renderer, NULL);
}
