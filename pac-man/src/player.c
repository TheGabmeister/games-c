#include "player.h"
#include "maze.h"
#include "pellet.h"
#include "score.h"
#include <math.h>

/* Pac-Man spawns between rows 23 and 24, column 13 */
#define PACMAN_START_COL 13
#define PACMAN_START_ROW 23

/* How close to a tile center (px) before we allow direction change */
#define SNAP_THRESHOLD 2.0f

/* Death animation: shrinks mouth from 0 to 270 over 1.2s, then holds */
#define DEATH_DURATION 1.4f

static Direction key_to_dir(void) {
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) return DIR_LEFT;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) return DIR_RIGHT;
    if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) return DIR_UP;
    if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) return DIR_DOWN;
    return DIR_NONE;
}

static int dir_dc(Direction d) {
    if (d == DIR_LEFT)  return -1;
    if (d == DIR_RIGHT) return  1;
    return 0;
}
static int dir_dr(Direction d) {
    if (d == DIR_UP)   return -1;
    if (d == DIR_DOWN) return  1;
    return 0;
}

void player_reset_position(GameState *gs) {
    Vector2 px = tile_to_pixel(PACMAN_START_COL, PACMAN_START_ROW);
    gs->player.x           = px.x;
    gs->player.y           = px.y;
    gs->player.dir         = DIR_LEFT;
    gs->player.queued_dir  = DIR_NONE;
    gs->player.mouth_angle = 0.0f;
    gs->player.mouth_closing = false;
    gs->player.mouth_timer = 0.0f;
    gs->player.alive       = true;
    gs->player.death_timer = 0.0f;
    gs->player.death_angle = 0.0f;
    gs->player.speed       = gs->pacman_speed;
}

void player_init(GameState *gs) {
    player_reset_position(gs);
}

void player_input(GameState *gs) {
    Direction d = key_to_dir();
    if (d != DIR_NONE)
        gs->player.queued_dir = d;
}

/* Returns the pixel center of the tile col,row is in */
static Vector2 tile_center(int col, int row) {
    return tile_to_pixel(col, row);
}

/* True if px is within SNAP_THRESHOLD of the tile center it's in */
static bool near_center(float px, float py) {
    TilePos tp = pixel_to_tile(px, py);
    Vector2 c  = tile_center(tp.col, tp.row);
    float dx = px - c.x;
    float dy = py - c.y;
    return (dx * dx + dy * dy) <= SNAP_THRESHOLD * SNAP_THRESHOLD;
}

static bool can_move(const GameState *gs, float x, float y, Direction dir) {
    /* Project one step in dir and check if the new tile is passable */
    TilePos tp = pixel_to_tile(x, y);
    int nc = tp.col + dir_dc(dir);
    int nr = tp.row + dir_dr(dir);
    /* Warp column */
    if (nc < 0) nc = MAZE_COLS - 1;
    if (nc >= MAZE_COLS) nc = 0;
    return maze_passable_pacman(gs, nc, nr);
}

void player_update(GameState *gs, float dt) {
    Player *p = &gs->player;
    if (!p->alive) return;

    /* Update mouth animation */
    p->mouth_timer += dt;
    if (p->mouth_timer >= 0.05f) { /* ~20 steps/s */
        p->mouth_timer = 0.0f;
        if (p->mouth_closing) {
            p->mouth_angle -= 5.0f;
            if (p->mouth_angle <= 0.0f) { p->mouth_angle = 0.0f; p->mouth_closing = false; }
        } else {
            p->mouth_angle += 5.0f;
            if (p->mouth_angle >= 45.0f) { p->mouth_angle = 45.0f; p->mouth_closing = true; }
        }
    }

    /* Try to change direction when near a tile center */
    if (near_center(p->x, p->y)) {
        /* Snap exactly to center for precision */
        TilePos tp = pixel_to_tile(p->x, p->y);
        Vector2 c  = tile_center(tp.col, tp.row);
        p->x = c.x;
        p->y = c.y;

        if (p->queued_dir != DIR_NONE && can_move(gs, p->x, p->y, p->queued_dir)) {
            p->dir = p->queued_dir;
            p->queued_dir = DIR_NONE;
        }
    }

    /* Move */
    float dist = p->speed * dt;
    float nx = p->x + dir_dc(p->dir) * dist;
    float ny = p->y + dir_dr(p->dir) * dist;

    /* Check passability of destination tile */
    TilePos dest = pixel_to_tile(nx, ny);
    /* Adjust dest col for warp */
    if (dest.col < 0) dest.col = MAZE_COLS - 1;
    if (dest.col >= MAZE_COLS) dest.col = 0;

    if (maze_passable_pacman(gs, dest.col, dest.row)) {
        p->x = nx;
        p->y = ny;
    } else {
        /* Stop at tile boundary */
        TilePos cur = pixel_to_tile(p->x, p->y);
        Vector2 c   = tile_center(cur.col, cur.row);
        p->x = c.x;
        p->y = c.y;
    }

    /* Warp through tunnels */
    if (p->x < 0.0f)
        p->x = MAZE_COLS * TILE_SIZE - 1.0f;
    else if (p->x >= MAZE_COLS * TILE_SIZE)
        p->x = 1.0f;

    /* Eat tile */
    TilePos tp = pixel_to_tile(p->x, p->y);
    TileType old_type = maze_tile(gs, tp.col, tp.row);
    int pts = maze_eat_tile(gs, tp.col, tp.row);
    if (pts > 0) {
        score_add(gs, pts);
        pellet_on_eat(gs, old_type);
    }

    /* Check fruit collision */
    if (gs->fruit.active) {
        Vector2 fc = tile_to_pixel(FRUIT_SPAWN_COL, FRUIT_SPAWN_ROW);
        float dx = p->x - fc.x;
        float dy = p->y - fc.y;
        if (dx*dx + dy*dy < (float)(TILE_SIZE * TILE_SIZE) * 0.25f) {
            score_add(gs, gs->fruit.score_value);
            gs->fruit.active = false;
        }
    }

    /* Ghost collisions */
    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &gs->ghosts[i];
        float dx = p->x - g->x;
        float dy = p->y - g->y;
        float dist2 = dx*dx + dy*dy;
        float threshold = TILE_SIZE * 0.6f;
        if (dist2 < threshold * threshold) {
            if (g->mode == GSTATE_FRIGHTENED) {
                /* Eat ghost */
                g->mode      = GSTATE_EATEN;
                g->eaten_score = GHOST_BASE_SCORE << gs->score_state.ghost_combo;
                if (g->eaten_score > 1600) g->eaten_score = 1600;
                g->eaten_display_timer = 0.8f;
                score_ghost_eaten(gs);
            } else if (g->mode == GSTATE_SCATTER || g->mode == GSTATE_CHASE) {
                /* Die */
                p->alive       = false;
                p->death_timer = 0.0f;
                p->death_angle = 45.0f;
                gs->phase = GAME_PAC_DYING;
            }
        }
    }
}

bool player_update_death(GameState *gs, float dt) {
    Player *p = &gs->player;
    p->death_timer += dt;
    /* Animate: mouth opens wider then collapses to nothing over DEATH_DURATION */
    float t = p->death_timer / DEATH_DURATION;
    if (t < 0.5f) {
        /* First half: mouth opens to 180 */
        p->death_angle = 45.0f + (180.0f - 45.0f) * (t / 0.5f);
    } else {
        /* Second half: shrink to 0 */
        p->death_angle = 180.0f * (1.0f - (t - 0.5f) / 0.5f);
    }
    return p->death_timer >= DEATH_DURATION;
}
