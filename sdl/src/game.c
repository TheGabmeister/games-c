/*
 * Game Bootstrap
 * ==============
 * This file owns game-specific setup and the active state graph. Keep
 * platform/window code in platform.c and use this layer for gameplay wiring.
 */

#include "game.h"
#include <SDL3/SDL.h>
#include "game_state.h"
#include "drawing.h"

/* -----------------------------------------------------------------------
 * Example state: a placeholder that draws a rectangle.
 * In a real project, move each state to its own file (state_menu.c, etc.)
 * ----------------------------------------------------------------------- */

static void example_init(void)
{
    /* Load assets here:
     *   SDL_Texture *player = res_load_texture("assets/player.png");
     *   TTF_Font    *font   = res_load_font("assets/ui.ttf", 24);
     *   MIX_Audio   *sfx    = res_load_sound("assets/jump.wav");
     */
}

static void example_update(float dt)
{
    /* Game logic and input handling:
     *   if (is_key_pressed(KEY_ESCAPE))
     *       game_state_switch(STATE_PAUSE);
     *
     *   player.x += speed * dt;
     */
    (void)dt;
}

static void example_draw(void)
{
    /* Rendering:
     *   draw_texture(player_tex, player.x, player.y);
     *   draw_text(font, "Hello", 10, 10, (SDL_Color){255,255,255,255});
     *   draw_circle(400, 300, 50, (SDL_Color){255,0,0,255});
     */
    draw_rect(200.0f, 175.0f, 200.0f, 150.0f, (SDL_Color){255, 255, 255, 255});
}

static void example_cleanup(void)
{
    /* Free state-specific resources if needed.
     * Assets loaded via res_load_* are freed by res_free_all() at shutdown,
     * so you only need to clean up non-resource state here. */
}

void game_init(void)
{
    game_state_register(STATE_GAMEPLAY, (GameState){
        .init = example_init,
        .update = example_update,
        .draw = example_draw,
        .cleanup = example_cleanup,
    });

    game_state_switch(STATE_GAMEPLAY);
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
