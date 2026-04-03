#include "draw.h"
#include "config.h"
#include <stdarg.h>
#include <stdio.h>

static SDL_Renderer *g_renderer;
static Camera g_camera = {{0, 0}, 1.0f};

void draw_init(SDL_Renderer *renderer)
{
    g_renderer = renderer;
}

void draw_set_camera(Camera cam)
{
    if (cam.zoom < 0.01f) cam.zoom = 0.01f;
    g_camera = cam;
}

Camera draw_get_camera(void)
{
    return g_camera;
}

Vec2 draw_world_to_screen(Vec2 world)
{
    Vec2 rel = vec2_sub(world, g_camera.pos);
    return vec2(
        rel.x * g_camera.zoom + WINDOW_WIDTH * 0.5f,
        rel.y * g_camera.zoom + WINDOW_HEIGHT * 0.5f
    );
}

Vec2 draw_screen_to_world(Vec2 screen)
{
    return vec2(
        (screen.x - WINDOW_WIDTH * 0.5f) / g_camera.zoom + g_camera.pos.x,
        (screen.y - WINDOW_HEIGHT * 0.5f) / g_camera.zoom + g_camera.pos.y
    );
}

void draw_clear(Color c)
{
    SDL_SetRenderDrawColor(g_renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(g_renderer);
}

/* ---- internal helpers ---- */

static void set_color(Color c)
{
    SDL_SetRenderDrawColor(g_renderer, c.r, c.g, c.b, c.a);
}

/* ---- world-space primitives ---- */

void draw_rect(Rect r, Color color)
{
    Vec2 tl = draw_world_to_screen(vec2(r.x, r.y));
    float w = r.w * g_camera.zoom;
    float h = r.h * g_camera.zoom;
    SDL_FRect fr = {tl.x, tl.y, w, h};
    set_color(color);
    SDL_RenderRect(g_renderer, &fr);
}

void draw_rect_fill(Rect r, Color color)
{
    Vec2 tl = draw_world_to_screen(vec2(r.x, r.y));
    float w = r.w * g_camera.zoom;
    float h = r.h * g_camera.zoom;
    SDL_FRect fr = {tl.x, tl.y, w, h};
    set_color(color);
    SDL_RenderFillRect(g_renderer, &fr);
}

void draw_line(Vec2 a, Vec2 b, Color color)
{
    Vec2 sa = draw_world_to_screen(a);
    Vec2 sb = draw_world_to_screen(b);
    set_color(color);
    SDL_RenderLine(g_renderer, sa.x, sa.y, sb.x, sb.y);
}

void draw_circle(Vec2 center, float radius, Color color)
{
    Vec2 sc = draw_world_to_screen(center);
    float sr = radius * g_camera.zoom;
    set_color(color);

    /* Midpoint circle algorithm */
    int r = (int)sr;
    int x = r, y = 0, err = 1 - r;
    float cx = sc.x, cy = sc.y;

    while (x >= y) {
        SDL_RenderPoint(g_renderer, cx + x, cy + y);
        SDL_RenderPoint(g_renderer, cx - x, cy + y);
        SDL_RenderPoint(g_renderer, cx + x, cy - y);
        SDL_RenderPoint(g_renderer, cx - x, cy - y);
        SDL_RenderPoint(g_renderer, cx + y, cy + x);
        SDL_RenderPoint(g_renderer, cx - y, cy + x);
        SDL_RenderPoint(g_renderer, cx + y, cy - x);
        SDL_RenderPoint(g_renderer, cx - y, cy - x);
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void draw_circle_fill(Vec2 center, float radius, Color color)
{
    Vec2 sc = draw_world_to_screen(center);
    float sr = radius * g_camera.zoom;
    set_color(color);

    int r = (int)sr;
    int x = r, y = 0, err = 1 - r;
    float cx = sc.x, cy = sc.y;

    while (x >= y) {
        SDL_RenderLine(g_renderer, cx - x, cy + y, cx + x, cy + y);
        SDL_RenderLine(g_renderer, cx - x, cy - y, cx + x, cy - y);
        SDL_RenderLine(g_renderer, cx - y, cy + x, cx + y, cy + x);
        SDL_RenderLine(g_renderer, cx - y, cy - x, cx + y, cy - x);
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

/* ---- screen-space primitives ---- */

void draw_rect_screen(Rect r, Color color)
{
    SDL_FRect fr = {r.x, r.y, r.w, r.h};
    set_color(color);
    SDL_RenderRect(g_renderer, &fr);
}

void draw_rect_fill_screen(Rect r, Color color)
{
    SDL_FRect fr = {r.x, r.y, r.w, r.h};
    set_color(color);
    SDL_RenderFillRect(g_renderer, &fr);
}

void draw_line_screen(Vec2 a, Vec2 b, Color color)
{
    set_color(color);
    SDL_RenderLine(g_renderer, a.x, a.y, b.x, b.y);
}

/* ---- text (screen-space) ---- */

void draw_text(Vec2 pos, float scale, Color color, const char *fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    set_color(color);
    if (scale != 1.0f) {
        SDL_SetRenderScale(g_renderer, scale, scale);
        SDL_RenderDebugText(g_renderer, pos.x / scale, pos.y / scale, buf);
        SDL_SetRenderScale(g_renderer, 1.0f, 1.0f);
    } else {
        SDL_RenderDebugText(g_renderer, pos.x, pos.y, buf);
    }
}

void draw_present(void)
{
    SDL_RenderPresent(g_renderer);
}
