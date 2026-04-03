#ifndef WINDOW_H
#define WINDOW_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/* Initialize SDL, create window and renderer. Returns false on failure. */
bool window_init(void);

/* Clean up window, renderer, and SDL. */
void window_shutdown(void);

/* Access the renderer (owned by the window module). */
SDL_Renderer *window_renderer(void);

/* Pump and process SDL events. Returns false when quit is requested. */
bool window_poll_events(void);

#endif
