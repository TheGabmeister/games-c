#ifndef GALAXIAN_H
#define GALAXIAN_H

#include <stdbool.h>
#include <SDL3/SDL.h>
#include "platform.h"

/* ==== Window / Playfield ==== */
#define SCREEN_W        600
#define SCREEN_H        800
#define HUD_H           48
#define FIELD_LEFT      24.0f
#define FIELD_RIGHT     576.0f
#define FIELD_TOP       72.0f
#define FIELD_BOTTOM    752.0f

/* ==== Player ==== */
#define PLAYER_SPAWN_X  300.0f
#define PLAYER_SPAWN_Y  724.0f
#define PLAYER_SPEED    320.0f
#define PLAYER_LIVES    3
#define PLAYER_W        28.0f
#define PLAYER_H        24.0f
#define PLAYER_HB_SCALE 0.7f
#define RESPAWN_DELAY   0.9f
#define INVULN_TIME     2.0f
#define INVULN_FLASH_HZ 12.0f
#define EXTRA_LIFE_AT   7000

/* ==== Formation ==== */
#define FORM_COLS       10
#define FORM_ROWS       6
#define FORM_HSPACE     40.0f
#define FORM_VSPACE     34.0f
#define FORM_TOP_Y      120.0f
#define FORM_X_OFF      120.0f   /* col-0 center X  (centers the 10-col grid) */
#define FORM_SWAY       28.0f

/* ==== Entities ==== */
#define MAX_ENEMIES       46
#define MAX_ENEMY_BULLETS 6
#define MAX_PARTICLES     768
#define NUM_STARS         80

/* ==== Bullets ==== */
#define PBULLET_SPEED   620.0f
#define PBULLET_W       4.0f
#define PBULLET_H       16.0f
#define EBULLET_W       3.0f
#define EBULLET_H       10.0f
#define ENEMY_FIRE_LINE 680.0f

/* ==== Timing ==== */
#define STAGE_INTRO_TIME   1.0f
#define STAGE_CLEAR_TIME   1.2f
#define SHAKE_DEATH_TIME   0.25f
#define SHAKE_FLAG_TIME    0.08f
#define DEATH_SEQ_TIME     1.5f

/* ==== Swarm ==== */
#define SWARM_THRESHOLD 3

/* ==== Entity sizes (rendered) ==== */
#define DRONE_SIZE      16.0f
#define RAIDER_W        18.0f
#define RAIDER_H        20.0f
#define ESCORT_SIZE     20.0f
#define FLAGSHIP_SIZE   24.0f
#define ENEMY_HB_SCALE  0.85f

/* ==== Font ==== */
#define FONT_PATH       "C:/Windows/Fonts/consola.ttf"
#define FONT_HUD        20.0f
#define FONT_TITLE      48.0f
#define FONT_LARGE      36.0f
#define FONT_SMALL      16.0f

/* ==== Palette (compound literals, use in expression context) ==== */
#define COL_BG           ((SDL_Color){  9,  11,  22, 255})
#define COL_BG2          ((SDL_Color){ 17,  22,  42, 255})
#define COL_PLAYER       ((SDL_Color){ 77, 246, 255, 255})
#define COL_PLAYER_GLOW  ((SDL_Color){  0, 200, 255, 100})
#define COL_DRONE        ((SDL_Color){ 74, 141, 255, 255})
#define COL_DRONE_GLOW   ((SDL_Color){140, 199, 255, 100})
#define COL_RAIDER       ((SDL_Color){155, 107, 255, 255})
#define COL_RAIDER_GLOW  ((SDL_Color){200, 167, 255, 100})
#define COL_ESCORT       ((SDL_Color){255,  74,  94, 255})
#define COL_ESCORT_GLOW  ((SDL_Color){255, 154, 138, 100})
#define COL_FLAGSHIP     ((SDL_Color){255, 212,  77, 255})
#define COL_FLAG_GLOW    ((SDL_Color){255, 241, 168, 100})
#define COL_PBULLET      ((SDL_Color){255, 255, 255, 255})
#define COL_PBULLET_GLOW ((SDL_Color){125, 249, 255, 100})
#define COL_EBULLET      ((SDL_Color){255, 122,  61, 255})
#define COL_EBULLET_GLOW ((SDL_Color){255, 193, 110, 100})
#define COL_HUD          ((SDL_Color){245, 247, 255, 255})
#define COL_HUD_DIM      ((SDL_Color){122, 162, 255, 255})

/* ==== Enums ==== */

typedef enum {
    ENEMY_DRONE,
    ENEMY_RAIDER,
    ENEMY_ESCORT,
    ENEMY_FLAGSHIP
} EnemyType;

typedef enum {
    ENEMY_IN_FORMATION,
    ENEMY_DIVING,
    ENEMY_RETURNING,
    ENEMY_DEAD
} EnemyState;

typedef enum {
    PATH_HOOK_LEFT,
    PATH_HOOK_RIGHT,
    PATH_CENTER_S,
    PATH_WIDE_LOOP,
    PATH_COUNT
} DivePathType;

/* ==== Structs ==== */

typedef struct {
    vector2 p0, p1, p2, p3;
} BezierPath;

typedef struct {
    float x, y;
    float speed;
    float brightness;
} Star;

typedef struct {
    vector2 pos;
    bool alive;
    bool invulnerable;
    float invuln_timer;
    float respawn_timer;
} Player;

typedef struct {
    EnemyType type;
    EnemyState state;
    int row, col;
    vector2 home;
    vector2 pos;
    BezierPath path;
    float path_t;
    float path_dur;
    float shimmer;
    int shots_left;
    float next_fire_t;
    bool alive;
    bool convoy;
} Enemy;

typedef struct {
    vector2 pos;
    vector2 vel;
    bool active;
} Bullet;

typedef struct {
    vector2 pos;
    vector2 vel;
    float life;
    float max_life;
    float size;
    SDL_Color color;
    bool active;
} Particle;

typedef struct {
    float attack_interval;
    float bullet_speed;
    int   max_divers;
    int   shots_per_dive;
} DiffParams;

/* ==== Session state (galaxian.c) ==== */
int  gx_high_score(void);
void gx_set_high_score(int s);
int  gx_last_score(void);
void gx_set_last_score(int s);
bool gx_is_new_high(void);
void gx_set_new_high(bool v);

/* ==== Difficulty ==== */
DiffParams gx_difficulty(int stage);
float gx_first_attack_delay(int stage);
float gx_sway_speed(int stage);

/* ==== Formation ==== */
void gx_formation_setup(Enemy enemies[]);

/* ==== Bezier ==== */
vector2 bezier_eval(const BezierPath *p, float t);
void gx_dive_path(Enemy *e, DivePathType type);
void gx_return_path(Enemy *e);

/* ==== Attack selection ==== */
int  gx_pick_solo_diver(const Enemy enemies[]);
bool gx_pick_convoy(const Enemy enemies[], int *flag_idx,
                    int escorts[2], int *esc_count);
int  gx_convoy_score(int esc_count, int esc_killed);

/* ==== Starfield ==== */
void starfield_init(Star stars[], int n);
void starfield_update(Star stars[], int n, float dt);
void starfield_draw(const Star stars[], int n);

/* ==== State callbacks (registered in main.c) ==== */
void menu_init(void);
void menu_update(float dt);
void menu_draw(void);
void menu_cleanup(void);

void gameplay_init(void);
void gameplay_update(float dt);
void gameplay_draw(void);
void gameplay_cleanup(void);

void gameover_init(void);
void gameover_update(float dt);
void gameover_draw(void);
void gameover_cleanup(void);

#endif /* GALAXIAN_H */
