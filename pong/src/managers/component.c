#include "../components/transform.h"
#include "../components/input.h"
#include "../components/sprite.h"
#include "../components/audible.h"
#include "../components/velocity.h"
#include "../components/shape.h"
#include "../components/ball.h"
#include "../components/paddle.h"
#include "../components/score.h"
#include "../components/collider.h"
#include "../components/collision.h"
#include "../components/goal_scored.h"
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
    ECS_COMPONENT_DEFINE(world, Velocity);
    ECS_COMPONENT_DEFINE(world, Shape);
    ECS_COMPONENT_DEFINE(world, Ball);
    ECS_COMPONENT_DEFINE(world, Paddle);
    ECS_COMPONENT_DEFINE(world, Score);
    ECS_COMPONENT_DEFINE(world, Collider);
    ECS_COMPONENT_DEFINE(world, Collision);
    ECS_COMPONENT_DEFINE(world, GoalScored);
    ecs_singleton_set(world, Input, {0});
    ecs_singleton_set(world, Score, {0});

}
