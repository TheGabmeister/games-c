#include "../components/label.h"
#include "../components/score.h"
#include "../components/score_label.h"

#include "score.h"

#include <SDL3/SDL.h>

//==============================================================================

void sync_score_labels(ecs_iter_t *it)
{
    ScoreLabel *score_label = ecs_field(it, ScoreLabel, 0);
    Label      *label       = ecs_field(it, Label,      1);
    const Score *score      = ecs_singleton_get(it->world, Score);

    for (int i = 0; i < it->count; ++i)
    {
        int value = (score_label[i].player == 1) ? score->p1 : score->p2;
        SDL_snprintf(label[i].text, sizeof(label[i].text), "%d", value);
    }
}
