#include "defines.h"

#include "platform.h"
#include <flecs.h>
#include <SDL3/SDL.h>

#include "components/input.h"

#include "managers/audio.h"
#include "managers/component.h"
#include "managers/entity.h"
#include "components/shape.h"
#include "components/paddle.h"
#include "components/ball.h"
#include "components/velocity.h"
#include "components/collider.h"
#include "components/transform.h"
#include "managers/system.h"
#include "managers/input.h"

#include "event_bus.h"
#include "score.h"
#include "ui.h"
#include "game.h"

//==============================================================================

static ecs_world_t *_world = NULL;



static inline void _init_managers(void)
{
    component_manager_init(_world);
    input_manager_init(_world);
    entity_manager_init(_world);
    system_manager_init(_world);
}


static void _load_level(void)
{
    Shape paddle = {
        .type = SHAPE_RECTANGLE,
        .color = {255, 255, 255, 255},
        .rectangle = {20, 80}
    };

    ecs_entity_t p1 = entity_manager_spawn_shape(_world, 0, paddle, (vector2){50, WINDOW_HEIGHT * 0.5f});
    ecs_entity_t p2 = entity_manager_spawn_shape(_world, 0, paddle, (vector2){WINDOW_WIDTH - 50, WINDOW_HEIGHT * 0.5f});
    ecs_set(_world, p1, Paddle,   {.player = 1});
    ecs_set(_world, p2, Paddle,   {.player = 2});
    ecs_set(_world, p1, Collider, {.layer = 2, .mask = 1, .type = COLLIDER_RECT, .rect = {20, 80}});
    ecs_set(_world, p2, Collider, {.layer = 2, .mask = 1, .type = COLLIDER_RECT, .rect = {20, 80}});

    float ball_speed = 400.0f;
    ecs_entity_t b = entity_manager_spawn_shape(_world, 0, (Shape){
        .type = SHAPE_RECTANGLE,
        .color = {255, 255, 255, 255},
        .rectangle = {12, 12}
    }, (vector2){WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f});
    vector2 ball_spawn = {WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f};
    ecs_set(_world, b, Ball,     {.speed = ball_speed, .spawn = ball_spawn});
    ecs_set(_world, b, Velocity, {.value = {ball_speed, ball_speed}});
    ecs_set(_world, b, Collider, {.layer = 1, .mask = 2, .type = COLLIDER_RECT, .rect = {12, 12}});

}

bool game_init(void)
{
    if (!init_window(WINDOW_WIDTH, WINDOW_HEIGHT, GAME_NAME))
        return false;

    event_bus_init();
    score_init();
    audio_init();  // opens mixer, loads sounds, subscribes to events
    _world = ecs_init();
    if (!_world)
    {
        SDL_Log("ecs_init failed");
        audio_fini();
        close_window();
        return false;
    }

    _init_managers();
    _load_level();
    return true;
}

//------------------------------------------------------------------------------

void game_loop(void)
{
    SDL_Event event;
    bool running = true;

    if (!is_window_ready() || !_world)
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
        ui_render();
        SDL_RenderPresent(get_renderer());

        //ecs_entities_t entities = ecs_get_entities(_world);
        //SDL_Log("%d", entities.count);

    }   
}

void game_fini(void)
{
    if (_world)
    {
        ecs_fini(_world);
        _world = NULL;
    }
    audio_fini();
    close_window();    // SDL_Quit called last, after mixer is already destroyed
}
