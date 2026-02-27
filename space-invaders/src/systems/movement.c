#include <flecs.h>
#include "movement.h"
#include "../components/transform.h"
#include "../components/velocity.h"

static ecs_query_t *movement_query;

void movement_system_init(ecs_world_t *world)
{
    movement_query = ecs_query(world, {
        .terms = {{ ecs_id(Transform) }, { ecs_id(Velocity) }}
    });
}

void movement_system_run(ecs_world_t *world, float dt)
{
    ecs_iter_t it = ecs_query_iter(world, movement_query);
    while (ecs_query_next(&it))
    {
        Transform *transforms = ecs_field(&it, Transform, 0);
        Velocity  *velocities = ecs_field(&it, Velocity,  1);
        for (int i = 0; i < it.count; i++)
        {
            transforms[i].position[0] += velocities[i].x * dt;
            transforms[i].position[1] += velocities[i].y * dt;
        }
    }
}

void movement_system_destroy(void)
{
    ecs_query_fini(movement_query);
}
