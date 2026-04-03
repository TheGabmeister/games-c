#ifndef DRAWING_H
#define DRAWING_H

#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "platform.h"

/*
 * Drawing Helpers
 * ===============
 * Thin wrappers for common rendering operations. All functions draw to the
 * renderer returned by get_renderer() — no renderer parameter needed.
 *
 * --- Textures / Sprites ---
 *
 *     SDL_Texture *tex = res_load_texture("assets/player.png");
 *
 *     // Draw at position (simple)
 *     draw_texture(tex, 100.0f, 200.0f);
 *
 *     // Draw a sub-region of the texture (sprite sheets)
 *     SDL_FRect src = {0, 0, 32, 32};  // first 32x32 tile
 *     draw_texture_region(tex, &src, 100.0f, 200.0f, 64.0f, 64.0f);
 *
 *     // Draw with rotation and scale
 *     draw_texture_ex(tex, 100.0f, 200.0f, 45.0f, 2.0f);
 *
 * --- Text ---
 *
 *     TTF_Font *font = res_load_font("assets/ui.ttf", 24);
 *
 *     // White text at position
 *     draw_text(font, "Score: 100", 10.0f, 10.0f, (SDL_Color){255,255,255,255});
 *
 * Text rendering creates a surface+texture each call. For text that changes
 * every frame (score, timer) this is fine. For large static text blocks,
 * consider caching the texture yourself.
 *
 * --- Shapes ---
 *
 *     draw_rect(x, y, w, h, color);            // filled rectangle
 *     draw_rect_outline(x, y, w, h, color);    // unfilled rectangle
 *     draw_line(x1, y1, x2, y2, color);        // single line
 *     draw_circle(cx, cy, radius, color);       // filled circle (approximated)
 */

/* Draw a texture at (x, y) using its native size */
void draw_texture(SDL_Texture *texture, float x, float y);

/* Draw a sub-region of a texture, scaled to (w, h) at (x, y) */
void draw_texture_region(SDL_Texture *texture, const SDL_FRect *src,
                         float x, float y, float w, float h);

/* Draw a texture at (x, y) with rotation (degrees) and uniform scale */
void draw_texture_ex(SDL_Texture *texture, float x, float y,
                     float rotation_deg, float scale);

/* Measure a text string without rendering it. */
bool measure_text(TTF_Font *font, const char *text, float *w, float *h);

/* Draw a text string at (x, y). Returns the width of the rendered text. */
float draw_text(TTF_Font *font, const char *text, float x, float y, SDL_Color color);

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
