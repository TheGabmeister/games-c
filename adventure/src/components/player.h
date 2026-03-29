#ifndef PLAYER_COMPONENT_H
#define PLAYER_COMPONENT_H

#include <flecs.h>

typedef struct Player
{
    ecs_entity_t carried_entity;
} Player;

ECS_COMPONENT_DECLARE(Player);

#endif
