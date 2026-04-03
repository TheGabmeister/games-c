#ifndef DRAWING_H
#define DRAWING_H

#include <stdbool.h>
#include <SDL3/SDL.h>
#include "platform.h"

/*
 * Drawing Helpers
 * ===============
 * Thin wrappers for common rendering operations. All functions draw to the
 * renderer returned by get_renderer() — no renderer parameter needed.
 *
 * --- Text ---
 *
 * SDL3 ships with a tiny built-in debug font, which is enough for the
 * game HUD and menus.
 *
 *     draw_text("Score: 100", 10.0f, 10.0f, (SDL_Color){255,255,255,255});
 *
 * --- Shapes ---
 *
 *     draw_rect(x, y, w, h, color);            // filled rectangle
 *     draw_rect_outline(x, y, w, h, color);    // unfilled rectangle
 *     draw_line(x1, y1, x2, y2, color);        // single line
 *     draw_circle(cx, cy, radius, color);       // filled circle (approximated)
 */

/* Measure a text string without rendering it. */
bool measure_text(const char *text, float *w, float *h);

/* Draw a text string at (x, y). Returns the width of the rendered text. */
float draw_text(const char *text, float x, float y, SDL_Color color);

/* Filled rectangle */
void draw_rect(float x, float y, float w, float h, SDL_Color color);

/* Unfilled rectangle outline */
void draw_rect_outline(float x, float y, float w, float h, SDL_Color color);

/* Single line between two points */
void draw_line(float x1, float y1, float x2, float y2, SDL_Color color);

/* Filled circle (midpoint algorithm) */
void draw_circle(float cx, float cy, float radius, SDL_Color color);

/* Circle outline (ring) */
void draw_circle_outline(float cx, float cy, float radius, SDL_Color color);

/* Filled triangle */
void draw_triangle(float x1, float y1, float x2, float y2,
                   float x3, float y3, SDL_Color color);

#endif /* DRAWING_H */
