#ifndef DEFINES_H
#define DEFINES_H

#define GAME_NAME "Adventure"

#define WINDOW_WIDTH  960
#define WINDOW_HEIGHT 720

#define PLAYFIELD_X 0
#define PLAYFIELD_Y 0
#define PLAYFIELD_W 960
#define PLAYFIELD_H 720

#define WALL_THICKNESS 16.0f
#define ROOM_TRANSITION_INSET 12.0f

#define PLAYER_SPEED 200.0f
#define PLAYER_SIZE  16.0f

/* Collision layers */
#define COL_LAYER_PLAYER  (1 << 0)
#define COL_LAYER_WALL    (1 << 1)
#define COL_LAYER_DRAGON  (1 << 2)
#define COL_LAYER_ITEM    (1 << 3)
#define COL_LAYER_BAT     (1 << 4)

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : (v) > (hi) ? (hi) : (v))
#endif

#endif
