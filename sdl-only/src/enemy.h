#ifndef ENEMY_H
#define ENEMY_H

#include "platform.h"
#include "gamestate.h"
#include <stdbool.h>

#define MAX_ENEMIES 32

typedef enum {
    ENEMY_WANDERER,
    ENEMY_CHASER,
    ENEMY_BOUNCER,
    ENEMY_TYPE_COUNT
} EnemyType;

typedef struct {
    rectangle rect;
    vector2 velocity;
    EnemyType type;
    float speed;
    float timer;
    bool active;
} Enemy;

void enemies_init(void);
void enemy_spawn(EnemyType type, float x, float y);
void enemies_update(float dt, rectangle player_rect, GameState *state);
void enemies_draw(void);

#endif // ENEMY_H
