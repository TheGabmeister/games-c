#include "game.h"
#include "platform.h"

#include <SDL3/SDL_main.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    init_window(600, 800, "sdl-only");

    if (!is_window_ready()) {
        game_shutdown();
        return 1;
    }

    SDL_Event event;

    if (!is_window_ready())
        return 1;

    while (!window_should_close())
    {
        platform_begin_frame();
        while (SDL_PollEvent(&event))
        {
            platform_process_event(&event);
        }

        (void)get_deltatime();
    }
    
    close_window();    // SDL_Quit called last

    return 0;
}
