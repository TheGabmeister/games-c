#include "score.h"
#include "event_bus.h"

static Score _score;

static void _on_goal(const void *data)
{
    const GoalScoredData *goal = data;
    if (goal->player == 1)
        _score.p1++;
    else
        _score.p2++;
}

void score_init(void)
{
    _score = (Score){0};
    event_bus_subscribe(EVENT_GOAL_SCORED, _on_goal);
}

const Score *score_get(void)
{
    return &_score;
}
