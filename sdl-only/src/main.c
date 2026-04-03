#include "config.h"
#include "platform.h"
#include "input.h"
#include "game.h"
#include "scene.h"

#include <SDL3/SDL_main.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    init_window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    if (!is_window_ready())
        return 1;

    input_init();
    game_init();

    SDL_Event event;

    while (!window_should_close())
    {
        input_begin_frame();
        while (SDL_PollEvent(&event))
        {
            platform_process_event(&event);
            input_process_event(&event);
        }

        float dt = get_deltatime();
        scene_update(dt);
        scene_draw();
    }

    game_shutdown();
    close_window();

    return 0;
}
