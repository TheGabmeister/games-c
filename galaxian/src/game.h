/*
 * game.h — Game lifecycle interface
 *
 * Platform calls these to configure, run, and shut down the game
 * without knowing anything about game-specific logic.
 */

#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>

typedef struct GameConfig {
    int screen_width;
    int screen_height;
    const char *title;
    SDL_Color clear_color;
} GameConfig;

GameConfig game_config(void);
void game_init(void);
void game_update(float dt);
void game_draw(void);
void game_shutdown(void);

#endif // GAME_H
