#include "entities/bullet.h"

#define BULLET_SIZE 10.0f

ecs_entity_t bullet_spawn(ecs_world_t *world, const char *name,
                           float x, float y, float vy, ecs_entity_t tag) {
    ecs_entity_t b = ecs_new(world);
    ecs_set_name(world, b, name);
    ecs_add_id(world, b, tag);
    return b;
}