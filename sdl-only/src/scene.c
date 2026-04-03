#include "scene.h"
#include <stddef.h>

static Scene current_scene = { 0 };

void scene_set(Scene scene)
{
    if (current_scene.cleanup) current_scene.cleanup();
    current_scene = scene;
    if (current_scene.init) current_scene.init();
}

Scene scene_get(void)
{
    return current_scene;
}

void scene_update(float dt)
{
    if (current_scene.update) current_scene.update(dt);
}

void scene_draw(void)
{
    if (current_scene.draw) current_scene.draw();
}

void scene_cleanup(void)
{
    if (current_scene.cleanup) current_scene.cleanup();
    current_scene = (Scene){ 0 };
}
