#ifndef GAME_H
#define GAME_H

#include "sarsa.h"

#define WINDOW_TITLE    "Space Invaders"
#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600

static SDL_Window   *window;
static SDL_Renderer *renderer;

void game_init();
void game_run();
void game_destroy();

void setup_window();
void load_level();

#endif