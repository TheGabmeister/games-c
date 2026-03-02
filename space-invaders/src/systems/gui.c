/* Nuklear setup — must come before any other includes */
#include <SDL3/SDL.h>

#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT

/* Reuse SDL portable types instead of letting Nuklear detect them */
#define NK_INT8         Sint8
#define NK_UINT8        Uint8
#define NK_INT16        Sint16
#define NK_UINT16       Uint16
#define NK_INT32        Sint32
#define NK_UINT32       Uint32
#define NK_SIZE_TYPE    uintptr_t
#define NK_POINTER_TYPE uintptr_t
#define NK_BOOL         bool

#define NK_ASSERT(c)         SDL_assert(c)
#define NK_STATIC_ASSERT(e)  SDL_COMPILE_TIME_ASSERT(, e)
#define NK_MEMSET(d,c,n)     SDL_memset(d,c,n)
#define NK_MEMCPY(d,s,n)     SDL_memcpy(d,s,n)
#define NK_VSNPRINTF(s,n,f,a) SDL_vsnprintf(s,n,f,a)
#define NK_STRTOD(s,e)       SDL_strtod(s,e)
#define NK_INV_SQRT(f)       (1.0f / SDL_sqrtf(f))
#define NK_SIN(f)            SDL_sinf(f)
#define NK_COS(f)            SDL_cosf(f)

/* Forward-declare before the macro referencing it */
static char *nk_sdl_dtoa(char *str, double d);
#define NK_DTOA(s,d) nk_sdl_dtoa(s,d)

/* STB tweaks required by font baking */
#define STBTT_ifloor(x)  ((int)SDL_floor(x))
#define STBTT_iceil(x)   ((int)SDL_ceil(x))
#define STBTT_sqrt(x)    SDL_sqrt(x)
#define STBTT_pow(x,y)   SDL_pow(x,y)
#define STBTT_fmod(x,y)  SDL_fmod(x,y)
#define STBTT_cos(x)     SDL_cosf(x)
#define STBTT_acos(x)    SDL_acos(x)
#define STBTT_fabs(x)    SDL_fabs(x)
#define STBTT_assert(x)  SDL_assert(x)
#define STBTT_strlen(x)  SDL_strlen(x)
#define STBTT_memcpy     SDL_memcpy
#define STBTT_memset     SDL_memset
#define stbtt_uint8      Uint8
#define stbtt_int8       Sint8
#define stbtt_uint16     Uint16
#define stbtt_int16      Sint16
#define stbtt_uint32     Uint32
#define stbtt_int32      Sint32
#define STBRP_SORT       SDL_qsort
#define STBRP_ASSERT     SDL_assert

#define NK_IMPLEMENTATION
#include "nuklear.h"
#define NK_SDL3_RENDERER_IMPLEMENTATION
#include "nuklear_sdl3_renderer.h"

/* Game includes after Nuklear implementation is compiled */
#include <flecs.h>
#include "../components/transform.h"
#include "gui.h"

static struct nk_context       *ctx;
static SDL_Renderer            *nk_renderer;
static enum nk_anti_aliasing    AA;

void gui_system_init(SDL_Window *window, SDL_Renderer *renderer)
{
    struct nk_font_atlas   *atlas;
    struct nk_font_config   config;
    struct nk_font         *font;

    nk_renderer = renderer;

    ctx = nk_sdl_init(window, renderer, nk_sdl_allocator());

    atlas  = nk_sdl_font_stash_begin(ctx);
    config = nk_font_config(0);
    font   = nk_font_atlas_add_default(atlas, 13, &config);
    nk_sdl_font_stash_end(ctx);
    nk_style_set_font(ctx, &font->handle);

    AA = NK_ANTI_ALIASING_ON;

    nk_input_begin(ctx);
}

void gui_system_handle_event(SDL_Event *event)
{
    SDL_ConvertEventToRenderCoordinates(nk_renderer, event);
    nk_sdl_handle_event(ctx, event);
}

void gui_system_run(ecs_world_t *world)
{
    int32_t entity_count = ecs_count(world, Transform);
    char    buf[64];

    nk_input_end(ctx);

    if (nk_begin(ctx, "Debug", nk_rect(10, 10, 180, 60),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE))
    {
        SDL_snprintf(buf, sizeof(buf), "Entities: %d", (int)entity_count);
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, buf, NK_TEXT_LEFT);
    }
    nk_end(ctx);

    nk_sdl_render(ctx, AA);
    nk_sdl_update_TextInput(ctx);

    nk_input_begin(ctx);
}

void gui_system_destroy(void)
{
    nk_sdl_shutdown(ctx);
}

static char *
nk_sdl_dtoa(char *str, double d)
{
    NK_ASSERT(str);
    if (!str) return NULL;
    (void)SDL_snprintf(str, 99999, "%.17g", d);
    return str;
}
