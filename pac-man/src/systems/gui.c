#include "../defines.h"

#include <SDL3/SDL.h>
#include <engine.h>
#include <nuklear.h>
#include <nuklear_sdl3_renderer.h>

#include "../components/spatial.h"
#include "../managers/gui.h"

#include "gui.h"

//==============================================================================

void gui_render(ecs_iter_t *it)
{
    struct nk_context *ctx = gui_manager_context();
    if (!ctx) return;

    // End input collection started by the previous frame (or init)
    nk_input_end(ctx);

    // Count positioned game entities
    int count = ecs_count(it->world, Spatial);

    char buf[64];
    SDL_snprintf(buf, sizeof(buf), "Entities: %d", count);

    if (nk_begin(ctx, "Debug", nk_rect(10, 10, 160, 55),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE))
    {
        nk_layout_row_dynamic(ctx, 18, 1);
        nk_label(ctx, buf, NK_TEXT_LEFT);
    }
    nk_end(ctx);

    // Render nuklear directly to the screen (not to an off-screen texture)
    SDL_SetRenderTarget(get_renderer(), NULL);
    nk_sdl_render(ctx, NK_ANTI_ALIASING_ON);

    // Begin input collection for the next frame
    nk_input_begin(ctx);
}
