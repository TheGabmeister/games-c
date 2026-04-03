#include "platform.h"
#include <SDL3/SDL.h>

#include "game.h"

void game_init(void)
{
    init_window(600, 800, "sdl-only");
}

void game_loop(void)
{
    SDL_Event event;
    bool running = true;

    if (!is_window_ready())
        return;

    while (running)
    {
        platform_begin_frame();
        while (SDL_PollEvent(&event))
        {
            platform_process_event(&event);
	    }

        float delta = get_deltatime();

    }   
}

void game_shutdown(void)
{
    close_window();    // SDL_Quit called last
}
