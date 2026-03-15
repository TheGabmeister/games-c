#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include <flecs.h>
#include <engine.h>

#include "../components/physics.h"

void physics_manager_init(ecs_world_t *world);
void physics_manager_set_properties(ecs_world_t *world, float gravity, float damping);
void physics_ball(ecs_world_t *world, ecs_entity_t parent, float mass, float radius, vector2 position);
void physics_line(ecs_world_t *world, ecs_entity_t parent, vector2 from, vector2 to, float radius);
void physics_box(ecs_world_t *world, ecs_entity_t parent, vector2 position);
void physics_wedge(ecs_world_t *world, ecs_entity_t parent, vector2 position, int corner);

#ifdef DEBUG
cpSpaceDebugDrawOptions *physics_debug_options(void);
#endif

#endif
