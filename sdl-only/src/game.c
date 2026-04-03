#include "game.h"
#include "scene.h"
#include "scenes/title_scene.h"

void game_init(void)
{
    scene_set(title_scene());
}

void game_shutdown(void)
{
    scene_cleanup();
}
