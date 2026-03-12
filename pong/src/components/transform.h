#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <flecs.h>
#include <engine.h>

typedef struct Transform
{
  vector2 position;
  float rotation;
} Transform;

ECS_COMPONENT_DECLARE(Transform);

#endif
