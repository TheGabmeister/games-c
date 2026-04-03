#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include "platform.h"
#include <stdbool.h>

#define MAX_COLLECTIBLES 64

typedef enum {
    COLLECTIBLE_COIN,
    COLLECTIBLE_HEALTH,
    COLLECTIBLE_STAR,
    COLLECTIBLE_TYPE_COUNT
} CollectibleType;

typedef struct {
    rectangle rect;
    CollectibleType type;
    bool active;
} Collectible;

void collectibles_init(void);
void collectible_spawn(CollectibleType type, float x, float y);
void collectibles_update(rectangle player_rect, int *out_score, int *out_health);
void collectibles_draw(void);
int  collectibles_remaining(void);

#endif // COLLECTIBLE_H
