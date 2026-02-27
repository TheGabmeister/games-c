#ifndef SARSA_H
#define SARSA_H

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define RASTER_WIDTH 640
#define RASTER_HEIGHT 480

#define MAX_SHEEP 10

#define DEBUG_SIZE 20

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_INCLUDE_FIXED_TYPES

/* mandatory: sdl3_renderer depends on those defines */
//#define NK_INCLUDE_COMMAND_USERDATA

//#define NK_IMPLEMENTATION
//#include "nuklear.h"
//#define NK_SDL3_RENDERER_IMPLEMENTATION
//#include "nuklear_sdl3_renderer.h"

#define GUI_FONT_HEIGHT 16u

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#endif