#ifndef DEFINES_H
#define DEFINES_H

/* mandatory: sdl3_renderer depends on those defines */
//#define NK_INCLUDE_COMMAND_USERDATA

//#define NK_IMPLEMENTATION
//#include "nuklear.h"
//#define NK_SDL3_RENDERER_IMPLEMENTATION
//#include "nuklear_sdl3_renderer.h"

#define STB_IMAGE_IMPLEMENTATION

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif


#endif // DEFINES_H