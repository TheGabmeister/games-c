#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "../game_config.h"
#include "../anim.h"
#include <stdbool.h>
#include <stdint.h>

struct Projectile;
struct Game;

typedef enum EnemyType {
    ENEMY_SLIME = 0,
    ENEMY_BAT,
    ENEMY_SNAKE,
    ENEMY_ROCK_SPITTER,
    ENEMY_SPEAR_THROWER,
    ENEMY_DRAGON,
    ENEMY_TYPE_COUNT
} EnemyType;

typedef enum EnemyState {
    ESTATE_IDLE = 0,
    ESTATE_MOVING,
    ESTATE_CHARGING,
    ESTATE_DEAD,
} EnemyState;

struct Screen;

typedef struct Enemy {
    EnemyType type;
    int variant;
    Vector2 pos;
    Direction facing;
    EnemyState state;
    int state_timer;
    int health;
    bool active;
    int invuln_timer;
    Anim anim;
    Vector2 velocity;
    int ai_timer;
    int subtype;
    uint8_t extra[16];
} Enemy;

typedef struct EnemyDef {
    const char *name;
    int health;
    int contact_damage;
    int defense;
    float speed;
    bool ignores_walls;
    int hitbox_w;
    int hitbox_h;
    int drop_group;
    void (*update)(Enemy *self, Vector2 player_pos, const struct Screen *screen,
                   struct Projectile *projectiles, int *projectile_count, float dt);
    void (*draw)(const Enemy *self);
    void (*on_spawn)(Enemy *self);
    void (*on_death)(Enemy *self, struct Game *game);
    bool (*on_hit)(Enemy *self, struct Game *game, int damage);
} EnemyDef;

extern EnemyDef enemy_defs[ENEMY_TYPE_COUNT];
void enemy_defs_init(void);

void enemies_spawn(Enemy enemies[], int *count, const struct Screen *screen);
void enemies_update(Enemy enemies[], int count, Vector2 player_pos, const struct Screen *screen,
                    struct Projectile *projectiles, int *projectile_count, float dt);
void enemies_draw(const Enemy enemies[], int count);
Rectangle enemy_hitbox(const Enemy *enemy);
void enemy_take_damage(Enemy *enemy, int damage);

EnemyDef slime_def(void);
EnemyDef bat_def(void);
EnemyDef snake_def(void);
EnemyDef rock_spitter_def(void);
EnemyDef spear_thrower_def(void);
EnemyDef dragon_def(void);

#endif
