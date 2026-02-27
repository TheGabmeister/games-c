#include "entities/bullet.h"
#include "components.h"

#define BULLET_SIZE 10.0f

ecs_entity_t bullet_spawn(ecs_world_t *world, const char *name,
                           float x, float y, float vy, ecs_entity_t tag) {
    ecs_entity_t b = ecs_new(world);
    ecs_set_name(world, b, name);
    ecs_add_id(world, b, tag);
    ecs_set(world, b, Position,    { .x = x,           .y = y           });
    ecs_set(world, b, Size,        { .w = BULLET_SIZE,  .h = BULLET_SIZE });
    ecs_set(world, b, Velocity,    { .x = 0,            .y = vy          });
    ecs_set(world, b, BoxCollider, { .w = BULLET_SIZE,  .h = BULLET_SIZE });
    ecs_set(world, b, Sprite,      { .texture = NULL, .color = {255, 255, 255, 255} });
    return b;
}

bool bullet_update(ecs_world_t *world, ecs_entity_t bullet, float screen_h) {
    if (!ecs_is_alive(world, bullet)) return false;
    const Position *p = ecs_get(world, bullet, Position);
    const Size     *s = ecs_get(world, bullet, Size);
    if (p->y + s->h < 0.0f || p->y > screen_h) {
        ecs_delete(world, bullet);
        return false;
    }
    return true;
}
