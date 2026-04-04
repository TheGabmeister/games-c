#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <SDL3/SDL.h>

typedef SDL_FRect rectangle;
typedef SDL_Color color;
typedef struct { float x, y; } vector2;

// Window
void init_window(int width, int height, const char *title);
void close_window(void);
bool is_window_ready(void);
bool window_should_close(void);
void request_close(void);
int get_window_width(void);
int get_window_height(void);

// Timing
void platform_update_timing(void);
int get_fps(void);
float get_deltatime(void);

// Renderer
SDL_Renderer *get_renderer(void);

// Event handling
void platform_process_event(SDL_Event *event);

#endif // PLATFORM_H
