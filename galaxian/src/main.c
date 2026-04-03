/*
 * Galaxian — Entry Point
 */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "platform.h"
#include "game_state.h"
#include "resources.h"
#include "drawing.h"
#include "audio.h"
#include "galaxian.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    init_window(SCREEN_W, SCREEN_H, "Galaxian");
    if (!is_window_ready())
        return 1;

    audio_init();

    /* Enable alpha blending for glow / particle effects */
    SDL_SetRenderDrawBlendMode(get_renderer(), SDL_BLENDMODE_BLEND);

    /* Register game states */
    game_state_register(STATE_MENU, (GameState){
        .init    = menu_init,
        .update  = menu_update,
        .draw    = menu_draw,
        .cleanup = menu_cleanup,
    });
    game_state_register(STATE_GAMEPLAY, (GameState){
        .init    = gameplay_init,
        .update  = gameplay_update,
        .draw    = gameplay_draw,
        .cleanup = gameplay_cleanup,
    });
    game_state_register(STATE_GAME_OVER, (GameState){
        .init    = gameover_init,
        .update  = gameover_update,
        .draw    = gameover_draw,
        .cleanup = gameover_cleanup,
    });

    /* Start at the menu */
    game_state_switch(STATE_MENU);

    /* --- Main loop --- */
    SDL_Event event;
    while (!window_should_close()) {
        engine_begin_frame();
        while (SDL_PollEvent(&event))
            engine_process_event(&event);

        float dt = get_deltatime();

        game_state_update(dt);

        SDL_SetRenderDrawColor(get_renderer(), 9, 11, 22, 255);
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
