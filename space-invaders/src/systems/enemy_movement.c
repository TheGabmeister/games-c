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
static bool s_reached_bottom = false;

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
    s_reached_bottom = false;

    /* Pass 1: check if any enemy has crossed a horizontal boundary.
     * Also detect if any enemy has reached the bottom of the screen. */
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
                float sign = (vl[i].x < 0.0f) ? -1.0f : 1.0f;
                vl[i].x = sign * ENEMY_SPEED;
                float x  = tr[i].position[0];
                float hw = bc[i].w * 0.5f;
                float vx = vl[i].x;
                if ((vx > 0.0f && x + hw >= (float)WINDOW_WIDTH) ||
                    (vx < 0.0f && x - hw <= 0.0f))
                    hit_edge = true;
                if (tr[i].position[1] + bc[i].h * 0.5f >= (float)WINDOW_HEIGHT - 40.0f)
                    s_reached_bottom = true;
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
                vl[i].x               = (vl[i].x > 0.0f) ? -ENEMY_SPEED : ENEMY_SPEED;
                tr[i].position[1]    += ENEMY_DROP;
            }
        }
    }
}

void enemy_movement_system_destroy(void)
{
    ecs_query_fini(enemy_move_query);
}

bool enemy_movement_system_reached_bottom(void)
{
    return s_reached_bottom;
}
