#ifndef INTERACTION_SYSTEM_H
#define INTERACTION_SYSTEM_H
#include <flecs.h>
void resolve_player_interactions(ecs_iter_t *it);
void resolve_gate_state(ecs_iter_t *it);
#endif
