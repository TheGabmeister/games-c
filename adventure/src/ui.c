#include "ui.h"
#include "game.h"
#include "menu.h"
#include "platform.h"
#include "defines.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>

/*=============================================================================
 * Font state
 *===========================================================================*/
static TTF_Font *_font_hud = NULL;
static TTF_Font *_font_large = NULL;

void ui_init(void)
{
    ui_fini();

    _font_hud = TTF_OpenFont("assets/fonts/press_start_2p/PressStart2P-Regular.ttf", 14);
    _font_large = TTF_OpenFont("assets/fonts/press_start_2p/PressStart2P-Regular.ttf", 24);

    if (!_font_hud)
        SDL_Log("ui: failed to load HUD font: %s", SDL_GetError());
    if (!_font_large)
        SDL_Log("ui: failed to load large font: %s", SDL_GetError());
}

void ui_fini(void)
{
    if (_font_hud)
    {
        TTF_CloseFont(_font_hud);
        _font_hud = NULL;
    }

    if (_font_large)
    {
        TTF_CloseFont(_font_large);
        _font_large = NULL;
    }
}

/*=============================================================================
 * Text helper
 *===========================================================================*/
static void _draw_text(SDL_Renderer *r, TTF_Font *font, const char *text,
                       float cx, float cy, SDL_Color color)
{
    if (!font)
    {
        SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
        float scale = 2.0f;
        SDL_SetRenderScale(r, scale, scale);
        SDL_RenderDebugText(r, cx / scale - (float)SDL_strlen(text) * 4.0f, cy / scale - 4.0f, text);
        SDL_SetRenderScale(r, 1.0f, 1.0f);
        return;
    }

    SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex)
    {
        float w = (float)surf->w;
        float h = (float)surf->h;
        SDL_FRect dst = { cx - w * 0.5f, cy - h * 0.5f, w, h };
        SDL_RenderTexture(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_DestroySurface(surf);
}

static void _draw_text_left(SDL_Renderer *r, TTF_Font *font, const char *text,
                            float x, float y, SDL_Color color)
{
    if (!font)
    {
        SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
        SDL_RenderDebugText(r, x, y, text);
        return;
    }

    SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex)
    {
        SDL_FRect dst = { x, y, (float)surf->w, (float)surf->h };
        SDL_RenderTexture(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_DestroySurface(surf);
}

/*=============================================================================
 * Render
 *===========================================================================*/
static void _render_hud(SDL_Renderer *r, const GameState *gs)
{
    /* Timer in top-right */
    int total_seconds = (int)gs->elapsed_time;
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;

    char timer_text[16];
    SDL_snprintf(timer_text, sizeof(timer_text), "%02d:%02d", minutes, seconds);

    SDL_Color white = {255, 255, 255, 200};
    _draw_text_left(r, _font_hud, timer_text,
                    (float)WINDOW_WIDTH - 120.0f, 10.0f, white);
}

static void _render_pause(SDL_Renderer *r)
{
    /* Dim overlay */
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
    SDL_FRect full = { 0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT };
    SDL_RenderFillRect(r, &full);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color dim   = {150, 150, 150, 255};
    float cx = WINDOW_WIDTH * 0.5f;

    _draw_text(r, _font_large, "PAUSED", cx, 300.0f, white);
    _draw_text(r, _font_hud, "RESUME: ESC / P", cx, 380.0f, dim);
    _draw_text(r, _font_hud, "QUIT: Q", cx, 420.0f, dim);
}

static void _render_victory(SDL_Renderer *r, const GameState *gs)
{
    /* Dim overlay */
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
    SDL_FRect full = { 0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT };
    SDL_RenderFillRect(r, &full);

    SDL_Color gold = {255, 215, 0, 255};
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color dim   = {150, 150, 150, 255};
    float cx = WINDOW_WIDTH * 0.5f;

    _draw_text(r, _font_large, "VICTORY!", cx, 240.0f, gold);

    /* Time */
    int total_seconds = (int)gs->elapsed_time;
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;
    int tenths  = (int)((gs->elapsed_time - (float)total_seconds) * 10.0f);

    char time_text[32];
    SDL_snprintf(time_text, sizeof(time_text), "TIME: %02d:%02d.%d", minutes, seconds, tenths);
    _draw_text(r, _font_hud, time_text, cx, 320.0f, white);

    /* Mode */
    const char *mode_name = "UNKNOWN";
    if (gs->mode == 1) mode_name = "BEGINNER";
    else if (gs->mode == 2) mode_name = "STANDARD";
    else if (gs->mode == 3) mode_name = "RANDOMIZED";

    char mode_text[32];
    SDL_snprintf(mode_text, sizeof(mode_text), "MODE: %s", mode_name);
    _draw_text(r, _font_hud, mode_text, cx, 370.0f, white);

    _draw_text(r, _font_hud, "PRESS ENTER TO CONTINUE", cx, 460.0f, dim);
}

static void _render_death_freeze(SDL_Renderer *r, const GameState *gs)
{
    /* Optional red flash */
    float t = gs->death_timer / 1.5f;
    if (t > 0.8f)
    {
        Uint8 alpha = (Uint8)((t - 0.8f) * 5.0f * 80.0f);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 200, 0, 0, alpha);
        SDL_FRect full = { 0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT };
        SDL_RenderFillRect(r, &full);
    }
}

void ui_render(void)
{
    SDL_Renderer *r = get_renderer();
    const GameState *gs = game_state_get();

    switch (gs->scene)
    {
        case SCENE_MENU:
            menu_render();
            break;

        case SCENE_PLAYING:
            _render_hud(r, gs);
            break;

        case SCENE_PAUSED:
            _render_hud(r, gs);
            _render_pause(r);
            break;

        case SCENE_DEATH_FREEZE:
            _render_hud(r, gs);
            _render_death_freeze(r, gs);
            break;

        case SCENE_VICTORY:
            _render_victory(r, gs);
            break;
    }

    /* Debug entity list */
    if (gs->debug_show_entity_list && gs->scene != SCENE_MENU)
    {
        SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
        char buf[64];
        SDL_snprintf(buf, sizeof(buf), "Room: %d  Mode: %d", gs->current_room, gs->mode);
        SDL_RenderDebugText(r, 10, 10, buf);
    }
}
