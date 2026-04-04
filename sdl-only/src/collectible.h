#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include "platform.h"
#include "collision.h"
#include "gamestate.h"

typedef enum {
    COLLECTIBLE_COIN,
    COLLECTIBLE_HEALTH,
    COLLECTIBLE_STAR,
    COLLECTIBLE_TYPE_COUNT
} CollectibleType;

void collectibles_init(void);
void collectible_spawn(CollectibleType type, float x, float y);
void collectibles_update(rectangle player_rect, CollisionFilter player_filter, GameState *state);
void collectibles_draw(void);
int  collectibles_remaining(void);

#endif // COLLECTIBLE_H
