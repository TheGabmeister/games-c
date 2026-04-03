#include "game_state.h"
#include <SDL3/SDL.h>

static GameState states[MAX_GAME_STATES] = {0};
static GameStateID current_state = STATE_NONE;

void game_state_register(GameStateID id, GameState state)
{
    if (id < 0 || id >= MAX_GAME_STATES) {
        SDL_Log("game_state_register: invalid state ID %d", id);
        return;
    }
    states[id] = state;
}

void game_state_switch(GameStateID id)
{
    if (id < 0 || id >= MAX_GAME_STATES) {
        SDL_Log("game_state_switch: invalid state ID %d", id);
        return;
    }

    /* Clean up old state */
    if (current_state != STATE_NONE && states[current_state].cleanup)
        states[current_state].cleanup();

    current_state = id;

    /* Initialize new state */
    if (states[current_state].init)
        states[current_state].init();
}

void game_state_update(float dt)
{
    if (current_state != STATE_NONE && states[current_state].update)
        states[current_state].update(dt);
}

void game_state_draw(void)
{
    if (current_state != STATE_NONE && states[current_state].draw)
        states[current_state].draw();
}

void game_state_shutdown(void)
{
    if (current_state != STATE_NONE && states[current_state].cleanup)
        states[current_state].cleanup();
    current_state = STATE_NONE;
}

GameStateID game_state_current(void)
{
    return current_state;
}
