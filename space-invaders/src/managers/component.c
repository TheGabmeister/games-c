#include "../components/transform.h"
#include "../components/sprite.h"
#include "../components/velocity.h"
#include "../components/box_collider.h"
#include "../components/health.h"

void component_manager_init(ecs_world_t *world)
{
    ECS_COMPONENT_DEFINE(world, Transform);
    ECS_COMPONENT_DEFINE(world, Sprite);
    ECS_COMPONENT_DEFINE(world, Velocity);
    ECS_COMPONENT_DEFINE(world, BoxCollider);
    ECS_COMPONENT_DEFINE(world, Health);
}
