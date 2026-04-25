#ifndef GAME_H
#define GAME_H

#include "game_config.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum SoundID {
    SOUND_START = 0,
    SOUND_PLAYER_SHOOT,
    SOUND_ENEMY_EXPLODE,
    SOUND_PLAYER_DIE,
    SOUND_HUMAN_RESCUE,
    SOUND_EXTRA_LIFE,
    SOUND_WAVE_CLEAR,
    SOUND_ENEMY_SHOOT,
    SOUND_BRAIN_MISSILE,
    SOUND_HULK_HIT,
    SOUND_COUNT
} SoundID;

typedef enum TextureID {
    TEXTURE_PLAYER = 0,
    TEXTURE_GRUNT,
    TEXTURE_HULK,
    TEXTURE_HUMAN_MOMMY,
    TEXTURE_HUMAN_DADDY,
    TEXTURE_HUMAN_MIKEY,
    TEXTURE_ELECTRODE,
    TEXTURE_SPHEROID,
    TEXTURE_ENFORCER,
    TEXTURE_QUARK,
    TEXTURE_TANK,
    TEXTURE_BRAIN,
    TEXTURE_PROG,
    TEXTURE_COUNT
} TextureID;

typedef enum ProjectileType {
    PROJECTILE_SPARK = 0,
    PROJECTILE_SHELL,
    PROJECTILE_CRUISE
} ProjectileType;

typedef enum GameMode {
    GAME_MODE_TITLE = 0,
    GAME_MODE_WAVE_INTRO,
    GAME_MODE_PLAYING,
    GAME_MODE_PAUSED,
    GAME_MODE_PLAYER_DEAD,
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

typedef struct Spheroid {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float speed;
    float radius;
    float spawn_timer;
    float retarget_timer;
} Spheroid;

typedef struct Enforcer {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float speed;
    float radius;
    float shoot_timer;
    float retarget_timer;
} Enforcer;

typedef struct Quark {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float speed;
    float radius;
    float spawn_timer;
    float retarget_timer;
} Quark;

typedef struct Tank {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float speed;
    float radius;
    float shoot_timer;
    float retarget_timer;
} Tank;

typedef struct Brain {
    bool active;
    Vector2 position;
    float speed;
    float radius;
    float shoot_timer;
    float wobble;
} Brain;

typedef struct Prog {
    bool active;
    Vector2 position;
    float speed;
    float radius;
} Prog;

typedef struct EnemyProjectile {
    bool active;
    ProjectileType type;
    Vector2 position;
    Vector2 velocity;
    float radius;
    float lifetime;
    float wobble;
} EnemyProjectile;

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

typedef struct Particle {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float radius;
    float lifetime;
    float max_lifetime;
    Color color;
} Particle;

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
    bool high_score_dirty;
    float mode_timer;

    Vector2 player_position;
    float player_speed;
    float player_radius;
    Color player_color;
    float player_fire_timer;
    float player_invulnerable_timer;

    Bullet bullets[MAX_BULLETS];
    Grunt grunts[MAX_GRUNTS];
    Hulk hulks[MAX_HULKS];
    Spheroid spheroids[MAX_SPHEROIDS];
    Enforcer enforcers[MAX_ENFORCERS];
    Quark quarks[MAX_QUARKS];
    Tank tanks[MAX_TANKS];
    Brain brains[MAX_BRAINS];
    Prog progs[MAX_PROGS];
    EnemyProjectile projectiles[MAX_PROJECTILES];
    Human humans[MAX_HUMANS];
    Electrode electrodes[MAX_ELECTRODES];
    FloatText float_text[MAX_FLOAT_TEXT];
    Particle particles[MAX_PARTICLES];

    float screen_shake;
    float screen_flash;

    Texture2D textures[TEXTURE_COUNT];
    bool textures_loaded;
    Sound sounds[SOUND_COUNT];
    bool sounds_loaded;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);
void game_shutdown(Game *game);

#endif
