#ifndef VIEWPORT_COMPONENT_H
#define VIEWPORT_COMPONENT_H

#include <flecs.h>
#include <raylib.h>
#include <SDL3/SDL.h>

typedef struct Viewport
{
  SDL_Texture *raster;
  Vector2 size;
  Color background;
  Camera2D camera;
  SDL_FRect src;
  SDL_FRect dst;
  Vector2 origin;
  float rotation;
  Color color;
  Vector2 pointer;
  Vector2 handle;
  bool active;
} Viewport;

ECS_COMPONENT_DECLARE(Viewport);

#endif
