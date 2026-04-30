#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "game_config.h"
#include "player.h"
#include "tilemap.h"
#include "dungeon.h"
#include "camera.h"
#include "enemy/enemy.h"
#include "projectile.h"
#include "pickup.h"
#include "sounds.h"
#include "inventory.h"
#include "save.h"
#include "title.h"
#include "dialogue.h"
#include "shop.h"
#include "world_interact.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum GameState {
    STATE_TITLE = 0,
    STATE_PLAY,
    STATE_TRANSITION,
    STATE_DEATH,
    STATE_PAUSE,
    STATE_ITEM_GET,
    STATE_DIALOGUE,
    STATE_SHOP,
    STATE_CONTINUE,
    STATE_COUNT
} GameState;

typedef struct Game {
    GameState state;
    Player player;
    Screen current_screen;
    Screen next_screen;
    int screen_x;
    int screen_y;

    TransitionCamera cam;
    TransitionType trans_type;
    Vector2 trans_player_start;
    Vector2 trans_player_end;

    int warp_dest_x;
    int warp_dest_y;
    char warp_dest_name[WARP_DEST_MAX];

    bool in_cave;
    int return_screen_x;
    int return_screen_y;
    int return_tile_col;
    int return_tile_row;

    bool in_dungeon;
    DungeonState dungeon;
    int dungeon_return_screen_x;
    int dungeon_return_screen_y;
    int dungeon_return_tile_col;
    int dungeon_return_tile_row;

    int item_get_timer;
    ItemType item_get_type;

    int push_timer;
    bool heavy_rock_pushed[SCREEN_TILES_Y][SCREEN_TILES_X];

    Enemy enemies[MAX_ENEMIES_PER_SCREEN];
    int enemy_count;

    Projectile projectiles[MAX_PROJECTILES];
    int projectile_count;

    Pickup pickups[MAX_PICKUPS];
    int pickup_count;

    PauseState pause_state;
    TitleState title_state;
    DialogueState dialogue_state;
    ShopState shop_state;
    WorldState world;

    int active_save_slot;
    int save_message_timer;
    bool candle_used_this_screen;

    int death_timer;
    int low_health_counter;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif
