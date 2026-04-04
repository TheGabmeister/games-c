#include "draw.h"

static void set_color(color c)
{
    SDL_SetRenderDrawColor(get_renderer(), c.r, c.g, c.b, c.a);
}

void begin_drawing(color clear_color)
{
    set_color(clear_color);
    SDL_RenderClear(get_renderer());
}

void end_drawing(void)
{
    SDL_RenderPresent(get_renderer());
}

void draw_rect(rectangle rect, color c)
{
    set_color(c);
    SDL_RenderRect(get_renderer(), &rect);
}

void draw_rect_filled(rectangle rect, color c)
{
    set_color(c);
    SDL_RenderFillRect(get_renderer(), &rect);
}

void draw_line(vector2 start, vector2 end, color c)
{
    set_color(c);
    SDL_RenderLine(get_renderer(), start.x, start.y, end.x, end.y);
}

void draw_circle(vector2 center, float radius, color c)
{
    set_color(c);

    int x = (int)radius;
    int y = 0;
    int err = 1 - x;
    float cx = center.x;
    float cy = center.y;
    SDL_Renderer *r = get_renderer();

    while (x >= y) {
        SDL_RenderPoint(r, cx + x, cy + y);
        SDL_RenderPoint(r, cx + y, cy + x);
        SDL_RenderPoint(r, cx - y, cy + x);
        SDL_RenderPoint(r, cx - x, cy + y);
        SDL_RenderPoint(r, cx - x, cy - y);
        SDL_RenderPoint(r, cx - y, cy - x);
        SDL_RenderPoint(r, cx + y, cy - x);
        SDL_RenderPoint(r, cx + x, cy - y);
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void draw_circle_filled(vector2 center, float radius, color c)
{
    set_color(c);

    int x = (int)radius;
    int y = 0;
    int err = 1 - x;
    float cx = center.x;
    float cy = center.y;
    SDL_Renderer *r = get_renderer();

    while (x >= y) {
        SDL_RenderLine(r, cx - x, cy + y, cx + x, cy + y);
        SDL_RenderLine(r, cx - y, cy + x, cx + y, cy + x);
        SDL_RenderLine(r, cx - x, cy - y, cx + x, cy - y);
        SDL_RenderLine(r, cx - y, cy - x, cx + y, cy - x);
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void draw_text(const char *text, float x, float y, float scale, color c)
{
    set_color(c);
    SDL_Renderer *r = get_renderer();

    if (scale != 1.0f) {
        SDL_SetRenderScale(r, scale, scale);
        SDL_RenderDebugText(r, x / scale, y / scale, text);
        SDL_SetRenderScale(r, 1.0f, 1.0f);
    } else {
        SDL_RenderDebugText(r, x, y, text);
    }
}
