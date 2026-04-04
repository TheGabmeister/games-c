#include "config.h"
#include "platform.h"
#include "input.h"
#include "scene.h"
#include "scenes/scene_title.h"

#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    srand((unsigned)time(NULL));

    init_window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    if (!is_window_ready())
        return 1;

    input_init();
    scene_set(scene_title());
    scene_apply_pending();

    SDL_Event event;

    while (!window_should_close())
    {
        input_begin_frame();
        while (SDL_PollEvent(&event))
        {
            platform_process_event(&event);
            input_process_event(&event);
        }

        platform_update_timing();
        float dt = get_deltatime();

        scene_update(dt);
        scene_draw();
        scene_apply_pending();
    }

    scene_cleanup();
    close_window();

    return 0;
}
