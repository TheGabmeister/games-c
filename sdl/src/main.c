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
#include "audio.h"
#include "resources.h"
#include "game.h"

/* Keep the entry point thin: initialize subsystems, run the frame loop,
 * then shut everything down in the correct order. */

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    init_window(600, 800, "template");
    if (!is_window_ready()) {
        return 1;
    }

    audio_init();
    game_init();

    /* --- Main loop --- */
    SDL_Event event;
    while (!window_should_close()) {
        platform_begin_frame();
        while (SDL_PollEvent(&event)) {
            platform_process_event(&event);
        }

        game_update(get_deltatime());

        SDL_SetRenderDrawColor(get_renderer(), 25, 25, 50, 255);
        SDL_RenderClear(get_renderer());

        game_draw();

        SDL_RenderPresent(get_renderer());
    }

    /* --- Shutdown (order matters) --- */
    game_shutdown();
    res_free_all();
    audio_shutdown();
    close_window();
    return 0;
}
