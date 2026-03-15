#include "defines.h"

#include "platform.h"
#include <flecs.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "components/input.h"


#include "managers/texture.h"
#include "managers/sound.h"
#include "managers/music.h"
#include "managers/component.h"
#include "managers/entity.h"
#include "components/shape.h"
#include "managers/system.h"
#include "managers/input.h"

#include "game.h"

//==============================================================================

static ecs_world_t *_world = NULL;



static inline void _init_managers(void)
{
  
    texture_manager_init(_world);
    sound_manager_init(_world);
    music_manager_init(_world);
    //font_manager_init(_world);
    //data_manager_init(_world);
    component_manager_init(_world);
    input_manager_init(_world);
    entity_manager_init(_world);
    //debug_manager_init(_world);
    //settings_manager_init(_world);
    
    //gui_manager_init(_world);
    //physics_manager_init(_world);
    system_manager_init(_world);
    
}


static void _load_level(void)
{
    Shape paddle = {
        .type = SHAPE_RECTANGLE,
        .color = {255, 255, 255, 255},
        .rectangle = {20, 80}
    };

    spawn_shape(_world, 0, paddle, (vector2){50, WINDOW_HEIGHT * 0.5f});
    spawn_shape(_world, 0, paddle, (vector2){WINDOW_WIDTH - 50, WINDOW_HEIGHT * 0.5f});
}

void game_init(void)
{
    init_window(WINDOW_WIDTH, WINDOW_HEIGHT, GAME_NAME);
    _world = ecs_init();
    _init_managers();
    _load_level();
}

//------------------------------------------------------------------------------

void game_loop(void)
{
    SDL_Event event;
    bool running = true;
    float time = 0;

    if (!is_window_ready())
        return;

    while (running)
    {
       
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT) {
                ecs_quit(_world);
            }

            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                ecs_quit(_world);
            }
	    }

        float delta = get_delta_time();
        running = ecs_progress(_world, delta);
        time += delta;

        //ecs_entities_t entities = ecs_get_entities(_world);
        //SDL_Log("%d", entities.count);

    }   
}

void game_fini(void)
{
    ecs_fini(_world);
    _world = NULL;
    close_window();    // SDL_Quit called last, after mixer is already destroyed
}
