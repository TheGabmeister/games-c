#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "game_config.h"
#include "anim.h"
#include "tilemap.h"
#include <stdint.h>

typedef enum PlayerState {
    PSTATE_IDLE,
    PSTATE_MOVING,
    PSTATE_ATTACKING,
    PSTATE_KNOCKBACK,
    PSTATE_USING_ITEM,
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
    int arrows;
    int arrow_capacity;
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
    int attack_timer;
    int attack_cooldown;
    int invuln_timer;
    int knockback_timer;
    Direction knockback_dir;
} Player;

extern const AnimDef player_idle_anims[DIR_COUNT];
extern const AnimDef player_walk_anims[DIR_COUNT];

struct Projectile;

void player_init(Player *player);
void player_update(Player *player, const Screen *screen,
                   struct Projectile *projectiles, int *projectile_count, float dt);
void player_draw(const Player *player);
Rectangle player_hitbox(const Player *player);
Rectangle player_sword_hitbox(const Player *player);
void player_take_damage(Player *player, int damage, Direction knockback_dir);

#endif
