#include "../components/goal_scored.h"
#include "../components/score.h"
#include "../components/score_label.h"
#include "../components/label.h"

#include "score.h"

#include <SDL3/SDL.h>

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

static void _on_goal_update_labels(ecs_iter_t *it)
{
    const Score *score = ecs_singleton_get(it->world, Score);

    ecs_query_t *q = ecs_query(it->world, {
        .terms = {
            { ecs_id(ScoreLabel), .inout = EcsIn },
            { ecs_id(Label),      .inout = EcsInOut },
        }
    });

    ecs_iter_t qit = ecs_query_iter(it->world, q);
    while (ecs_query_next(&qit))
    {
        ScoreLabel *sl = ecs_field(&qit, ScoreLabel, 0);
        Label      *lb = ecs_field(&qit, Label,      1);

        for (int j = 0; j < qit.count; ++j)
        {
            int value = (sl[j].player == 1) ? score->p1 : score->p2;
            SDL_snprintf(lb[j].text, sizeof(lb[j].text), "%d", value);
        }
    }
    ecs_query_fini(q);
}

//------------------------------------------------------------------------------

void score_observers_init(ecs_world_t *world)
{
    ecs_observer(world, {
        .query.terms = {{ ecs_id(GoalScored) }},
        .events = { ecs_id(GoalScored) },
        .callback = _on_goal_update_score
    });

    ecs_observer(world, {
        .query.terms = {{ ecs_id(GoalScored) }},
        .events = { ecs_id(GoalScored) },
        .callback = _on_goal_update_labels
    });
}
