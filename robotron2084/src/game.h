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

// --- Hulks ---
#define MAX_HULKS           18
#define HULK_RADIUS         22.0f
#define HULK_SPEED          78.0f
#define HULK_STUN_TIME      0.18f
#define HULK_KNOCKBACK      24.0f

// --- Humans ---
#define MAX_HUMANS          40
#define HUMAN_RADIUS        10.0f
#define HUMAN_SPEED         55.0f
#define WAVE_START_HUMANS   5
#define WAVE_HUMAN_MAX      12

// --- Electrodes ---
#define MAX_ELECTRODES      48
#define ELECTRODE_RADIUS    12.0f
#define WAVE_START_ELECTRODES 10
#define WAVE_ELECTRODE_MAX  24

// --- Floating Text ---
#define MAX_FLOAT_TEXT      48
#define FLOAT_TEXT_LIFETIME 0.85f

// --- Waves ---
#define WAVE_START_GRUNTS   18
#define WAVE_GRUNT_STEP     6
#define RESPAWN_INVULN_TIME 1.25f
#define EXTRA_LIFE_SCORE    25000

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

typedef struct Hulk {
    bool active;
    Vector2 position;
    float speed;
    float radius;
    float stun_timer;
} Hulk;

typedef struct Human {
    bool active;
    int type;
    Vector2 position;
    Vector2 velocity;
    float radius;
    float retarget_timer;
} Human;

typedef struct FloatText {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    int value;
    Color color;
} FloatText;

typedef struct Electrode {
    bool active;
    Vector2 position;
    float radius;
    float pulse;
} Electrode;

typedef struct Game {
    GameMode mode;

    int score;
    int high_score;
    int lives;
    int wave;
    int next_extra_life_score;
    int humans_rescued_this_wave;

    Vector2 player_position;
    float player_speed;
    float player_radius;
    Color player_color;
    float player_fire_timer;
    float player_invulnerable_timer;

    Bullet bullets[MAX_BULLETS];
    Grunt grunts[MAX_GRUNTS];
    Hulk hulks[MAX_HULKS];
    Human humans[MAX_HUMANS];
    Electrode electrodes[MAX_ELECTRODES];
    FloatText float_text[MAX_FLOAT_TEXT];

    Sound sounds[SOUND_COUNT];
    bool sounds_loaded;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif
