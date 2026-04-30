#ifndef DUNGEON_ROOM_H
#define DUNGEON_ROOM_H

#include "items.h"
#include "textures.h"

struct Game;

void dungeon_check_locked_door(struct Game *game);
void dungeon_check_shutter_room(struct Game *game);
void dungeon_check_push_block(struct Game *game);
void dungeon_check_items(struct Game *game);
void dungeon_draw_items(const struct Game *game);
TextureID dungeon_item_texture(ItemType type);

#endif
