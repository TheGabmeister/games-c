#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "raylib.h"

#define SCREEN_WIDTH        800
#define SCREEN_HEIGHT       450

#define MAX_BULLETS         30
#define MAX_ASTEROIDS       64
#define MAX_PARTICLES       256
#define MAX_STARS           100
#define MAX_ASTEROID_VERTS  12

#define SHIP_SIZE           20.0f
#define SHIP_ROTATION_SPEED 270.0f
#define SHIP_THRUST_ACCEL   300.0f
#define SHIP_MAX_SPEED      400.0f
#define SHIP_DRAG           0.98f
#define SHIP_INVULN_TIME    3.0f
#define SHIP_BLINK_RATE     0.1f

#define BULLET_SPEED        500.0f
#define BULLET_LIFETIME     1.5f
#define BULLET_RADIUS       2.0f
#define FIRE_COOLDOWN       0.15f

#define ASTEROID_SPEED_MIN  30.0f
#define ASTEROID_SPEED_MAX  80.0f
#define ASTEROID_LARGE_R    40.0f
#define ASTEROID_MEDIUM_R   20.0f
#define ASTEROID_SMALL_R    10.0f
#define ASTEROID_VERT_MIN   8
#define ASTEROID_VERT_MAX   12
#define ASTEROID_JAGGEDNESS 0.4f

#define PARTICLE_LIFETIME_MIN 0.3f
#define PARTICLE_LIFETIME_MAX 1.2f

#define SCORE_LARGE         20
#define SCORE_MEDIUM        50
#define SCORE_SMALL         100
#define EXTRA_LIFE_SCORE    10000

#define INITIAL_LIVES       3
#define WAVE_DELAY          2.0f
#define SCREEN_SHAKE_DURATION 0.3f
#define SCREEN_SHAKE_MAGNITUDE 6.0f

#define STAR_LAYERS         3

typedef enum ActionType {
    NO_ACTION = 0,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_FIRE,
    MAX_ACTION
} ActionType;

typedef struct ActionInput {
    int key;
    int button;
} ActionInput;

typedef enum GameState {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_GAMEOVER,
} GameState;

typedef enum AsteroidSize {
    ASTEROID_LARGE = 0,
    ASTEROID_MEDIUM,
    ASTEROID_SMALL,
} AsteroidSize;

typedef struct Ship {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float invulnTimer;
    bool alive;
    bool thrusting;
    float fireCooldown;
    float respawnTimer;
} Ship;

typedef struct Bullet {
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    bool active;
} Bullet;

typedef struct Asteroid {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float rotationSpeed;
    float radius;
    AsteroidSize size;
    int vertCount;
    float vertDistances[MAX_ASTEROID_VERTS];
    bool active;
} Asteroid;

typedef struct Particle {
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    float maxLifetime;
    Color color;
    float size;
    bool active;
} Particle;

typedef struct Star {
    Vector2 position;
    float brightness;
    float size;
    int layer;
} Star;

typedef struct Game {
    GameState state;
    int score;
    int highScore;
    int lives;
    int wave;
    float waveTimer;
    bool waveCleared;
    float screenShakeTimer;
    float screenShakeMagnitude;
    int nextExtraLife;
    float gameOverTimer;
} Game;

typedef struct GameContext {
    int gamepadIndex;
    ActionInput actionInputs[MAX_ACTION];
    Game game;
    Ship ship;
    Bullet bullets[MAX_BULLETS];
    Asteroid asteroids[MAX_ASTEROIDS];
    Particle particles[MAX_PARTICLES];
    Star stars[MAX_STARS];
} GameContext;

#endif
