#ifndef ITEM_COMPONENT_H
#define ITEM_COMPONENT_H

#include <flecs.h>

typedef enum ItemType {
    ITEM_SWORD,
    ITEM_GOLD_KEY,
    ITEM_BLACK_KEY,
    ITEM_WHITE_KEY,
    ITEM_MAGNET,
    ITEM_BRIDGE,
    ITEM_CHALICE,
    ITEM_SECRET_DOT,
    ITEM_TYPE_COUNT
} ItemType;

typedef struct Item
{
    ItemType type;
} Item;

ECS_COMPONENT_DECLARE(Item);

#endif
