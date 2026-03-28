#ifndef VIEWPORT_COMPONENT_H
#define VIEWPORT_COMPONENT_H

#include <flecs.h>
#include "platform.h"
#include <SDL3/SDL.h>

typedef struct Viewport
{
  SDL_Texture *raster;
  vector2 size;
  color background;
  camera cam;
  rectangle src;
  rectangle dst;
  vector2 origin;
  float rotation;
  color color;
  vector2 pointer;
  vector2 handle;
  bool active;
} Viewport;

ECS_COMPONENT_DECLARE(Viewport);

#endif
