#include "entities/enemy.h"
#include "entities/bullet.h"
#include "components.h"

#define BULLET_SIZE  10.0f
#define BULLET_SPEED 600.0f

ecs_entity_t enemy_spawn(ecs_world_t *world, float x, float y, SDL_Texture *tex) {
    ecs_entity_t e = ecs_new(world);
    ecs_set_name(world, e, "EnemyShip");
    ecs_add_id(world, e, Enemy);
    ecs_set(world, e, Position,    { .x = x,   .y = y   });
    ecs_set(world, e, Size,        { .w = 100,  .h = 100 });
    ecs_set(world, e, Velocity,    { .x = 0,    .y = 0   });
    ecs_set(world, e, BoxCollider, { .w = 100,  .h = 100 });
    ecs_set(world, e, Sprite,      { .texture = tex, .color = {255, 255, 255, 255} });
    ecs_set(world, e, Health,      { .current = 1, .max = 1 });
    return e;
}

ecs_entity_t enemy_update(ecs_world_t *world, ecs_entity_t enemy,
                           float dt, float *shoot_timer, bool can_shoot) {
    *shoot_timer -= dt;
    if (*shoot_timer > 0.0f) return 0;

    /* Always reset so the interval is consistent even when blocked */
    *shoot_timer = ENEMY_SHOOT_INTERVAL;
    if (!can_shoot) return 0;

    const Position *ep = ecs_get(world, enemy, Position);
    const Size     *es = ecs_get(world, enemy, Size);
    float bx = ep->x + es->w / 2.0f - BULLET_SIZE / 2.0f;
    float by = ep->y + es->h;
    return bullet_spawn(world, "EnemyBullet", bx, by, BULLET_SPEED, EnemyProjectile);
}
