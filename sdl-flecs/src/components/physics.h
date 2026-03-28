#ifndef PHYSICS_COMPONENT_H
#define PHYSICS_COMPONENT_H

#include <flecs.h>
#include <box2d/box2d.h>

typedef struct Physics
{
  b2WorldId world;
} Physics;

ECS_COMPONENT_DECLARE(Physics);

#endif
