#include "ui.h"

#include "score.h"
#include "platform.h"
#include "defines.h"

#include <SDL3/SDL.h>

//==============================================================================

void ui_render(void)
{
    const Score *score = score_get();
    SDL_Renderer *renderer = get_renderer();

    float scale = 4.0f;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_SetRenderScale(renderer, scale, scale);

    char text[16];
    SDL_snprintf(text, sizeof(text), "%d", score->p1);
    SDL_RenderDebugText(renderer, WINDOW_WIDTH * 0.25f / scale, 30.0f / scale, text);

    SDL_snprintf(text, sizeof(text), "%d", score->p2);
    SDL_RenderDebugText(renderer, WINDOW_WIDTH * 0.75f / scale, 30.0f / scale, text);

    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}
