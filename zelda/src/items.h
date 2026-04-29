#ifndef ITEMS_H
#define ITEMS_H

typedef enum ItemType {
    ITEM_NONE = 0,
    ITEM_BOOMERANG,
    ITEM_BOW,
    ITEM_BOMB,
    ITEM_CANDLE,
    ITEM_KEY,
    ITEM_MAP,
    ITEM_COMPASS,
    ITEM_HEART_CONTAINER,
    ITEM_FRAGMENT,
    ITEM_TYPE_COUNT
} ItemType;

static const char *const item_type_names[ITEM_TYPE_COUNT] = {
    [ITEM_BOOMERANG]        = "boomerang",
    [ITEM_BOW]              = "bow",
    [ITEM_BOMB]             = "bomb",
    [ITEM_CANDLE]           = "candle",
    [ITEM_KEY]              = "key",
    [ITEM_MAP]              = "map",
    [ITEM_COMPASS]          = "compass",
    [ITEM_HEART_CONTAINER]  = "heart_container",
    [ITEM_FRAGMENT]         = "fragment",
};

static const char *const item_display_names[ITEM_TYPE_COUNT] = {
    [ITEM_BOOMERANG]        = "BOOMERANG",
    [ITEM_BOW]              = "BOW",
    [ITEM_BOMB]             = "BOMB",
    [ITEM_CANDLE]           = "CANDLE",
    [ITEM_KEY]              = "KEY",
    [ITEM_MAP]              = "DUNGEON MAP",
    [ITEM_COMPASS]          = "COMPASS",
    [ITEM_HEART_CONTAINER]  = "HEART CONTAINER",
    [ITEM_FRAGMENT]         = "RELIC FRAGMENT",
};

#endif
