#ifndef SHOP_H
#define SHOP_H

#include "game_config.h"
#include "tilemap.h"

typedef struct ShopState {
    CaveShopMeta shop;
    int cursor;
    char message[CAVE_TEXT_MAX];
} ShopState;

struct Game;

void shop_open(struct Game *game, const CaveShopMeta *shop);
void shop_update(struct Game *game);
void shop_draw(const ShopState *state, const struct Game *game);

#endif
