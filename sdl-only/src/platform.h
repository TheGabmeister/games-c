#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <SDL3/SDL.h>

typedef SDL_FRect rectangle;
typedef SDL_Color color;
typedef struct { float x, y; } vector2;
typedef struct { float x, y, z; } vector3;
typedef struct { float x, y, z, w; } vector4;

// 2D camera
typedef struct camera {
    vector2 offset;
    vector2 target;
    float rotation;
    float zoom;
} camera;

void init_window(int width, int height, const char *title);
void close_window(void);
bool is_window_ready(void);
bool window_should_close(void);
int get_fps(void);
float get_deltatime(void);
SDL_Renderer *get_renderer(void);
bool is_key_down(int key);
bool is_key_pressed(int key);
void platform_begin_frame(void);
void platform_process_event(SDL_Event *event);
bool is_mouse_button_down(int button);
bool is_mouse_button_pressed(int button);
vector2 get_mouse_position(void);
float get_mouse_wheel_move(void);

#endif // PLATFORM_H
