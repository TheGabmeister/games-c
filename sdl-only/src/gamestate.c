#include "gamestate.h"

static GameState state;

GameState *gamestate_get(void)
{
    return &state;
}

void gamestate_reset(void)
{
    state = (GameState){
        .score  = 0,
        .health = 3
    };
}
