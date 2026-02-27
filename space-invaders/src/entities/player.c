#include "entities/player.h"
#include "components.h"

ecs_entity_t player_spawn(ecs_world_t *world, float x, float y, SDL_Texture *tex) {
    ecs_entity_t e = ecs_new(world);
    ecs_set_name(world, e, "PlayerShip");
    ecs_add_id(world, e, Player);
    ecs_set(world, e, Position,    { .x = x,   .y = y   });
    ecs_set(world, e, Size,        { .w = 100,  .h = 100 });
    ecs_set(world, e, Velocity,    { .x = 0,    .y = 0   });
    ecs_set(world, e, BoxCollider, { .w = 100,  .h = 100 });
    ecs_set(world, e, Sprite,      { .texture = tex, .color = {255, 255, 255, 255} });
    ecs_set(world, e, Health,      { .current = 3, .max = 3 });
    return e;
}
