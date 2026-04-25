#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

// --- Window ---
#define WINDOW_WIDTH        1200
#define WINDOW_HEIGHT       900
#define TARGET_FPS          60

// --- World ---
#define MAX_PLATFORMS       12
#define LAVA_HEIGHT         64.0f
#define LANDING_TOLERANCE   6.0f

// --- Player ---
#define PLAYER_LIVES         3
#define PLAYER_RADIUS        18.0f
#define PLAYER_FLAP_VELOCITY -430.0f
#define PLAYER_GRAVITY       900.0f
#define PLAYER_ACCEL         850.0f
#define PLAYER_MAX_SPEED_X   260.0f
#define PLAYER_MAX_SPEED_Y   620.0f
#define PLAYER_AIR_DRAG      0.92f
#define PLAYER_FLAP_COOLDOWN 0.12f
#define PLAYER_INVULN_TIME   2.0f

// --- Combat ---
#define ENEMY_RADIUS         18.0f
#define JOUST_WIN_HEIGHT     10.0f
#define JOUST_BOUNCE_SPEED   220.0f

// --- Collections ---
#define MAX_ENEMIES          16
#define MAX_EGGS             16
#define MAX_PARTICLES        256

typedef enum GameMode {
    GAME_MODE_TITLE,
    GAME_MODE_PLAYING,
    GAME_MODE_PAUSED,
    GAME_MODE_WAVE_CLEAR,
    GAME_MODE_PLAYER_DEAD,
    GAME_MODE_GAME_OVER
} GameMode;

typedef enum SoundID {
    SOUND_FLAP,
    SOUND_JOUST_WIN,
    SOUND_JOUST_BOUNCE,
    SOUND_EGG_COLLECT,
    SOUND_EGG_HATCH,
    SOUND_PLAYER_DIE,
    SOUND_WAVE_CLEAR,
    SOUND_START,
    SOUND_LAVA,
    SOUND_PREDATOR,
    SOUND_PAUSE,
    SOUND_COUNT
} SoundID;

typedef enum TextureID {
    TEXTURE_PLAYER_IDLE,
    TEXTURE_PLAYER_FLAP_1,
    TEXTURE_PLAYER_FLAP_2,
    TEXTURE_ENEMY_GRUNT,
    TEXTURE_ENEMY_HUNTER,
    TEXTURE_ENEMY_CHAMPION,
    TEXTURE_EGG,
    TEXTURE_EGG_HATCHING,
    TEXTURE_PLATFORM,
    TEXTURE_LAVA,
    TEXTURE_SPARK,
    TEXTURE_COUNT
} TextureID;

typedef enum EnemyType {
    ENEMY_GRUNT,
    ENEMY_HUNTER,
    ENEMY_CHAMPION
} EnemyType;

typedef enum EggState {
    EGG_FALLING,
    EGG_RESTING,
    EGG_HATCHING
} EggState;

typedef struct Actor {
    Vector2 previous_position;
    Vector2 position;
    Vector2 velocity;
    float radius;
    float flap_cooldown;
    float invuln_timer;
    int facing;
    bool alive;
    bool grounded;
} Actor;

typedef struct Enemy {
    Actor actor;
    EnemyType type;
    float think_timer;
    float target_y;
    float spawn_grace_timer;
    float stuck_timer;
    int escape_dir;
} Enemy;

typedef struct Egg {
    Vector2 previous_position;
    Vector2 position;
    Vector2 velocity;
    EggState state;
    float timer;
    EnemyType hatch_type;
    bool active;
} Egg;

typedef struct Platform {
    Rectangle bounds;
} Platform;

typedef struct Particle {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float lifetime;
    float max_lifetime;
    Color color;
    bool active;
} Particle;

typedef struct GameTexture {
    Texture2D texture;
    bool loaded;
} GameTexture;

typedef struct Game {
    GameMode mode;
    float mode_timer;
    GameMode paused_from;

    Actor player;
    Enemy enemies[MAX_ENEMIES];
    Egg eggs[MAX_EGGS];
    Particle particles[MAX_PARTICLES];
    Platform platforms[MAX_PLATFORMS];
    int platform_count;

    Rectangle lava;
    GameTexture textures[TEXTURE_COUNT];
    Sound sounds[SOUND_COUNT];

    int score;
    int high_score;
    int lives;
    int wave;
    int combo;
    bool debug_draw;
    bool sounds_loaded;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif
