#ifndef DISPLAY_COMPONENT_H
#define DISPLAY_COMPONENT_H

#include <SDL3/SDL.h>
#include <flecs.h>

typedef struct Display
{
  float scale;
  SDL_Color border;
  SDL_Color background;
  SDL_FRect raster;
  SDL_FRect window;
  SDL_FRect screen;
} Display;

ECS_COMPONENT_DECLARE(Display);

#endif
