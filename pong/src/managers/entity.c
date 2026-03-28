#include "../platform.h"
#include <SDL3/SDL.h>
#include "../components/transform.h"
#include "../components/sprite.h"
#include "../components/shape.h"

#include "entity.h"

void entity_manager_init(ecs_world_t *world)
{
    ECS_TAG_DEFINE(world, DebugTag);
}


//------------------------------------------------------------------------------

ecs_entity_t entity_manager_spawn_debug(ecs_world_t *world, const char *text)
{
    (void)text;
    ecs_entity_t entity = ecs_new(world);
    ecs_add(world, entity, DebugTag);
    return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t entity_manager_spawn_shape(ecs_world_t *world, ecs_entity_t parent, Shape shape, vector2 position)
{
    ecs_entity_t entity = ecs_new(world);
    ecs_set(world, entity, Shape, { .type = shape.type, .color = shape.color, .rectangle = shape.rectangle });
    ecs_set(world, entity, Transform, {.position = position});
    if (parent != 0)
        ecs_add_pair(world, entity, EcsChildOf, parent);
    return entity;
}

