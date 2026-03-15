#ifndef SCORE_LABEL_COMPONENT_H
#define SCORE_LABEL_COMPONENT_H

#include <flecs.h>

typedef struct ScoreLabel
{
    int player;
} ScoreLabel;

ECS_COMPONENT_DECLARE(ScoreLabel);

#endif
