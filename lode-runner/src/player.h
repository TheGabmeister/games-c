#ifndef PLAYER_H
#define PLAYER_H

#include "world.h"
#include "raylib.h"
#include <stdbool.h>

typedef enum Dir {
    DIR_LEFT = -1,
    DIR_RIGHT = 1
} Dir;

typedef enum PlayerState {
    PSTATE_WALK,
    PSTATE_CLIMB,
    PSTATE_HANG,
    PSTATE_FALL,
    PSTATE_DIG,
    PSTATE_DEAD
} PlayerState;

typedef struct Actor {
    int tile_r;
    int tile_c;
    int dst_r;
    int dst_c;
    float t;
    Dir facing;
} Actor;

typedef struct Player {
    Actor actor;
    PlayerState state;
    float dig_lock_timer;
    float death_timer;
} Player;

typedef struct PlayerTickResult {
    bool collected_gold;
    bool committed_new_tile;
    bool died;
    bool reached_exit;
    bool dug_brick;
} PlayerTickResult;

void player_spawn(Player *player, const World *world);
void player_start_death(Player *player);
PlayerTickResult player_update(Player *player, World *world, float dt);
Vector2 player_pixel_position(const Player *player);
const char *player_state_name(PlayerState state);

#endif
