/*
 * Galaxian — Game Flow
 */

#include "game.h"
#include "game_state.h"
#include "galaxian.h"

GameConfig game_config(void)
{
    return (GameConfig){
        .screen_width  = SCREEN_W,
        .screen_height = SCREEN_H,
        .title         = "Galaxian",
        .clear_color   = { 9, 11, 22, 255 },
    };
}

void game_init(void)
{
    game_state_register(STATE_MENU, gx_menu_state());
    game_state_register(STATE_GAMEPLAY, gx_gameplay_state());
    game_state_register(STATE_GAME_OVER, gx_gameover_state());
    game_state_switch(STATE_MENU);
}

void game_update(float dt)
{
    game_state_update(dt);
}

void game_draw(void)
{
    game_state_draw();
}

void game_shutdown(void)
{
    game_state_shutdown();
}
