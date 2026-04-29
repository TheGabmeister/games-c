#ifndef PICKUP_H
#define PICKUP_H

#include "raylib.h"
#include "game_config.h"
#include <stdbool.h>

#define MAX_PICKUPS 16
#define PICKUP_LIFETIME 600

typedef enum PickupType {
    PICKUP_RUPEE = 0,
    PICKUP_HEART,
    PICKUP_BOMB,
    PICKUP_ARROW,
    PICKUP_TYPE_COUNT
} PickupType;

typedef struct Pickup {
    PickupType type;
    Vector2 pos;
    bool active;
    int lifetime;
    int value;
} Pickup;

void pickup_spawn(Pickup pickups[], int *count, PickupType type, Vector2 pos);
void pickups_update(Pickup pickups[], int count);
void pickups_draw(const Pickup pickups[], int count);
void pickups_clear(Pickup pickups[], int *count);
Rectangle pickup_hitbox(const Pickup *pickup);

#endif
