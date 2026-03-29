#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <flecs.h>

/*=============================================================================
 * Game scenes
 *===========================================================================*/
typedef enum GameScene {
    SCENE_MENU,
    SCENE_PLAYING,
    SCENE_PAUSED,
    SCENE_DEATH_FREEZE,
    SCENE_VICTORY
} GameScene;

/*=============================================================================
 * Central game state (non-ECS)
 *===========================================================================*/
typedef struct GameState {
    GameScene scene;
    int       mode;               /* 1=beginner, 2=standard, 3=randomized */
    float     elapsed_time;
    float     death_timer;
    int       current_room;       /* cached from player RoomOccupant */
    bool      debug_show_colliders;
    bool      debug_show_room_info;
    bool      debug_show_entity_list;
} GameState;

GameState      *game_state_get(void);
ecs_world_t    *game_world_get(void);

bool  game_init(void);
void  game_loop(void);
void  game_fini(void);
void  game_start_mode(int mode);
void  game_return_to_menu(void);

#endif
