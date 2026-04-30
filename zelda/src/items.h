#ifndef ITEMS_H
#define ITEMS_H

typedef enum ItemType {
    ITEM_NONE = 0,
    ITEM_BOOMERANG,
    ITEM_BOW,
    ITEM_BOMB,
    ITEM_CANDLE,
    ITEM_ARROW,
    ITEM_SHIELD,
    ITEM_RAFT,
    ITEM_LADDER,
    ITEM_BRACELET,
    ITEM_FOOD,
    ITEM_LETTER,
    ITEM_POTION,
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
    [ITEM_ARROW]            = "arrows",
    [ITEM_SHIELD]           = "shield",
    [ITEM_RAFT]             = "raft",
    [ITEM_LADDER]           = "ladder",
    [ITEM_BRACELET]         = "bracelet",
    [ITEM_FOOD]             = "food",
    [ITEM_LETTER]           = "letter",
    [ITEM_POTION]           = "potion",
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
    [ITEM_ARROW]            = "ARROWS",
    [ITEM_SHIELD]           = "SHIELD",
    [ITEM_RAFT]             = "RAFT",
    [ITEM_LADDER]           = "LADDER",
    [ITEM_BRACELET]         = "BRACELET",
    [ITEM_FOOD]             = "FOOD",
    [ITEM_LETTER]           = "LETTER",
    [ITEM_POTION]           = "POTION",
    [ITEM_KEY]              = "KEY",
    [ITEM_MAP]              = "DUNGEON MAP",
    [ITEM_COMPASS]          = "COMPASS",
    [ITEM_HEART_CONTAINER]  = "HEART CONTAINER",
    [ITEM_FRAGMENT]         = "RELIC FRAGMENT",
};

static inline const char *item_display_name(ItemType item) {
    if (item <= ITEM_NONE || item >= ITEM_TYPE_COUNT || !item_display_names[item])
        return "ITEM";
    return item_display_names[item];
}

static inline unsigned int item_bit(ItemType item) {
    return 1u << (unsigned int)item;
}

#endif
