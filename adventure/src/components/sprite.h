#ifndef RENDERABLE_COMPONENT_H
#define RENDERABLE_COMPONENT_H

#include <flecs.h>
#include <SDL3/SDL.h>

typedef struct Sprite
{
  SDL_Texture *texture;
  SDL_FRect src;
  float scale;
} Sprite;

ECS_COMPONENT_DECLARE(Sprite);

#endif
