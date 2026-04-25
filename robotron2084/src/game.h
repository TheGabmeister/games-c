#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// --- Window ---
#define WINDOW_WIDTH        1200
#define WINDOW_HEIGHT       900
#define TARGET_FPS          60

// --- Player ---
#define PLAYER_START_X      (WINDOW_WIDTH * 0.5f)
#define PLAYER_START_Y      (WINDOW_HEIGHT * 0.5f)
#define PLAYER_LIVES        3
#define PLAYER_RADIUS       14.0f
#define PLAYER_SPEED        360.0f

// --- Bullets ---
#define MAX_BULLETS         96
#define BULLET_RADIUS       5.0f
#define BULLET_SPEED        760.0f
#define BULLET_LIFETIME     0.8f
#define BULLET_FIRE_RATE    0.08f

// --- Grunts ---
#define MAX_GRUNTS          96
#define GRUNT_RADIUS        13.0f
#define GRUNT_SPEED         115.0f
#define GRUNT_SCORE         100

// --- Waves ---
#define WAVE_START_GRUNTS   18
#define WAVE_GRUNT_STEP     6
#define RESPAWN_INVULN_TIME 1.25f

typedef enum SoundID {
    SOUND_COIN = 0,
    SOUND_COUNT
} SoundID;

typedef enum GameMode {
    GAME_MODE_TITLE = 0,
    GAME_MODE_PLAYING,
    GAME_MODE_PAUSED,
    GAME_MODE_GAME_OVER
} GameMode;

typedef struct Bullet {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float lifetime;
} Bullet;

typedef struct Grunt {
    bool active;
    Vector2 position;
    float speed;
    float radius;
} Grunt;

typedef struct Game {
    GameMode mode;

    int score;
    int high_score;
    int lives;
    int wave;

    Vector2 player_position;
    float player_speed;
    float player_radius;
    Color player_color;
    float player_fire_timer;
    float player_invulnerable_timer;

    Bullet bullets[MAX_BULLETS];
    Grunt grunts[MAX_GRUNTS];

    Sound sounds[SOUND_COUNT];
    bool sounds_loaded;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif
