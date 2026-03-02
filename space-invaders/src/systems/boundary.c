#include "boundary.h"
#include "tags.h"
#include "../components/transform.h"
#include "../components/box_collider.h"
#include "../settings.h"

/* Terms: [Projectile(tag), Transform]
 * indices:      0              1     */
static ecs_query_t *boundary_query;

/* Terms: [Player(tag), Transform, BoxCollider]
 * indices:     0           1           2      */
static ecs_query_t *player_clamp_query;

void boundary_system_init(ecs_world_t *world)
{
    boundary_query = ecs_query(world, {
        .terms = {
            { .id = Projectile },
            { .id = ecs_id(Transform) }
        }
    });

    player_clamp_query = ecs_query(world, {
        .terms = {
            { .id = Player },
            { .id = ecs_id(Transform) },
            { .id = ecs_id(BoxCollider) }
        }
    });
}

void boundary_system_run(ecs_world_t *world)
{
    /* Delete projectiles that have left the screen vertically */
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

    /* Clamp player horizontally to screen edges */
    ecs_iter_t pit = ecs_query_iter(world, player_clamp_query);
    while (ecs_query_next(&pit))
    {
        Transform   *tr = ecs_field(&pit, Transform,   1);
        BoxCollider *bc = ecs_field(&pit, BoxCollider, 2);
        for (int i = 0; i < pit.count; i++)
        {
            float hw = bc[i].w * 0.5f;
            if (tr[i].position[0] - hw < 0.0f)
                tr[i].position[0] = hw;
            if (tr[i].position[0] + hw > (float)WINDOW_WIDTH)
                tr[i].position[0] = (float)WINDOW_WIDTH - hw;
        }
    }
}

void boundary_system_destroy(void)
{
    ecs_query_fini(boundary_query);
    ecs_query_fini(player_clamp_query);
}
