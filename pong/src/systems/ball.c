#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/ball.h"
#include "../components/shape.h"
#include "../components/goal_scored.h"
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

        // Goal (left/right wall) — emit event and respawn
        if (bx - bw <= 0 || bx + bw >= WINDOW_WIDTH)
        {
            GoalScored goal = { .player = (bx + bw >= WINDOW_WIDTH) ? 1 : 2 };
            ecs_enqueue(it->world, &(ecs_event_desc_t){
                .event = ecs_id(GoalScored),
                .entity = it->entities[i],
                .ids = &(ecs_type_t){
                    .array = (ecs_id_t[]){ ecs_id(GoalScored) },
                    .count = 1
                },
                .param = &goal
            });

            transform[i].position = ball[i].spawn;
            velocity[i].value.x   = -velocity[i].value.x;
        }
    }
}
