#ifndef HUD_RECT_COMPONENT_H
#define HUD_RECT_COMPONENT_H

#include <flecs.h>
#include <engine.h>

typedef struct HudRect
{
  rectangle rect;
  color tint;
} HudRect;

ECS_COMPONENT_DECLARE(HudRect);

#endif
