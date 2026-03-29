#include "defines.h"
#include "platform.h"
#include "game.h"
#include "event_bus.h"
#include "menu.h"
#include "ui.h"

#include <flecs.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "components/input.h"
#include "components/player.h"
#include "components/transform.h"
#include "components/room_occupant.h"
#include "data/rooms.h"
#include "data/game_modes.h"
#include "managers/audio.h"
#include "managers/component.h"
#include "managers/entity.h"
#include "managers/system.h"
#include "managers/input.h"
#include "systems/fog.h"
#include "systems/particle.h"

/*=============================================================================
 * Static state
 *===========================================================================*/
static ecs_world_t *_world = NULL;
static GameState    _game_state = {0};

GameState *game_state_get(void)
{
    return &_game_state;
}

ecs_world_t *game_world_get(void)
{
    return _world;
}

/*=============================================================================
 * Init managers
 *===========================================================================*/
static void _init_managers(void)
{
    component_manager_init(_world);
    input_manager_init(_world);
    entity_manager_init(_world);
    system_manager_init(_world);
}

/*=============================================================================
 * Scene management
 *===========================================================================*/
static void _handle_scene_input(const SDL_Event *event)
{
    GameState *gs = &_game_state;

    if (gs->scene == SCENE_MENU)
    {
        menu_handle_event(event);
        return;
    }

    if (event->type != SDL_EVENT_KEY_DOWN) return;

    if (gs->scene == SCENE_PLAYING)
    {
        if (event->key.key == SDLK_ESCAPE || event->key.key == SDLK_P)
            gs->scene = SCENE_PAUSED;
    }
    else if (gs->scene == SCENE_PAUSED)
    {
        if (event->key.key == SDLK_ESCAPE || event->key.key == SDLK_P)
            gs->scene = SCENE_PLAYING;
        else if (event->key.key == SDLK_Q)
            game_return_to_menu();
    }
    else if (gs->scene == SCENE_VICTORY)
    {
        if (event->key.key == SDLK_RETURN || event->key.key == SDLK_SPACE ||
            event->key.key == SDLK_ESCAPE)
            game_return_to_menu();
    }
}

static void _scene_update(float dt)
{
    GameState *gs = &_game_state;

    if (gs->scene == SCENE_PLAYING)
    {
        gs->elapsed_time += dt;
    }
    else if (gs->scene == SCENE_DEATH_FREEZE)
    {
        gs->death_timer -= dt;
        if (gs->death_timer <= 0.0f)
        {
            /* Respawn player at gold castle entrance */
            if (_world)
            {
                ecs_query_t *q = ecs_query(_world, {
                    .terms = {
                        { ecs_id(Player),        .inout = EcsInOut },
                        { ecs_id(Transform),     .inout = EcsInOut },
                        { ecs_id(RoomOccupant),  .inout = EcsInOut },
                    }
                });
                ecs_iter_t qit = ecs_query_iter(_world, q);
                while (ecs_query_next(&qit))
                {
                    Transform    *t = ecs_field(&qit, Transform,    1);
                    RoomOccupant *r = ecs_field(&qit, RoomOccupant, 2);
                    for (int i = 0; i < qit.count; i++)
                    {
                        t[i].position = (vector2){480.0f, 400.0f};
                        r[i].room_id  = ROOM_GOLD_ENTRANCE;
                    }
                }
                ecs_query_fini(q);
            }

            gs->current_room = ROOM_GOLD_ENTRANCE;
            gs->scene = SCENE_PLAYING;
        }
    }

    /* Debug toggles */
    const Input *inp = _world ? ecs_singleton_get(_world, Input) : NULL;
    if (inp)
    {
        if (inp->debug_toggle_collision_pressed)
            gs->debug_show_colliders = !gs->debug_show_colliders;
        if (inp->debug_toggle_room_info_pressed)
            gs->debug_show_room_info = !gs->debug_show_room_info;
        if (inp->debug_toggle_entity_list_pressed)
            gs->debug_show_entity_list = !gs->debug_show_entity_list;
    }
}

/*=============================================================================
 * Public API
 *===========================================================================*/
void game_start_mode(int mode)
{
    GameState *gs = &_game_state;

    if (_world)
        entity_manager_clear_gameplay(_world);

    gs->scene        = SCENE_PLAYING;
    gs->mode         = mode;
    gs->elapsed_time = 0.0f;
    gs->death_timer  = 0.0f;
    gs->current_room = ROOM_GOLD_ENTRANCE;
    gs->debug_show_colliders    = false;
    gs->debug_show_room_info    = false;
    gs->debug_show_entity_list  = false;

    if (_world)
        entity_manager_spawn_world(_world, mode);
}

void game_return_to_menu(void)
{
    GameState *gs = &_game_state;

    if (_world)
        entity_manager_clear_gameplay(_world);

    gs->scene = SCENE_MENU;
}

bool game_init(void)
{
    if (!init_window(WINDOW_WIDTH, WINDOW_HEIGHT, GAME_NAME))
        return false;

    if (!TTF_Init())
    {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        close_window();
        return false;
    }

    event_bus_init();
    audio_init();
    fog_init();
    particles_init();

    /* Precompute BFS distances */
    rooms_compute_distances();

    _world = ecs_init();
    if (!_world)
    {
        SDL_Log("ecs_init failed");
        fog_fini();
        audio_fini();
        TTF_Quit();
        close_window();
        return false;
    }

    _init_managers();
    menu_init();
    ui_init();

    _game_state = (GameState){0};
    _game_state.scene = SCENE_MENU;
    return true;
}

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
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
                break;
            }
            _handle_scene_input(&event);
        }
        if (!running) break;

        float delta = get_delta_time();

        _scene_update(delta);

        /* ECS progress — systems run and do their own scene gating */
        ecs_progress(_world, delta);

        /* UI overlay (menu, HUD, pause, victory) */
        ui_render();

        SDL_RenderPresent(get_renderer());
    }
}

void game_fini(void)
{
    if (_world)
    {
        ecs_fini(_world);
        _world = NULL;
    }
    ui_fini();
    menu_fini();
    fog_fini();
    audio_fini();
    if (TTF_WasInit())
        TTF_Quit();
    close_window();
}
