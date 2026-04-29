#ifndef NAVIGATION_H
#define NAVIGATION_H

struct Game;

void nav_load_screen(struct Game *game, int sx, int sy);
void nav_load_dungeon_room(struct Game *game, int rx, int ry);
void nav_load_cave(struct Game *game, const char *cave_name);
void nav_check_edge_transition(struct Game *game);
void nav_check_warp(struct Game *game);
void nav_position_at_return_warp(struct Game *game);

#endif
