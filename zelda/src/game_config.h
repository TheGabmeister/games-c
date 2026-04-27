#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

// Window / layout
#define WINDOW_WIDTH        1024
#define WINDOW_HEIGHT       960
#define TARGET_FPS          60

// Logical resolution (1:1 with window, no scaling)
#define LOGICAL_WIDTH       1024
#define LOGICAL_HEIGHT      960
#define RENDER_SCALE        1

// Tile size in pixels
#define TILE_SIZE           64

// Screen layout
#define HUD_HEIGHT          224
#define HUD_DIVIDER         32
#define PLAY_AREA_Y         (HUD_HEIGHT + HUD_DIVIDER)
#define PLAY_AREA_HEIGHT    (WINDOW_HEIGHT - HUD_HEIGHT - HUD_DIVIDER)
#define SCREEN_TILES_X      16
#define SCREEN_TILES_Y      11

// Overworld grid
#define OVERWORLD_COLS      16
#define OVERWORLD_ROWS      8
#define START_SCREEN_X      7
#define START_SCREEN_Y      4
#define SCREEN_PATH_MAX     64

// Transitions
#define SCROLL_FRAMES       30
#define FADE_OUT_FRAMES     15
#define FADE_IN_FRAMES      15

// Warps
#define MAX_WARPS_PER_SCREEN 4
#define WARP_DEST_MAX        32

// Player
#define PLAYER_LIVES        3
#define PLAYER_SPEED        256.0f
#define PLAYER_MAX_HEALTH   6
#define PLAYER_ANIM_FRAMES  8

#define GAMEPAD_DEADZONE    0.5f

#endif
