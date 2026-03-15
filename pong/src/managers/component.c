#include "../components/transform.h"
#include "../components/input.h"
#include "../components/sprite.h"
#include "../components/audible.h"

#include "component.h"

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
}

//------------------------------------------------------------------------------

void component_manager_init(ecs_world_t *world)
{
    ecs_atfini(world, _fini, NULL);

    ECS_COMPONENT_DEFINE(world, Transform);
    ECS_COMPONENT_DEFINE(world, Sprite);
    ECS_COMPONENT_DEFINE(world, Input);
    ECS_COMPONENT_DEFINE(world, Audible);

    ecs_add_id(world, ecs_id(Input), EcsSingleton);

}
