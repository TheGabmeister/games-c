#include "../defines.h"

#include <flecs.h>
#include <SDL3/SDL.h>
#include <engine.h>
#include <nuklear.h>
#include <nuklear_sdl3_renderer.h>

#include "gui.h"

//==============================================================================

static struct nk_context *_ctx = NULL;

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
    (void)world;
    (void)context;
    if (_ctx) {
        nk_sdl_shutdown(_ctx);
        _ctx = NULL;
    }
}

//------------------------------------------------------------------------------

void gui_manager_init(ecs_world_t *world)
{
    ecs_atfini(world, _fini, NULL);

    SDL_Renderer *renderer = get_renderer();
    SDL_Window *window = SDL_GetRenderWindow(renderer);

    _ctx = nk_sdl_init(window, renderer, nk_sdl_allocator());

    struct nk_font_atlas *atlas = nk_sdl_font_stash_begin(_ctx);
    struct nk_font_config config = nk_font_config(0);
    struct nk_font *font = nk_font_atlas_add_default(atlas, GUI_FONT_HEIGHT, &config);
    nk_sdl_font_stash_end(_ctx);
    nk_style_set_font(_ctx, &font->handle);

    // Begin the first frame's input collection
    nk_input_begin(_ctx);
}

//------------------------------------------------------------------------------

void gui_manager_handle_event(SDL_Event *event)
{
    if (!_ctx) return;
    SDL_ConvertEventToRenderCoordinates(get_renderer(), event);
    nk_sdl_handle_event(_ctx, event);
}

//------------------------------------------------------------------------------

struct nk_context *gui_manager_context(void)
{
    return _ctx;
}
