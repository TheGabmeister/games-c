#include "boundary.h"
#include "tags.h"
#include "../components/transform.h"
#include "../settings.h"

static ecs_query_t *boundary_query;

void boundary_system_init(ecs_world_t *world)
{
    boundary_query = ecs_query(world, {
        .terms = {
            { .id = Projectile },
            { .id = ecs_id(Transform) }
        }
    });
}

void boundary_system_run(ecs_world_t *world)
{
    ecs_entity_t to_delete[64];
    int          count = 0;

    ecs_iter_t it = ecs_query_iter(world, boundary_query);
    while (ecs_query_next(&it))
    {
        Transform *transforms = ecs_field(&it, Transform, 1);
        for (int i = 0; i < it.count; i++)
        {
            float y = transforms[i].position[1];
            if ((y < 0.0f || y > (float)WINDOW_HEIGHT) && count < 64)
                to_delete[count++] = it.entities[i];
        }
    }

    for (int i = 0; i < count; i++)
        ecs_delete(world, to_delete[i]);
}

void boundary_system_destroy(void)
{
    ecs_query_fini(boundary_query);
}
