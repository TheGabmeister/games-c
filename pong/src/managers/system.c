#include "system.h"

#include "../components/shape.h"
#include "../systems/render.h"
#include "../systems/input.h"
#include "../systems/paddle.h"
#include "../systems/ball.h"
#include "../systems/collision.h"
#include "../components/paddle.h"
#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/ball.h"
#include "../components/collider.h"
#include "../components/collision.h"

#ifdef DEBUG
#include "../systems/debug.h"
#endif

static void _fini(ecs_world_t *world, void *context)
{
}


void system_manager_init(ecs_world_t *world)
{
    ecs_atfini(world, _fini, NULL);
   ECS_SYSTEM(world, process_input,    EcsOnLoad,     [out] Input);
   ECS_SYSTEM(world, collision_clear,  EcsPreUpdate,  [in]  Collision);
   ECS_SYSTEM(world, move_paddles,    EcsOnUpdate,   [in]  Paddle,    [inout] Transform);
   ECS_SYSTEM(world, update_ball,     EcsOnUpdate,   [inout] Transform, [inout] Velocity, [in] Ball, [in] Shape);
   ECS_SYSTEM(world, collision_detect, EcsPostUpdate, 0);
   ECS_SYSTEM(world, apply_bounce,    EcsOnValidate, [inout] Velocity, [in] Collision);

//    ECS_SYSTEM(world, physics_update, EcsOnValidate, [in] Time, [inout] Physics);
//    ECS_SYSTEM(world, physics_collide, EcsOnValidate, [in] Physics, [inout] Collision);
#ifdef DEBUG
    ECS_SYSTEM(world, debug_input, EcsPostUpdate, [in] Input, [out] Debug);
    ECS_SYSTEM(world, debug_scene, EcsPostUpdate, [in] Scene, [in] Stateful, [out] Debug);
#endif
    ECS_SYSTEM(world, render_clear, EcsPreStore, 0);
    ECS_SYSTEM(world, render_shapes, EcsOnStore, [in] Shape, [in] Transform);
    ECS_SYSTEM(world, render_sprites, EcsOnStore, [in] Sprite, [in] Transform);
    ECS_SYSTEM(world, render_colliders, EcsOnStore, [in] Collider, [in] Transform);
#ifdef DEBUG
    ECS_SYSTEM(world, debug_physics, EcsOnStore, [in] Physics, [in] Debug, [inout] Viewport);
    ECS_SYSTEM(world, debug_render, EcsOnStore, [in] Label, [in] Debug, DebugTag);
    ECS_SYSTEM(world, debug_display, EcsOnStore, [in] Display, [in] Debug);
#endif
    
}