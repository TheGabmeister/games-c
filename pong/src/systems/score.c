#include "../components/goal_scored.h"
#include "../components/score.h"

#include "score.h"

//==============================================================================

static void _on_goal_update_score(ecs_iter_t *it)
{
    const GoalScored *goal = it->param;
    Score *score = ecs_singleton_get_mut(it->world, Score);

    if (goal->player == 1)
        score->p1++;
    else
        score->p2++;
}

//------------------------------------------------------------------------------

void score_observers_init(ecs_world_t *world)
{
    ecs_observer(world, {
        .query.terms = {{ ecs_id(GoalScored) }},
        .events = { ecs_id(GoalScored) },
        .callback = _on_goal_update_score
    });
}
