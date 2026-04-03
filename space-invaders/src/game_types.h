#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "raylib.h"
#include <stdbool.h>

// --- Window & Display ---
#define SCREEN_W 800
#define SCREEN_H 600
#define COLOR_BG (Color){10, 10, 30, 255}

// --- Play Area ---
#define PLAY_LEFT   20
#define PLAY_RIGHT  780
#define GROUND_Y    580
#define HUD_TOP     8
#define UFO_LANE_Y  40
#define PLAYER_BASE_Y 550
#define SHIELD_TOP_Y  460

// --- Player ---
#define PLAYER_W     30
#define PLAYER_H     20
#define PLAYER_SPEED 300.0f
#define PLAYER_LIVES 3
#define INVULN_DURATION 2.0f
#define DEATH_DELAY  1.5f

// --- Aliens ---
#define ALIEN_COLS   11
#define ALIEN_ROWS   5
#define ALIEN_COUNT  (ALIEN_COLS * ALIEN_ROWS)
#define ALIEN_SPACING_X 40
#define ALIEN_SPACING_Y 36
#define ALIEN_ORIGIN_X  80
#define ALIEN_ORIGIN_Y  70
#define ALIEN_STEP_X 4
#define ALIEN_DROP_Y 12
#define ALIEN_W      20
#define ALIEN_H      16

// --- Bullets ---
#define BULLET_W     3
#define BULLET_H     10
#define PBULLET_SPEED 500.0f
#define EBULLET_SPEED 250.0f
#define MAX_ENEMY_BULLETS 3
#define TRAIL_COUNT  3

// --- Shields ---
#define SHIELD_COUNT 4
#define SHIELD_COLS  8
#define SHIELD_ROWS  5
#define SHIELD_CELL  6

// --- Mystery Ship ---
#define UFO_SPEED    150.0f
#define UFO_W        40
#define UFO_H        16

// --- Effects ---
#define PARTICLE_MAX    512
#define FLOAT_TEXT_MAX  8
#define STAR_COUNT      100
#define WAVE_PAUSE      2.0f

// --- Colors ---
#define COL_PLAYER     (Color){0, 255, 255, 255}
#define COL_ALIEN_TOP  (Color){255, 0, 180, 255}
#define COL_ALIEN_MID  (Color){0, 220, 255, 255}
#define COL_ALIEN_BOT  (Color){0, 255, 100, 255}
#define COL_SHIELD     (Color){0, 200, 80, 255}
#define COL_ENEMY_FIRE (Color){255, 140, 0, 255}
#define COL_UI_CYAN    (Color){0, 200, 255, 255}

// --- Enums ---
typedef enum { CELL_INTACT, CELL_FLASH, CELL_DEAD } CellState;
typedef enum { AT_TOP, AT_MID, AT_BOT } AlienType;

// --- Structs ---

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Color color;
    float life;
    float maxLife;
    float size;
    bool active;
} Particle;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Color color;
    bool active;
    Vector2 trail[TRAIL_COUNT];
    int trailN;
} Bullet;

typedef struct {
    bool alive;
    AlienType type;
} Alien;

typedef struct {
    CellState cells[SHIELD_ROWS][SHIELD_COLS];
    float flash[SHIELD_ROWS][SHIELD_COLS];
    float x, y;
} Shield;

typedef struct {
    bool active;
    float x, y;
    float dir;
    int score;
    float glowT;
} UFOShip;

typedef struct {
    Vector2 pos;
    Color color;
    char text[16];
    float life;
    bool active;
} FloatText;

typedef struct {
    float x, y;
    float speed;
    float bright;
} Star;

typedef struct {
    // Player
    float playerX;
    int lives;
    bool playerActive;
    float invulnT;
    float deathDelayT;
    bool dead;

    // Bullets
    Bullet pBullet;
    Bullet eBullets[MAX_ENEMY_BULLETS];

    // Aliens
    Alien aliens[ALIEN_ROWS][ALIEN_COLS];
    float formX, formY;
    float alienDir;
    float stepTimer;
    int aliveCount;
    int animFrame;

    // Shields
    Shield shields[SHIELD_COUNT];

    // UFO
    UFOShip ufo;
    float ufoTimer;
    float ufoSpawnTime;
    bool ufoFromLeft;

    // Score & wave
    int score;
    int wave;

    // Enemy fire
    float eFireTimer;

    // Wave state
    bool waveComplete;
    float wavePauseT;

    // Shake
    float shakeT;
    float shakeAmp;

    // Particles
    Particle particles[PARTICLE_MAX];

    // Float texts
    FloatText floatTexts[FLOAT_TEXT_MAX];

    // Stars
    Star stars[STAR_COUNT];
} GameState;

// --- Inline Helpers ---

static inline AlienType AlienTypeForRow(int row) {
    if (row <= 1) return AT_TOP;
    if (row <= 3) return AT_MID;
    return AT_BOT;
}

static inline int AlienScore(AlienType t) {
    switch (t) {
        case AT_TOP: return 30;
        case AT_MID: return 20;
        default:     return 10;
    }
}

static inline Color AlienColor(AlienType t) {
    switch (t) {
        case AT_TOP: return COL_ALIEN_TOP;
        case AT_MID: return COL_ALIEN_MID;
        default:     return COL_ALIEN_BOT;
    }
}

#endif // GAME_TYPES_H
