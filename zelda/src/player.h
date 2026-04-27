#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "anim.h"
#include "tilemap.h"
#include <stdint.h>

typedef enum Direction {
    DIR_S = 0,
    DIR_N,
    DIR_E,
    DIR_W,
    DIR_COUNT
} Direction;

typedef enum PlayerState {
    PSTATE_IDLE,
    PSTATE_MOVING,
    PSTATE_COUNT
} PlayerState;

typedef struct Inventory {
    uint32_t items;
    int sword_tier;
    int shield_tier;
    int armor_tier;
    int rupees;
    int bombs;
    int bomb_capacity;
    int keys;
    ItemID equipped;
} Inventory;

typedef struct Player {
    Vector2 pos;
    Direction facing;
    PlayerState state;
    int state_timer;
    int health;
    int max_health;
    Anim anim;
    Inventory inventory;
} Player;

extern const AnimDef player_idle_anims[DIR_COUNT];
extern const AnimDef player_walk_anims[DIR_COUNT];

void player_init(Player *player);
void player_update(Player *player, const Screen *screen, float dt);
void player_draw(const Player *player);
Rectangle player_hitbox(const Player *player);

#endif
