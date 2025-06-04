#ifndef PLAYER_H
#define PLAYER_H

#include <SDL3/SDL.h>

typedef struct {
    float x, y;           // Position of the player (paddle)
    float width, height;  // Size of the paddle
    float speed;          // Movement speed
    SDL_FRect rect;       // SDL rectangle for rendering
} Player;

#endif // PLAYER_H