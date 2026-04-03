#include "drawing.h"
#include <math.h>

void draw_texture(SDL_Texture *texture, float x, float y)
{
    if (!texture) return;

    float w, h;
    SDL_GetTextureSize(texture, &w, &h);

    SDL_FRect dst = {x, y, w, h};
    SDL_RenderTexture(get_renderer(), texture, NULL, &dst);
}

void draw_texture_region(SDL_Texture *texture, const SDL_FRect *src,
                         float x, float y, float w, float h)
{
    if (!texture) return;

    SDL_FRect dst = {x, y, w, h};
    SDL_RenderTexture(get_renderer(), texture, src, &dst);
}

void draw_texture_ex(SDL_Texture *texture, float x, float y,
                     float rotation_deg, float scale)
{
    if (!texture) return;

    float w, h;
    SDL_GetTextureSize(texture, &w, &h);

    SDL_FRect dst = {x, y, w * scale, h * scale};
    SDL_FPoint center = {(w * scale) / 2.0f, (h * scale) / 2.0f};
    SDL_RenderTextureRotated(get_renderer(), texture, NULL, &dst,
                             (double)rotation_deg, &center, SDL_FLIP_NONE);
}

float draw_text(TTF_Font *font, const char *text, float x, float y, SDL_Color color)
{
    if (!font || !text) return 0.0f;

    SDL_Surface *surface = TTF_RenderText_Blended(font, text, 0, color);
    if (!surface) return 0.0f;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(get_renderer(), surface);
    float w = (float)surface->w;
    float h = (float)surface->h;
    SDL_DestroySurface(surface);

    if (!texture) return 0.0f;

    SDL_FRect dst = {x, y, w, h};
    SDL_RenderTexture(get_renderer(), texture, NULL, &dst);
    SDL_DestroyTexture(texture);

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
