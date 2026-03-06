#ifndef TINTED_COMPONENT_H
#define TINTED_COMPONENT_H

#include <flecs.h>
#include <engine.h>

typedef struct Tinted
{
  color tint;
} Tinted;

ECS_COMPONENT_DECLARE(Tinted);

#endif
