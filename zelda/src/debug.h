#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>

struct Game;

extern bool debug_enabled;

void debug_toggle(void);
void debug_draw_overlay(void);
void debug_draw_game(const struct Game *game);

#endif
