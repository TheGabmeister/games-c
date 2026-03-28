#include "../components/transform.h"
#include "../components/input.h"
#include "../components/sprite.h"
#include "../components/velocity.h"
#include "../components/shape.h"
#include "../components/ball.h"
#include "../components/paddle.h"
#include "../components/collider.h"
#include "../components/collision.h"
#include "component.h"

void component_manager_init(ecs_world_t *world)
{
    ECS_COMPONENT_DEFINE(world, Transform);
    ECS_COMPONENT_DEFINE(world, Sprite);
    ECS_COMPONENT_DEFINE(world, Input);
    ECS_COMPONENT_DEFINE(world, Velocity);
    ECS_COMPONENT_DEFINE(world, Shape);
    ECS_COMPONENT_DEFINE(world, Ball);
    ECS_COMPONENT_DEFINE(world, Paddle);
    ECS_COMPONENT_DEFINE(world, Collider);
    ECS_COMPONENT_DEFINE(world, Collision);
    ecs_singleton_set(world, Input, {0});
}
