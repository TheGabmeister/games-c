#include "drawing.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool measure_text(const char *text, float *w, float *h)
{
    size_t longest_line = 0;
    size_t current_line = 0;
    int line_count = 1;

    if (w) {
        *w = 0.0f;
    }
    if (h) {
        *h = 0.0f;
    }

    if (!text) {
        return false;
    }

    for (const char *p = text; *p; ++p) {
        if (*p == '\n') {
            if (current_line > longest_line) {
                longest_line = current_line;
            }
            current_line = 0;
            line_count++;
        } else {
            current_line++;
        }
    }

    if (current_line > longest_line) {
        longest_line = current_line;
    }

    if (w) {
        *w = (float)(longest_line * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE);
    }
    if (h) {
        *h = (float)(line_count * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE);
    }
    return true;
}

float draw_text(const char *text, float x, float y, SDL_Color color)
{
    SDL_Renderer *renderer = get_renderer();
    float w = 0.0f;

    if (!renderer || !text) {
        return 0.0f;
    }

    measure_text(text, &w, NULL);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDebugText(renderer, x, y, text);
    return w;
}

void draw_rect(float x, float y, float w, float h, SDL_Color color)
{
    SDL_SetRenderDrawColor(get_renderer(), color.r, color.g, color.b, color.a);
    SDL_FRect rect = {x, y, w, h};
    SDL_RenderFillRect(get_renderer(), &rect);
}

void draw_rect_outline(float x, float y, float w, float h, SDL_Color color)
{
    SDL_SetRenderDrawColor(get_renderer(), color.r, color.g, color.b, color.a);
    SDL_FRect rect = {x, y, w, h};
    SDL_RenderRect(get_renderer(), &rect);
}

void draw_line(float x1, float y1, float x2, float y2, SDL_Color color)
{
    SDL_SetRenderDrawColor(get_renderer(), color.r, color.g, color.b, color.a);
    SDL_RenderLine(get_renderer(), x1, y1, x2, y2);
}

void draw_circle(float cx, float cy, float radius, SDL_Color color)
{
    SDL_Renderer *renderer = get_renderer();
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    /* Scanline fill using the midpoint circle boundary */
    int r = (int)radius;
    int x = r;
    int y = 0;
    int err = 1 - r;

    while (x >= y) {
        SDL_RenderLine(renderer, cx - x, cy + y, cx + x, cy + y);
        SDL_RenderLine(renderer, cx - x, cy - y, cx + x, cy - y);
        SDL_RenderLine(renderer, cx - y, cy + x, cx + y, cy + x);
        SDL_RenderLine(renderer, cx - y, cy - x, cx + y, cy - x);

        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void draw_circle_outline(float cx, float cy, float radius, SDL_Color color)
{
    SDL_Renderer *renderer = get_renderer();
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    int segs = (int)(radius * 4);
    if (segs < 16) segs = 16;
    if (segs > 64) segs = 64;

    float step = 2.0f * (float)M_PI / segs;
    float px = cx + radius;
    float py = cy;

    for (int i = 1; i <= segs; i++) {
        float a  = i * step;
        float nx = cx + cosf(a) * radius;
        float ny = cy + sinf(a) * radius;
        SDL_RenderLine(renderer, px, py, nx, ny);
        px = nx;
        py = ny;
    }
}

void draw_triangle(float x1, float y1, float x2, float y2,
                   float x3, float y3, SDL_Color color)
{
    SDL_FColor fc = {
        color.r / 255.0f, color.g / 255.0f,
        color.b / 255.0f, color.a / 255.0f
    };
    SDL_Vertex verts[3] = {
        { {x1, y1}, fc, {0, 0} },
        { {x2, y2}, fc, {0, 0} },
        { {x3, y3}, fc, {0, 0} },
    };
    SDL_RenderGeometry(get_renderer(), NULL, verts, 3, NULL, 0);
}
