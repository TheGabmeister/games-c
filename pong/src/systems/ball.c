#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/ball.h"
#include "../components/paddle.h"
#include "../components/shape.h"
#include "../defines.h"

#include "ball.h"

//==============================================================================

void update_ball(ecs_iter_t *it)
{
    Transform *transform = ecs_field(it, Transform, 0);
    Velocity  *velocity  = ecs_field(it, Velocity,  1);
    Ball      *ball      = ecs_field(it, Ball,      2);
    Shape     *shape     = ecs_field(it, Shape,     3);

    for (int i = 0; i < it->count; ++i)
    {
        // Move
        transform[i].position.x += velocity[i].value.x * it->delta_time;
        transform[i].position.y += velocity[i].value.y * it->delta_time;

        float bx = transform[i].position.x;
        float by = transform[i].position.y;
        float bw = shape[i].rectangle.width  * 0.5f;
        float bh = shape[i].rectangle.height * 0.5f;

        // Wall bounce (top/bottom)
        if (by - bh <= 0 || by + bh >= WINDOW_HEIGHT)
            velocity[i].value.y = -velocity[i].value.y;

        // Goal (left/right wall) — respawn at center
        if (bx - bw <= 0 || bx + bw >= WINDOW_WIDTH)
        {
            transform[i].position = ball[i].spawn;
            velocity[i].value.x   = -velocity[i].value.x;
            continue;
        }

        // Paddle bounce
        ecs_query_t *q = ecs_query(it->world, {
            .terms = {
                { ecs_id(Paddle),    .inout = EcsIn },
                { ecs_id(Transform), .inout = EcsIn },
                { ecs_id(Shape),     .inout = EcsIn },
            }
        });

        ecs_iter_t pit = ecs_query_iter(it->world, q);
        while (ecs_query_next(&pit))
        {
            Transform *pt = ecs_field(&pit, Transform, 1);
            Shape     *ps = ecs_field(&pit, Shape,     2);

            for (int j = 0; j < pit.count; ++j)
            {
                float pw = ps[j].rectangle.width  * 0.5f;
                float ph = ps[j].rectangle.height * 0.5f;
                float px = pt[j].position.x;
                float py = pt[j].position.y;

                bool overlap_x = bx + bw > px - pw && bx - bw < px + pw;
                bool overlap_y = by + bh > py - ph && by - bh < py + ph;

                if (overlap_x && overlap_y)
                    velocity[i].value.x = -velocity[i].value.x;
            }
        }
        ecs_query_fini(q);
    }
}
