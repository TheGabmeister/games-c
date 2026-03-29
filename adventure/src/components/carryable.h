#ifndef CARRYABLE_COMPONENT_H
#define CARRYABLE_COMPONENT_H

#include <flecs.h>
#include <stdbool.h>

typedef struct Carryable
{
    bool player_can_carry;
    bool bat_can_carry;
} Carryable;

ECS_COMPONENT_DECLARE(Carryable);

#endif
