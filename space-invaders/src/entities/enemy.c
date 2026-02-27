#include "entities/enemy.h"
#include "entities/bullet.h"

#define BULLET_SIZE  10.0f
#define BULLET_SPEED 600.0f

ecs_entity_t enemy_spawn(ecs_world_t *world, float x, float y, SDL_Texture *tex) {
    ecs_entity_t e = ecs_new(world);
    ecs_set_name(world, e, "EnemyShip");
    ecs_add_id(world, e, Enemy);
    return e;
}