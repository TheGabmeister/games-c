#ifndef GUARD_H
#define GUARD_H

#include "player.h"
#include "world.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum GuardState {
    GSTATE_WALK,
    GSTATE_CLIMB,
    GSTATE_HANG,
    GSTATE_FALL,
    GSTATE_TRAPPED,
    GSTATE_RESPAWN
} GuardState;

typedef struct Guard {
    Actor actor;
    GuardState state;
    float hole_timer;
    float respawn_timer;
    int spawn_r;
    int spawn_c;
    int prev_r;
    int prev_c;
    uint32_t seed;
    bool carries_gold;
    bool active;
} Guard;

typedef struct GuardTickResult {
    bool picked_up_gold;
    bool dropped_gold;
    bool died_in_refill;
    bool fell_in_hole;
    bool respawned;
    bool committed_new_tile;
} GuardTickResult;

void guards_spawn_from_world(Guard guards[MAX_GUARDS], const World *world);
bool guard_kill_in_refill(Guard *guard, World *world);
GuardTickResult guard_update(Guard *guard, int guard_index, const Guard guards[MAX_GUARDS], World *world, const Player *player, const PursuitDir pursuit[GRID_ROWS][GRID_COLS], float dt);
bool guard_can_catch_player(const Guard *guard, const Player *player);
const char *guard_state_name(GuardState state);

#endif
