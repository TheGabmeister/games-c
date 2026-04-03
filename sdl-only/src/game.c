#include "game.h"
#include "scene.h"
#include "scenes/scene_title.h"

void game_init(void)
{
    scene_set(scene_title());
}

void game_shutdown(void)
{
    scene_cleanup();
}
