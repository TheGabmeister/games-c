#include "components/transform_comp.h"
#include "components/sprite_comp.h"
#include <SDL3/SDL.h>
#include <flecs.h>

/* mandatory: sdl3_renderer depends on those defines */
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#include "nuklear.h"
#include "nuklear_sdl3_renderer.h"

#include "systems/render_system.h"

static SDL_Renderer      *s_renderer;
static struct nk_context *s_ctx;
static ecs_query_t       *s_query;

void render_system_init(SDL_Window *window, SDL_Renderer *renderer, ecs_world_t *world) {
    s_renderer = renderer;
    s_ctx = nk_sdl_init(window, renderer, nk_sdl_allocator());
    nk_sdl_style_set_debug_font(s_ctx);

    s_query = ecs_query(world, {
        .terms = {
            { ecs_id(Position) },
            { ecs_id(Size)     },
            { ecs_id(Sprite)   }
        }
    });

    /* Prime the first frame's input collection */
    nk_input_begin(s_ctx);
}

struct nk_context *render_system_get_ctx(void) {
    return s_ctx;
}

void render_system_handle_event(SDL_Event *event) {
    nk_sdl_handle_event(s_ctx, event);
}

void render_system_end_input(void) {
    nk_input_end(s_ctx);
}

void render_system_clear(void) {
    SDL_SetRenderDrawColor(s_renderer, 0, 0, 0, 255);
    SDL_RenderClear(s_renderer);
}

void render_system_update(ecs_world_t *world) {
    ecs_iter_t it = ecs_query_iter(world, s_query);
    while (ecs_query_next(&it)) {
        Position *p  = ecs_field(&it, Position, 0);
        Size     *s  = ecs_field(&it, Size,     1);
        Sprite   *sp = ecs_field(&it, Sprite,   2);
        for (int i = 0; i < it.count; i++) {
            SDL_FRect rect = { p[i].x, p[i].y, s[i].w, s[i].h };
            if (sp[i].texture) {
                SDL_RenderTexture(s_renderer, sp[i].texture, NULL, &rect);
            } else {
                SDL_SetRenderDrawColor(s_renderer,
                    sp[i].color.r, sp[i].color.g, sp[i].color.b, sp[i].color.a);
                SDL_RenderFillRect(s_renderer, &rect);
            }
        }
    }
}

void render_system_present(void) {
    nk_sdl_render(s_ctx, NK_ANTI_ALIASING_ON);
    nk_sdl_update_TextInput(s_ctx);
    SDL_RenderPresent(s_renderer);
    nk_input_begin(s_ctx); /* start collecting input for the next frame */
}

void render_system_shutdown(void) {
    ecs_query_fini(s_query);
    s_query = NULL;
    nk_sdl_shutdown(s_ctx);
    s_ctx = NULL;
    s_renderer = NULL;
}
