#ifndef RENDERABLE_COMPONENT_H
#define RENDERABLE_COMPONENT_H

#include <flecs.h>
#include <SDL3/SDL.h>

typedef struct Renderable
{
  SDL_Texture *texture;
  SDL_FRect src;
  float scale;
} Renderable;

ECS_COMPONENT_DECLARE(Renderable);

#endif
