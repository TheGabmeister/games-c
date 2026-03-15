#include "defines.h"

#define STBRP_STATIC
#define STBTT_STATIC
#define NK_IMPLEMENTATION
#include <nuklear.h>

#include <SDL3/SDL.h>   // <-- add this

#define NK_SDL3_RENDERER_IMPLEMENTATION
#include <nuklear_sdl3_renderer.h>
