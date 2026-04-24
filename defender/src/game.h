#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// --- Window ---
#define WINDOW_WIDTH        1280
#define WINDOW_HEIGHT       720
#define TARGET_FPS          60

// --- World ---
#define WORLD_WIDTH         4096.0f
#define WORLD_HEIGHT        720.0f
#define TERRAIN_SAMPLES     257
#define TERRAIN_BASE_Y      610.0f
#define TERRAIN_VARIANCE    54.0f
#define SKY_TOP             82.0f
#define RADAR_X             320
#define RADAR_Y             20
#define RADAR_WIDTH         640
#define RADAR_HEIGHT        46

// --- Player ---
#define PLAYER_START_X      (WORLD_WIDTH * 0.5f)
#define PLAYER_START_Y      330.0f
#define PLAYER_LIVES        3
#define PLAYER_SMART_BOMBS  3
#define PLAYER_ACCEL        900.0f
#define PLAYER_DRAG         0.88f
#define PLAYER_MAX_SPEED_X  560.0f
#define PLAYER_MAX_SPEED_Y  380.0f
#define PLAYER_RADIUS       18.0f
#define PLAYER_FIRE_COOLDOWN 0.12f
#define PLAYER_RESPAWN_TIME 1.8f
#define EXTRA_LIFE_SCORE    10000

// --- Combat ---
#define MAX_LASERS          14
#define LASER_SPEED         980.0f
#define LASER_LIFETIME      0.65f
#define LASER_LENGTH        84.0f
#define LASER_HIT_RADIUS    24.0f
#define SMART_BOMB_RADIUS   (WINDOW_WIDTH * 0.58f)
#define HYPERSPACE_DEATH_CHANCE 15

// --- Humanoids ---
#define MAX_HUMANOIDS       10
#define HUMANOID_RADIUS     9.0f
#define HUMANOID_FALL_SPEED 145.0f
#define HUMANOID_RESCUE_RADIUS 34.0f
#define HUMANOID_PICKUP_Y   18.0f

// --- Enemies ---
#define MAX_ENEMIES         36
#define LANDER_BASE_SPEED   82.0f
#define MUTANT_BASE_SPEED   190.0f
#define ENEMY_FIRE_COOLDOWN 1.45f
#define ENEMY_RADIUS        18.0f
#define MAX_BULLETS         48
#define BULLET_SPEED        250.0f
#define BULLET_LIFETIME     3.0f

// --- Particles ---
#define MAX_PARTICLES       420

// --- Scoring ---
#define SCORE_LANDER        150
#define SCORE_MUTANT        300
#define SCORE_RESCUE        500
#define SCORE_WAVE_CLEAR    1000
#define SCORE_HUMANOID_SAFE 100

#define PI_F 3.1415926535f

#define COLOR_BG        (Color){ 4, 6, 18, 255 }
#define COLOR_GRID      (Color){ 26, 44, 78, 130 }
#define COLOR_TERRAIN   (Color){ 36, 240, 157, 255 }
#define COLOR_PLAYER    (Color){ 90, 225, 255, 255 }
#define COLOR_LASER     (Color){ 255, 84, 122, 255 }
#define COLOR_LANDER    (Color){ 255, 209, 92, 255 }
#define COLOR_MUTANT    (Color){ 255, 68, 218, 255 }
#define COLOR_BAITER    (Color){ 255, 92, 80, 255 }
#define COLOR_HUMANOID  (Color){ 118, 255, 131, 255 }
#define COLOR_HUD       (Color){ 205, 236, 255, 255 }

typedef enum {
    STATE_TITLE,
    STATE_READY,
    STATE_PLAYING,
    STATE_DYING,
    STATE_WAVE_COMPLETE,
    STATE_GAME_OVER,
} GameState;

typedef enum {
    FACE_LEFT = -1,
    FACE_RIGHT = 1,
} Facing;

typedef enum {
    ENEMY_LANDER,
    ENEMY_MUTANT,
    ENEMY_BAITER,
} EnemyKind;

typedef enum {
    HUMANOID_SAFE,
    HUMANOID_ABDUCTED,
    HUMANOID_FALLING,
    HUMANOID_RESCUED,
    HUMANOID_LOST,
} HumanoidState;

typedef enum {
    SND_SHOOT,
    SND_EXPLOSION,
    SND_RESCUE,
    SND_HUMANOID_LOST,
    SND_SMART_BOMB,
    SND_HYPERSPACE,
    SND_PLAYER_DEATH,
    SND_WAVE_CLEAR,
    SND_EXTRA_LIFE,
    SOUND_COUNT,
} SoundID;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    Facing facing;
    bool alive;
    float fire_timer;
    float invuln_timer;
} PlayerShip;

typedef struct {
    float x;
    float y;
    float vx;
    float life;
    Facing facing;
    bool active;
} Laser;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float life;
    bool active;
} EnemyBullet;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float radius;
    float life;
    float max_life;
    Color color;
    bool active;
} Particle;

typedef struct {
    float height[TERRAIN_SAMPLES];
} Terrain;

typedef struct {
    float x;
    float y;
    float ground_y;
    float vy;
    int carrier;
    HumanoidState state;
} Humanoid;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float phase;
    float fire_timer;
    int target_humanoid;
    bool carrying;
    EnemyKind kind;
    bool active;
} Enemy;

typedef struct {
    GameState state;
    float state_timer;
    int wave;
    int score;
    int high_score;
    int lives;
    int smart_bombs;
    int next_extra_life;

    PlayerShip player;
    Laser lasers[MAX_LASERS];
    EnemyBullet bullets[MAX_BULLETS];
    Enemy enemies[MAX_ENEMIES];
    Humanoid humanoids[MAX_HUMANOIDS];
    Particle particles[MAX_PARTICLES];
    Terrain terrain;

    float camera_x;
    float screen_shake;
    float star_scroll;
    float baiter_timer;
    int humanoid_count;
    int enemy_count;

    Sound sounds[SOUND_COUNT];
    bool sounds_loaded;
} Game;

static inline float wrap_x(float x) {
    while (x < 0.0f) x += WORLD_WIDTH;
    while (x >= WORLD_WIDTH) x -= WORLD_WIDTH;
    return x;
}

static inline float wrapped_delta(float from, float to) {
    float delta = to - from;
    if (delta > WORLD_WIDTH * 0.5f) delta -= WORLD_WIDTH;
    if (delta < -WORLD_WIDTH * 0.5f) delta += WORLD_WIDTH;
    return delta;
}

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

float randf(float min, float max);
float clampf(float value, float min, float max);
float approach(float value, float target, float delta);
float world_to_screen_x(const Game *game, float world_x);
bool near_camera(const Game *game, float world_x, float margin);
float terrain_height_at(const Terrain *terrain, float x);

void add_score(Game *game, int points);
void spawn_particle(Game *game, float x, float y, float vx, float vy, float radius, float life, Color color);
void burst(Game *game, float x, float y, Color color, int count, float speed);
void reset_projectiles(Game *game);
void generate_terrain(Game *game);
void reset_player(Game *game);
void spawn_humanoids(Game *game);
int living_humanoids(const Game *game);
int living_enemies(const Game *game);
void start_wave(Game *game);
void new_game(Game *game);

void update_player(Game *game, float dt);
void update_lasers(Game *game, float dt);
void update_bullets(Game *game, float dt);
void update_enemy_ai(Game *game, float dt);
void update_humanoids(Game *game, float dt);
void update_particles(Game *game, float dt);
void check_wave_complete(Game *game);
void render_game(Game *game);

#endif
