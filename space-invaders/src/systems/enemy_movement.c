#include "enemy_movement.h"
#include "tags.h"
#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/box_collider.h"
#include "../settings.h"

#define ENEMY_DROP 24.0f

/* Terms: [Enemy(tag), Transform, Velocity, BoxCollider]
 * indices:     0           1         2          3       */
static ecs_query_t *enemy_move_query;

void enemy_movement_system_init(ecs_world_t *world)
{
    enemy_move_query = ecs_query(world, {
        .terms = {
            { .id = Enemy },
            { .id = ecs_id(Transform) },
            { .id = ecs_id(Velocity) },
            { .id = ecs_id(BoxCollider) }
        }
    });
}

void enemy_movement_system_run(ecs_world_t *world)
{
    /* Pass 1: check if any enemy has crossed a horizontal boundary. */
    bool hit_edge = false;
    {
        ecs_iter_t it = ecs_query_iter(world, enemy_move_query);
        while (!hit_edge && ecs_query_next(&it))
        {
            Transform   *tr = ecs_field(&it, Transform,   1);
            Velocity    *vl = ecs_field(&it, Velocity,    2);
            BoxCollider *bc = ecs_field(&it, BoxCollider, 3);
            for (int i = 0; i < it.count && !hit_edge; i++)
            {
                float x  = tr[i].position[0];
                float hw = bc[i].w * 0.5f;
                float vx = vl[i].x;
                if ((vx > 0.0f && x + hw >= (float)WINDOW_WIDTH) ||
                    (vx < 0.0f && x - hw <= 0.0f))
                    hit_edge = true;
            }
        }
        if (hit_edge) ecs_iter_fini(&it);
    }

    if (!hit_edge) return;

    /* Pass 2: reverse all enemy x-velocities and step everyone down. */
    {
        ecs_iter_t it = ecs_query_iter(world, enemy_move_query);
        while (ecs_query_next(&it))
        {
            Transform *tr = ecs_field(&it, Transform, 1);
            Velocity  *vl = ecs_field(&it, Velocity,  2);
            for (int i = 0; i < it.count; i++)
            {
                vl[i].x               = -vl[i].x;
                tr[i].position[1]    += ENEMY_DROP;
            }
        }
    }
}

void enemy_movement_system_destroy(void)
{
    ecs_query_fini(enemy_move_query);
}
