#ifndef GAME_H
#define GAME_H

#include "scene.h"
#include <stdbool.h>

/* Initialize SDL, create window and renderer. Returns false on failure. */
bool game_init(void);

/* Run the main loop with the given initial scene. Blocks until quit. */
void game_run(const Scene *initial_scene);

/* Clean up SDL resources. */
void game_shutdown(void);

/* Request a scene transition (takes effect next frame). */
void game_set_scene(const Scene *scene);

/* Request the game to quit. */
void game_quit(void);

#endif
