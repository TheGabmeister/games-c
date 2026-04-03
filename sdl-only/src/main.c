#include "platform.h"
#include "game.h"
#include "scene.h"

#include <SDL3/SDL_main.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    init_window(800, 600, "2D Game Template");

    if (!is_window_ready())
        return 1;

    game_init();

    SDL_Event event;

    while (!window_should_close())
    {
        platform_begin_frame();
        while (SDL_PollEvent(&event))
        {
            platform_process_event(&event);
        }

        float dt = get_deltatime();
        scene_update(dt);
        scene_draw();
    }

    game_shutdown();
    close_window();

    return 0;
}
