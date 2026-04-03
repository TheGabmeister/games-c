/*
 * SDL3 Game Template — Entry Point
 * =================================
 * This file wires together the template systems. The architecture is:
 *
 *   platform.h   — Window, input, timing (don't modify unless extending the engine)
 *   game_state.h — State/scene management (menu, gameplay, pause, etc.)
 *   resources.h  — Asset loading and caching (textures, fonts, sounds, music)
 *   drawing.h    — Rendering helpers (sprites, text, shapes)
 *   audio.h      — Sound effect and music playback
 *
 * To start building a game:
 *   1. Create your state files (e.g., state_gameplay.c/h)
 *   2. Implement init/update/draw/cleanup callbacks
 *   3. Register them below and set the starting state
 *   4. Load assets via res_load_* and draw with draw_* helpers
 */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "platform.h"
#include "game_state.h"
#include "resources.h"
#include "drawing.h"
#include "audio.h"

/* -----------------------------------------------------------------------
 * Example state: a placeholder that draws a rectangle.
 * In a real project, move each state to its own file (state_menu.c, etc.)
 * ----------------------------------------------------------------------- */

static void example_init(void)
{
    /* Load assets here:
     *   SDL_Texture *player = res_load_texture("assets/player.png");
     *   TTF_Font    *font   = res_load_font("assets/ui.ttf", 24);
     *   Mix_Chunk   *sfx    = res_load_sound("assets/jump.wav");
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

/* -----------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    init_window(600, 800, "template");
    if (!is_window_ready())
        return 1;

    audio_init();

    /* Register your game states here */
    game_state_register(STATE_GAMEPLAY, (GameState){
        .init    = example_init,
        .update  = example_update,
        .draw    = example_draw,
        .cleanup = example_cleanup,
    });

    /* Set the starting state */
    game_state_switch(STATE_GAMEPLAY);

    /* --- Main loop --- */
    SDL_Event event;
    while (!window_should_close()) {
        engine_begin_frame();
        while (SDL_PollEvent(&event))
            engine_process_event(&event);

        float dt = get_deltatime();

        game_state_update(dt);

        SDL_SetRenderDrawColor(get_renderer(), 25, 25, 50, 255);
        SDL_RenderClear(get_renderer());

        game_state_draw();

        SDL_RenderPresent(get_renderer());
    }

    /* --- Shutdown (order matters) --- */
    game_state_shutdown();
    res_free_all();
    audio_shutdown();
    close_window();
    return 0;
}
