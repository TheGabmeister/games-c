#include <flecs.h>

#include "../components/hud_rect.h"

#include "hud.h"

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
}

//------------------------------------------------------------------------------

void hud_manager_init(ecs_world_t *world)
{
    ecs_atfini(world, _fini, NULL);
}

//------------------------------------------------------------------------------

ecs_entity_t hud_manager_spawn_rect(ecs_world_t *world, ecs_entity_t parent, rectangle rect, color tint)
{
    ecs_entity_t entity = ecs_new(world);
    ecs_set(world, entity, HudRect, {.rect = rect, .tint = tint});
    if (parent != 0)
        ecs_add_pair(world, entity, EcsChildOf, parent);
    return entity;
}
