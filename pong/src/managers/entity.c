#include "../platform.h"
#include <SDL3/SDL.h>
#include "../components/transform.h"
#include "../components/sprite.h"
#include "../components/shape.h"

#include "entity.h"

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
}

//------------------------------------------------------------------------------

void entity_manager_init(ecs_world_t *world)
{
    ecs_atfini(world, _fini, NULL);

    ECS_TAG_DEFINE(world, DebugTag);
}


//------------------------------------------------------------------------------

ecs_entity_t spawn_debug(ecs_world_t *world, const char *text)
{
    ecs_entity_t entity = ecs_new(world);
    ecs_add(world, entity, DebugTag);
    return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t spawn_shape(ecs_world_t *world, ecs_entity_t parent, Shape shape, vector2 position)
{
    ecs_entity_t entity = ecs_new(world);
    ecs_set(world, entity, Shape, { .type = shape.type, .color = shape.color, .rectangle = shape.rectangle });
    ecs_set(world, entity, Transform, {.position = position});
    if (parent != 0)
        ecs_add_pair(world, entity, EcsChildOf, parent);
    return entity;
}

