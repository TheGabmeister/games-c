#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <flecs.h>
#include <SDL3/SDL.h>

typedef enum TextureName
{
  TEXTURE_LOGO,
  TEXTURE_SHIP,
  TEXTURE_BULLET,
  TEXTURE_SHEEP,
  TEXTURE_BLIP,
  MAX_TEXTURES
} TextureName;

void texture_manager_init(ecs_world_t *world);
SDL_Texture *texture_manager_get(TextureName id);

#endif
