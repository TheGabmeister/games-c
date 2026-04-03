#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>

/*
 * Scene interface. Each game screen (menu, gameplay, pause, etc.)
 * implements this by filling in a static Scene struct.
 *
 * Usage:
 *   static bool my_init(void) { ... return true; }
 *   static void my_shutdown(void) { ... }
 *   static void my_update(float dt) { ... }
 *   static void my_render(void) { ... }
 *
 *   const Scene scene_my = {
 *       .init     = my_init,
 *       .shutdown = my_shutdown,
 *       .update   = my_update,
 *       .render   = my_render,
 *   };
 */
typedef struct Scene {
    bool (*init)(void);         /* called once when scene becomes active */
    void (*shutdown)(void);     /* called once when scene is being replaced */
    void (*update)(float dt);   /* called at fixed timestep */
    void (*render)(void);       /* called every frame */
} Scene;

#endif
