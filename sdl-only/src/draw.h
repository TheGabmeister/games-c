#ifndef DRAW_H
#define DRAW_H

#include "types.h"
#include <SDL3/SDL.h>

/* Camera (world-space -> screen-space transform) */
typedef struct Camera {
    Vec2 pos;       /* world position the camera is centered on */
    float zoom;     /* pixels per world unit (default: 1.0) */
} Camera;

/* Must be called once with the renderer before any draw calls */
void draw_init(SDL_Renderer *renderer);

/* Set the active camera for world-space drawing */
void draw_set_camera(Camera cam);

/* Get the current camera */
Camera draw_get_camera(void);

/* Convert between coordinate spaces */
Vec2 draw_world_to_screen(Vec2 world);
Vec2 draw_screen_to_world(Vec2 screen);

/* Clear the screen */
void draw_clear(Color color);

/* World-space primitives (affected by camera) */
void draw_rect(Rect r, Color color);
void draw_rect_fill(Rect r, Color color);
void draw_line(Vec2 a, Vec2 b, Color color);
void draw_circle(Vec2 center, float radius, Color color);
void draw_circle_fill(Vec2 center, float radius, Color color);

/* Screen-space primitives (not affected by camera, for HUD/UI) */
void draw_rect_screen(Rect r, Color color);
void draw_rect_fill_screen(Rect r, Color color);
void draw_line_screen(Vec2 a, Vec2 b, Color color);

/* Text (screen-space, using SDL debug font, 8x8 base size) */
void draw_text(Vec2 pos, float scale, Color color, const char *fmt, ...);

/* Present the frame */
void draw_present(void);

#endif
