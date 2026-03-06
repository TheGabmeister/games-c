#ifndef WINDOW_COMPONENT_H
#define WINDOW_COMPONENT_H

#include <flecs.h>
#include <engine.h>

typedef struct Window
{
  const char *name;
  rectangle bounds;
  unsigned int flags;
  unsigned int max;
  unsigned int button_height;
} Window;

ECS_COMPONENT_DECLARE(Window);

#endif
