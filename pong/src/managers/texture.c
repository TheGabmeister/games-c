#include "../defines.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "../platform.h"

#include "texture.h"

//==============================================================================

static SDL_Texture *_textures[MAX_TEXTURES];
static SDL_Texture *_playfield;

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
  for (int i = 0; i < MAX_TEXTURES; ++i)
  {
    SDL_DestroyTexture(_textures[i]);
    _textures[i] = NULL;
  }
  SDL_DestroyTexture(_playfield);
  _playfield = NULL;
}

//------------------------------------------------------------------------------

static void _load(int id, const char *name)
{
  char filename[256];
  SDL_snprintf(filename, sizeof(filename), "./res/gfx/%s.png", name);
  _textures[id] = IMG_LoadTexture(get_renderer(), filename);
  if (!_textures[id])
    SDL_Log("Failed to load texture '%s': %s", filename, SDL_GetError());
}

//------------------------------------------------------------------------------

void texture_manager_init(ecs_world_t *world)
{
    ecs_atfini(world, _fini, NULL);
    _playfield = SDL_CreateTexture(get_renderer(), SDL_PIXELFORMAT_RGBA8888,
                                  SDL_TEXTUREACCESS_TARGET,
                                  RASTER_WIDTH, RASTER_HEIGHT);
    _load(TEXTURE_LOGO,   "logo");
    _load(TEXTURE_SHIP,   "ship");
    _load(TEXTURE_BULLET, "bullet");
    _load(TEXTURE_SHEEP,  "sheep");
    _load(TEXTURE_BLIP,   "blip");
}

//------------------------------------------------------------------------------

SDL_Texture *texture_manager_get(TextureName id)
{
  return (id == MAX_TEXTURES) ? NULL : _textures[id];
}

//------------------------------------------------------------------------------

SDL_Texture *texture_manager_playfield(void)
{
  return _playfield;
}
