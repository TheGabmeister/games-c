#ifndef DUNGEON_INTERACT_H
#define DUNGEON_INTERACT_H

#include "dungeon.h"
#include "textures.h"

struct Game;

void dungeon_check_locked_door(struct Game *game);
void dungeon_check_shutter_room(struct Game *game);
void dungeon_check_push_block(struct Game *game);
void dungeon_check_items(struct Game *game);
void dungeon_draw_items(const struct Game *game);
TextureID dungeon_item_texture(DungeonItemType type);
Color dungeon_item_fallback_color(DungeonItemType type);

#endif
