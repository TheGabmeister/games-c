#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "game_types.h"
#include <stdint.h>

// Collision layers (bitmask)
#define COLLISION_LAYER_PLAYER   (1 << 0)
#define COLLISION_LAYER_BULLET   (1 << 1)
#define COLLISION_LAYER_ASTEROID (1 << 2)

// Entity tags for collision dispatch
typedef enum CollisionTag {
    COLLISION_TAG_SHIP = 0,
    COLLISION_TAG_BULLET,
    COLLISION_TAG_ASTEROID,
    COLLISION_TAG_COUNT
} CollisionTag;

void CollisionsUpdate(GameContext *ctx);

#endif
