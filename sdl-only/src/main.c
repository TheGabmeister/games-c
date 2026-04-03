#include "window.h"
#include "draw.h"
#include "game.h"
#include "demo_scene.h"
#include <SDL3/SDL_main.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    if (!window_init()) return 1;
    draw_init(window_renderer());
    game_run(&demo_scene);
    window_shutdown();

    return 0;
}
