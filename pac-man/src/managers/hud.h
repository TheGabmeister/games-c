#ifndef HUD_MANAGER_H
#define HUD_MANAGER_H

#include <flecs.h>
#include <engine.h>

void hud_manager_init(ecs_world_t *world);

ecs_entity_t hud_manager_spawn_rect(ecs_world_t *world, ecs_entity_t parent, rectangle rect, color tint);

#endif
