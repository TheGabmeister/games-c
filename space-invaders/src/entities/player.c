#include "entities/player.h"

ecs_entity_t player_spawn(ecs_world_t *world, float x, float y, SDL_Texture *tex) {
    ecs_entity_t e = ecs_new(world);
    ecs_set_name(world, e, "PlayerShip");
    ecs_add_id(world, e, Player);
    return e;
}
