#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H

#include <flecs.h>
#include <engine.h>

#include "physical.h"

typedef struct Collision
{
  ecs_entity_t entities[2];
  vector2 contacts[2];
  vector2 normal;
  float energy;
} Collision;

ECS_COMPONENT_DECLARE(Collision);

#endif
