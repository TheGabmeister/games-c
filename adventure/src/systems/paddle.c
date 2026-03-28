#include "../components/input.h"
#include "../components/transform.h"
#include "../components/paddle.h"

#include "paddle.h"

static const float PADDLE_SPEED = 300.0f;

void move_paddles(ecs_iter_t *it)
{
    Paddle *paddle = ecs_field(it, Paddle, 0);
    Transform *transform = ecs_field(it, Transform, 1);
    const Input *input = ecs_singleton_get(it->world, Input);

    for (int i = 0; i < it->count; ++i)
    {
        int dir = (paddle[i].player == 1) ? input->move_p1 : input->move_p2;
        transform[i].position.y += dir * PADDLE_SPEED * it->delta_time;
    }
}
