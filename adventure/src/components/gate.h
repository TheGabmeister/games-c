#ifndef GATE_COMPONENT_H
#define GATE_COMPONENT_H

#include <flecs.h>
#include <stdbool.h>

typedef enum KeyType {
    KEY_GOLD  = 0,
    KEY_BLACK = 1,
    KEY_WHITE = 2,
    KEY_TYPE_COUNT
} KeyType;

typedef struct Gate
{
    int     room_id;
    int     direction;
    KeyType key_type;
    bool    open;
} Gate;

ECS_COMPONENT_DECLARE(Gate);

#endif
