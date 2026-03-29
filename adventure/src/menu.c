#include "menu.h"
#include "game.h"
#include "platform.h"
#include "defines.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

static TTF_Font *_font_large = NULL;
static TTF_Font *_font_small = NULL;
static int _selected = 0;

void menu_init(void)
{
    menu_fini();

    _font_large = TTF_OpenFont("assets/fonts/press_start_2p/PressStart2P-Regular.ttf", 32);
    if (!_font_large)
        SDL_Log("menu: failed to load large font: %s", SDL_GetError());

    _font_small = TTF_OpenFont("assets/fonts/press_start_2p/PressStart2P-Regular.ttf", 16);
    if (!_font_small)
        SDL_Log("menu: failed to load small font: %s", SDL_GetError());

    _selected = 0;
}

void menu_fini(void)
{
    if (_font_large)
    {
        TTF_CloseFont(_font_large);
        _font_large = NULL;
    }

    if (_font_small)
    {
        TTF_CloseFont(_font_small);
        _font_small = NULL;
    }
}

void menu_handle_event(const SDL_Event *event)
{
    if (event->type != SDL_EVENT_KEY_DOWN)
        return;

    GameState *gs = game_state_get();
    if (gs->scene != SCENE_MENU)
        return;

    switch (event->key.key)
    {
        case SDLK_UP:
        case SDLK_W:
            _selected = (_selected + 2) % 3;
            break;
        case SDLK_DOWN:
        case SDLK_S:
            _selected = (_selected + 1) % 3;
            break;
        case SDLK_RETURN:
        case SDLK_SPACE:
            game_start_mode(_selected + 1);
            break;
        case SDLK_1:
            game_start_mode(1);
            break;
        case SDLK_2:
            game_start_mode(2);
            break;
        case SDLK_3:
            game_start_mode(3);
            break;
        default:
            break;
    }
}

static void _draw_text(SDL_Renderer *r, TTF_Font *font, const char *text,
                       float cx, float cy, SDL_Color color)
{
    if (!font) {
        /* Fallback to debug text */
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
    if (tex) {
        float w = (float)surf->w;
        float h = (float)surf->h;
        SDL_FRect dst = { cx - w * 0.5f, cy - h * 0.5f, w, h };
        SDL_RenderTexture(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_DestroySurface(surf);
}

void menu_render(void)
{
    SDL_Renderer *r = get_renderer();
    float cx = WINDOW_WIDTH * 0.5f;

    SDL_Color gold   = {255, 215, 0, 255};
    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color dim    = {150, 150, 150, 255};

    /* Title */
    _draw_text(r, _font_large, "ADVENTURE", cx, 140.0f, gold);

    /* Mode options */
    const char *labels[] = { "1 - BEGINNER", "2 - STANDARD", "3 - RANDOMIZED" };
    for (int i = 0; i < 3; i++)
    {
        SDL_Color c = (i == _selected) ? gold : white;
        float y = 300.0f + i * 60.0f;

        if (i == _selected)
        {
            _draw_text(r, _font_small, ">", cx - 160.0f, y, gold);
        }
        _draw_text(r, _font_small, labels[i], cx, y, c);
    }

    /* Controls */
    _draw_text(r, _font_small, "MOVE: WASD OR ARROWS", cx, 540.0f, dim);
    _draw_text(r, _font_small, "INTERACT: SPACE / ENTER", cx, 580.0f, dim);
}
