#include "game.h"
#include "demo_scene.h"
#include <SDL3/SDL_main.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    if (!game_init()) return 1;
    game_run(&demo_scene);
    game_shutdown();

    return 0;
}
