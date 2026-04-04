#include "scene.h"
#include <stdbool.h>

static Scene current_scene = { 0 };
static Scene pending_scene = { 0 };
static bool has_pending = false;

void scene_set(Scene scene)
{
    pending_scene = scene;
    has_pending = true;
}

void scene_apply_pending(void)
{
    if (!has_pending) return;
    has_pending = false;

    if (current_scene.cleanup) current_scene.cleanup();
    current_scene = pending_scene;
    pending_scene = (Scene){ 0 };
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
