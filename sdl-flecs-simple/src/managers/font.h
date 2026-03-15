#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include <flecs.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef enum FontName
{
  FONT_CLOVER,
  MAX_FONTS
} FontName;

void font_manager_init(ecs_world_t *world);
TTF_Font *font_manager_get(FontName id);

#endif
