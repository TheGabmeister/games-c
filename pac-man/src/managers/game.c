#include "../defines.h"

#include <raylib.h>
#include <engine.h>
#include <flecs.h>
#include <SDL3/SDL.h>

#include "../components/display.h"
#include "../components/time.h"
#include "../systems/scene.h"

#include "texture.h"
#include "sound.h"
#include "music.h"
#include "font.h"
#include "shader.h"
#include "data.h"
#include "component.h"
#include "entity.h"
#include "system.h"
#include "settings.h"
#include "debug.h"
#include "input.h"
#include "gui.h"
#include "physics.h"

#include "game.h"

//==============================================================================

static ecs_world_t *_world = NULL;
static Uint64 millisecsPreviousFrame = 0;

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
  // if (IsAudioDeviceReady())
  // {
  //   CloseAudioDevice();
  // }
  close_window();
}

//------------------------------------------------------------------------------

static inline void _init_flecs(void)
{
  _world = ecs_init();
  ecs_atfini(_world, _fini, NULL);
  // ecs_set_entity_range(_world, 1, 1000);
  // ecs_set_threads(world, 12);
}

//------------------------------------------------------------------------------

static inline void _init_sdl(void)
{
    init_window(600, 800, "PAC-MAN");

    //char *mappings = LoadFileText("./res/gamecontrollerdb.txt");
    //SetGamepadMappings(mappings);
    //UnloadFileText((unsigned char *)mappings);
    
}

//------------------------------------------------------------------------------

static inline void _init_managers(void)
{
  /*
    texture_manager_init(_world);
    sound_manager_init(_world);
    music_manager_init(_world);
    shader_manager_init(_world);
    font_manager_init(_world);
    data_manager_init(_world);
    component_manager_init(_world);
    entity_manager_init(_world);
    debug_manager_init(_world);
    settings_manager_init(_world);
    input_manager_init(_world);
    gui_manager_init(_world);
    physics_manager_init(_world);
    system_manager_init(_world);
    */
}

//------------------------------------------------------------------------------

static inline void _init_game(void)
{
  if (IsWindowReady())
  {
    Texture *icon_texture = texture_manager_get(TEXTURE_SHIP);
    if (icon_texture != NULL && IsTextureValid(*icon_texture))
    {
      Image icon = LoadImageFromTexture(*icon_texture);
      SetWindowIcon(icon);
      UnloadImage(icon);
    }
  }
  //ecs_singleton_set(_world, Display, {.border = BLACK, .background = WHITE, .raster = {0, 0, RASTER_WIDTH, RASTER_HEIGHT}});
  //ecs_singleton_set(_world, Time, {.scale = 1});
}

//------------------------------------------------------------------------------

static inline void _start_game(void)
{

}

//==============================================================================

void game_manager_init(void)
{
  _init_sdl();
  _init_flecs();
  _init_managers();
  _init_game();
}

//------------------------------------------------------------------------------

void game_manager_loop(void)
{
    SDL_Event event;
    bool running = true;
    bool started = false;
    float time = 0;
    if (!is_window_ready())
        return;

    millisecsPreviousFrame = SDL_GetTicksNS();

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT) {
                ecs_quit(_world);
                break;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
                ecs_quit(_world);
                break;
            }
			
	    }

        float delta = (SDL_GetTicksNS() - millisecsPreviousFrame) / 1e9f;
   		millisecsPreviousFrame = SDL_GetTicksNS(); // Store the "previous" frame time

        running = ecs_progress(_world, delta);
/*        time += delta;
        if (!started && time > 1)
        {
            _start_game();
            started = true;
        }
            */
        
    }   
}

//------------------------------------------------------------------------------

void game_manager_fini(void)
{
    ecs_fini(_world);
    _world = NULL;
}
