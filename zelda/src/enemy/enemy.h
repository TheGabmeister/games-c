#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "../game_config.h"
#include "../anim.h"
#include <stdbool.h>

typedef enum EnemyType {
    ENEMY_SLIME = 0,
    ENEMY_BAT,
    ENEMY_SNAKE,
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
} Enemy;

typedef struct EnemyDef {
    int health;
    int contact_damage;
    float speed;
    bool ignores_walls;
    int hitbox_w;
    int hitbox_h;
    void (*update)(Enemy *self, Vector2 player_pos, const struct Screen *screen, float dt);
    void (*draw)(const Enemy *self);
} EnemyDef;

extern const EnemyDef enemy_defs[ENEMY_TYPE_COUNT];

void enemies_spawn(Enemy enemies[], int *count, const struct Screen *screen);
void enemies_update(Enemy enemies[], int count, Vector2 player_pos, const struct Screen *screen, float dt);
void enemies_draw(const Enemy enemies[], int count);
Rectangle enemy_hitbox(const Enemy *enemy);
void enemy_take_damage(Enemy *enemy, int damage);

void slime_update(Enemy *self, Vector2 player_pos, const struct Screen *screen, float dt);
void slime_draw(const Enemy *self);
void bat_update(Enemy *self, Vector2 player_pos, const struct Screen *screen, float dt);
void bat_draw(const Enemy *self);
void snake_update(Enemy *self, Vector2 player_pos, const struct Screen *screen, float dt);
void snake_draw(const Enemy *self);

#endif
