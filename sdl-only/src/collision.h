#ifndef COLLISION_H
#define COLLISION_H

#include "platform.h"
#include <stdbool.h>
#include <stdint.h>

// Collision layers (bitmask — up to 32 layers)
#define COLLISION_LAYER_PLAYER      (1 << 0)
#define COLLISION_LAYER_ENEMY       (1 << 1)
#define COLLISION_LAYER_COLLECTIBLE (1 << 2)
#define COLLISION_LAYER_PROJECTILE  (1 << 3)

typedef struct {
    uint32_t layer;  // what this entity is
    uint32_t mask;   // what it collides with
} CollisionFilter;

static inline bool rects_overlap(rectangle a, rectangle b)
{
    return a.x < b.x + b.w && a.x + a.w > b.x
        && a.y < b.y + b.h && a.y + a.h > b.y;
}

static inline bool collision_check(rectangle a, CollisionFilter fa,
                                   rectangle b, CollisionFilter fb)
{
    return (fa.layer & fb.mask) && (fb.layer & fa.mask) && rects_overlap(a, b);
}

#endif // COLLISION_H
