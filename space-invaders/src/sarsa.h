#ifndef SARSA_H
#define SARSA_H

/* mandatory: sdl3_renderer depends on those defines */
//#define NK_INCLUDE_COMMAND_USERDATA

//#define NK_IMPLEMENTATION
//#include "nuklear.h"
//#define NK_SDL3_RENDERER_IMPLEMENTATION
//#include "nuklear_sdl3_renderer.h"

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define ASSETS      "assets/"

#include <stdbool.h>
#include <stdio.h>

#include "stb_image.h"

#include <SDL3/SDL.h>
#include <flecs.h>

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path);


#endif // SARSA_H