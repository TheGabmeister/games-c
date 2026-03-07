#include "ghost.h"
#include "maze.h"
#include "score.h"
#include <math.h>

/* ---- Direction helpers -------------------------------------------- */
static Direction opposite_dir(Direction d) {
    switch (d) {
        case DIR_LEFT:  return DIR_RIGHT;
        case DIR_RIGHT: return DIR_LEFT;
        case DIR_UP:    return DIR_DOWN;
        case DIR_DOWN:  return DIR_UP;
        default:        return DIR_NONE;
    }
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

static float tile_dist2(TilePos a, TilePos b) {
    float dc2 = (float)(a.col - b.col);
    float dr2 = (float)(a.row - b.row);
    return dc2*dc2 + dr2*dr2;
}

/* ---- Mode schedule ----------------------------------------------- */
static const ModeEntry DEFAULT_SCHEDULE[8] = {
    { GSTATE_SCATTER, 7.0f  },
    { GSTATE_CHASE,   20.0f },
    { GSTATE_SCATTER, 7.0f  },
    { GSTATE_CHASE,   20.0f },
    { GSTATE_SCATTER, 5.0f  },
    { GSTATE_CHASE,   20.0f },
    { GSTATE_SCATTER, 5.0f  },
    { GSTATE_CHASE,   0.0f  }, /* infinite */
};

/* ---- Scatter / Chase target -------------------------------------- */
TilePos ghost_scatter_target(GhostId id) {
    switch (id) {
        case GHOST_BLINKY: return (TilePos){ 25,  0 };
        case GHOST_PINKY:  return (TilePos){  2,  0 };
        case GHOST_INKY:   return (TilePos){ 27, 35 };
        case GHOST_CLYDE:  return (TilePos){  0, 35 };
        default:           return (TilePos){  0,  0 };
    }
}

TilePos ghost_chase_target(const GameState *gs, const Ghost *g) {
    const Player *p = &gs->player;
    TilePos pt = pixel_to_tile(p->x, p->y);

    switch (g->id) {
        case GHOST_BLINKY:
            return pt;

        case GHOST_PINKY: {
            int ahead_c = pt.col + dir_dc(p->dir) * 4;
            int ahead_r = pt.row + dir_dr(p->dir) * 4;
            if (p->dir == DIR_UP) ahead_c -= 4; /* original UP overflow */
            return (TilePos){ ahead_c, ahead_r };
        }

        case GHOST_INKY: {
            int pivot_c = pt.col + dir_dc(p->dir) * 2;
            int pivot_r = pt.row + dir_dr(p->dir) * 2;
            if (p->dir == DIR_UP) pivot_c -= 2;
            TilePos bl = pixel_to_tile(gs->ghosts[GHOST_BLINKY].x,
                                       gs->ghosts[GHOST_BLINKY].y);
            return (TilePos){
                bl.col + 2 * (pivot_c - bl.col),
                bl.row + 2 * (pivot_r - bl.row)
            };
        }

        case GHOST_CLYDE: {
            TilePos bt = pixel_to_tile(g->x, g->y);
            if (tile_dist2(bt, pt) > 64.0f) /* >8 tiles */
                return pt;
            return ghost_scatter_target(GHOST_CLYDE);
        }

        default: return pt;
    }
}

/* ---- Pathfinding ------------------------------------------------- */
/* Choose the best direction leaving `tile` toward `target`.
   Priority when equidistant: UP > LEFT > DOWN > RIGHT.
   Never reverses. Ghosts can pass GHOST_DOOR only when in_house or leaving. */
static Direction choose_dir(const GameState *gs, const Ghost *g,
                             TilePos tile, TilePos target) {
    static const Direction PRIO[4] = { DIR_UP, DIR_LEFT, DIR_DOWN, DIR_RIGHT };
    Direction rev = opposite_dir(g->dir);
    Direction best = DIR_NONE;
    float best_d = 1e30f;

    for (int i = 0; i < 4; i++) {
        Direction d = PRIO[i];
        if (d == rev) continue;

        int nc = tile.col + dir_dc(d);
        int nr = tile.row + dir_dr(d);
        if (nc < 0) nc = MAZE_COLS - 1;
        if (nc >= MAZE_COLS) nc = 0;

        TileType t = maze_tile(gs, nc, nr);
        if (t == TILE_WALL) continue;
        if (t == TILE_GHOST_DOOR && !g->in_house && !g->leaving) continue;
        if (t == TILE_GHOST_HOUSE && !g->in_house && !g->leaving) continue;

        /* Ghosts can't go UP at the two intersections bordering the ghost house */
        if (d == DIR_UP && !g->in_house) {
            if ((tile.col >= 10 && tile.col <= 17) &&
                (tile.row == 15 || tile.row == 19))
                continue;
        }

        TilePos nb = { nc, nr };
        float d2 = tile_dist2(nb, target);
        if (d2 < best_d) { best_d = d2; best = d; }
    }
    return best != DIR_NONE ? best : rev;
}

static Direction random_dir(const GameState *gs, const Ghost *g, TilePos tile) {
    Direction rev = opposite_dir(g->dir);
    Direction candidates[4];
    int count = 0;
    const Direction ALL[4] = { DIR_UP, DIR_LEFT, DIR_DOWN, DIR_RIGHT };
    for (int i = 0; i < 4; i++) {
        Direction d = ALL[i];
        if (d == rev) continue;
        int nc = tile.col + dir_dc(d);
        int nr = tile.row + dir_dr(d);
        if (nc < 0) nc = MAZE_COLS - 1;
        if (nc >= MAZE_COLS) nc = 0;
        TileType t = maze_tile(gs, nc, nr);
        if (t == TILE_WALL || t == TILE_GHOST_DOOR || t == TILE_GHOST_HOUSE)
            continue;
        candidates[count++] = d;
    }
    if (count == 0) return rev;
    return candidates[GetRandomValue(0, count - 1)];
}

/* ---- Init -------------------------------------------------------- */
static void ghost_reset(GameState *gs, Ghost *g, GhostId id) {
    g->id                  = id;
    g->prev_mode           = GSTATE_SCATTER;
    g->flash_visible       = true;
    g->flash_timer         = 0.0f;
    g->frightened_timer    = 0.0f;
    g->eaten_display_timer = 0.0f;

    GhostMode start_mode = gs->schedule[0].mode;

    switch (id) {
        case GHOST_BLINKY:
            g->x = tile_to_pixel(BLINKY_SPAWN_COL, BLINKY_SPAWN_ROW).x;
            g->y = tile_to_pixel(BLINKY_SPAWN_COL, BLINKY_SPAWN_ROW).y;
            g->dir = DIR_LEFT; g->next_dir = DIR_LEFT;
            g->mode = start_mode;
            g->in_house = false; g->leaving = false;
            break;
        case GHOST_PINKY:
            g->x = tile_to_pixel(PINKY_SPAWN_COL, PINKY_SPAWN_ROW).x;
            g->y = tile_to_pixel(PINKY_SPAWN_COL, PINKY_SPAWN_ROW).y;
            g->dir = DIR_UP; g->next_dir = DIR_UP;
            g->mode = start_mode;
            g->in_house = true; g->leaving = true;
            break;
        case GHOST_INKY:
            g->x = tile_to_pixel(INKY_SPAWN_COL, INKY_SPAWN_ROW).x;
            g->y = tile_to_pixel(INKY_SPAWN_COL, INKY_SPAWN_ROW).y;
            g->dir = DIR_UP; g->next_dir = DIR_UP;
            g->mode = GSTATE_SCATTER;
            g->in_house = true; g->leaving = false;
            break;
        case GHOST_CLYDE:
            g->x = tile_to_pixel(CLYDE_SPAWN_COL, CLYDE_SPAWN_ROW).x;
            g->y = tile_to_pixel(CLYDE_SPAWN_COL, CLYDE_SPAWN_ROW).y;
            g->dir = DIR_UP; g->next_dir = DIR_UP;
            g->mode = GSTATE_SCATTER;
            g->in_house = true; g->leaving = false;
            break;
        default: break;
    }
    g->target = ghost_scatter_target(id);
    g->speed  = gs->ghost_speed;
}

void ghost_init_all(GameState *gs) {
    for (int i = 0; i < 8; i++)
        gs->schedule[i] = DEFAULT_SCHEDULE[i];
    gs->schedule_idx  = 0;
    gs->mode_timer    = 0.0f;
    gs->schedule_done = false;

    for (int i = 0; i < GHOST_COUNT; i++)
        ghost_reset(gs, &gs->ghosts[i], (GhostId)i);
}

/* ---- Schedule ---------------------------------------------------- */
void ghost_update_schedule(GameState *gs, float dt) {
    if (gs->schedule_done) return;

    /* Pause schedule while any ghost is frightened */
    for (int i = 0; i < GHOST_COUNT; i++)
        if (gs->ghosts[i].mode == GSTATE_FRIGHTENED) return;

    gs->mode_timer += dt;
    const ModeEntry *entry = &gs->schedule[gs->schedule_idx];
    if (entry->duration <= 0.0f) return; /* infinite phase */

    if (gs->mode_timer >= entry->duration) {
        gs->mode_timer = 0.0f;
        int next = gs->schedule_idx + 1;
        if (next >= 8) { gs->schedule_done = true; return; }
        GhostMode new_mode = gs->schedule[next].mode;
        gs->schedule_idx = next;

        /* Reverse and switch mode for eligible ghosts */
        for (int i = 0; i < GHOST_COUNT; i++) {
            Ghost *g = &gs->ghosts[i];
            if (g->mode != GSTATE_SCATTER && g->mode != GSTATE_CHASE) continue;
            g->mode = new_mode;
            Direction rev = opposite_dir(g->dir);
            g->dir = rev;
            g->next_dir = rev;
        }
    }
}

/* ---- Release check ----------------------------------------------- */
static void maybe_release(GameState *gs, Ghost *g) {
    if (!g->in_house || g->leaving) return;
    switch (g->id) {
        case GHOST_INKY:
            if (gs->pellets.eaten >= INKY_DOT_THRESHOLD) g->leaving = true;
            break;
        case GHOST_CLYDE:
            if (gs->pellets.eaten >= CLYDE_DOT_THRESHOLD) g->leaving = true;
            break;
        default: break;
    }
}

/* ---- Main update ------------------------------------------------- */
void ghost_update(GameState *gs, Ghost *g, float dt) {
    /* Decay score display timer */
    if (g->eaten_display_timer > 0.0f)
        g->eaten_display_timer -= dt;

    maybe_release(gs, g);

    TilePos cur_tile = pixel_to_tile(g->x, g->y);

    /* Speed */
    float spd;
    if (g->mode == GSTATE_EATEN)
        spd = gs->ghost_speed * 2.0f;
    else if (g->mode == GSTATE_FRIGHTENED)
        spd = gs->ghost_speed * 0.5f;
    else if (maze_is_tunnel(cur_tile.col, cur_tile.row))
        spd = GHOST_TUNNEL_SPEED * TILE_SIZE;
    else
        spd = gs->ghost_speed;

    float move = spd * dt;

    /* ---- House exit ---- */
    if (g->in_house && g->leaving) {
        float center_x = tile_to_pixel(HOUSE_CENTER_COL, 0).x;
        float door_y   = tile_to_pixel(HOUSE_CENTER_COL, GHOST_DOOR_ROW).y;

        /* 1. Center horizontally */
        if (fabsf(g->x - center_x) > 1.0f) {
            float step = (g->x < center_x ? 1.0f : -1.0f) * move;
            if (fabsf(step) > fabsf(g->x - center_x)) step = center_x - g->x;
            g->x += step;
            g->dir = DIR_UP;
        } else {
            g->x = center_x;
            /* 2. Move up to door */
            g->y -= move;
            g->dir = DIR_UP;
            if (g->y <= door_y) {
                g->y = door_y;
                g->in_house = false;
                g->leaving  = false;
                g->dir      = DIR_LEFT;
                g->next_dir = DIR_LEFT;
            }
        }
        return;
    }

    /* ---- Eaten: return to house ---- */
    if (g->mode == GSTATE_EATEN) {
        TilePos door_tile = { HOUSE_CENTER_COL, GHOST_DOOR_ROW };
        Vector2 door_px   = tile_to_pixel(door_tile.col, door_tile.row);

        /* If above the door row, navigate normally */
        if (cur_tile.row < GHOST_DOOR_ROW) {
            TilePos target = door_tile;
            Direction next = choose_dir(gs, g, cur_tile, target);
            g->dir = next;
        } else if (cur_tile.row == GHOST_DOOR_ROW && !g->in_house) {
            /* Arrived at door row — move down into house */
            g->dir = DIR_DOWN;
            g->in_house = true;
        }

        /* Move */
        float nx = g->x + (float)dir_dc(g->dir) * move;
        float ny = g->y + (float)dir_dr(g->dir) * move;
        g->x = nx;
        g->y = ny;

        /* Snap to horizontal center when entering house */
        if (g->in_house) {
            if (fabsf(g->x - door_px.x) > 1.0f) g->x = door_px.x;
        }

        /* Check arrival at house center */
        Vector2 center_px = tile_to_pixel(HOUSE_CENTER_COL, HOUSE_CENTER_ROW);
        if (g->y >= center_px.y) {
            g->x        = center_px.x;
            g->y        = center_px.y;
            g->mode     = g->prev_mode;
            g->in_house = true;
            g->leaving  = true;
        }
        return;
    }

    /* ---- Normal movement (scatter / chase / frightened) ---- */

    /* Compute new position */
    float nx = g->x + (float)dir_dc(g->dir) * move;
    float ny = g->y + (float)dir_dr(g->dir) * move;

    /* Detect tile crossing */
    TilePos old_tp = pixel_to_tile(g->x, g->y);
    TilePos new_tp = pixel_to_tile(nx,   ny);

    /* Warp column */
    if (new_tp.col < 0)          new_tp.col = MAZE_COLS - 1;
    if (new_tp.col >= MAZE_COLS) new_tp.col = 0;

    bool crossed = (old_tp.col != new_tp.col || old_tp.row != new_tp.row);

    if (crossed) {
        /* Check if new tile is passable */
        TileType t = maze_tile(gs, new_tp.col, new_tp.row);
        bool passable = (t != TILE_WALL);
        if (t == TILE_GHOST_DOOR && !g->in_house && !g->leaving) passable = false;
        if (t == TILE_GHOST_HOUSE && !g->in_house && !g->leaving) passable = false;

        if (passable) {
            /* Snap to new tile center */
            Vector2 snap = tile_to_pixel(new_tp.col, new_tp.row);
            g->x = snap.x;
            g->y = snap.y;

            /* Choose next direction from new tile */
            if (g->mode == GSTATE_FRIGHTENED) {
                g->dir = random_dir(gs, g, new_tp);
            } else {
                /* Update target then choose */
                switch (g->mode) {
                    case GSTATE_SCATTER:
                        g->target = ghost_scatter_target(g->id);
                        break;
                    case GSTATE_CHASE:
                        g->target = ghost_chase_target(gs, g);
                        break;
                    default: break;
                }
                g->dir = choose_dir(gs, g, new_tp, g->target);
            }
            g->next_dir = g->dir;
        } else {
            /* Hit a wall: stop at current tile center */
            Vector2 snap = tile_to_pixel(old_tp.col, old_tp.row);
            g->x = snap.x;
            g->y = snap.y;
            /* Try to pick a new valid direction */
            if (g->mode == GSTATE_FRIGHTENED) {
                g->dir = random_dir(gs, g, old_tp);
            } else {
                switch (g->mode) {
                    case GSTATE_SCATTER:
                        g->target = ghost_scatter_target(g->id);
                        break;
                    case GSTATE_CHASE:
                        g->target = ghost_chase_target(gs, g);
                        break;
                    default: break;
                }
                g->dir = choose_dir(gs, g, old_tp, g->target);
            }
        }
    } else {
        /* Mid-tile movement */
        /* Warp x if needed */
        if (nx < 0.0f)
            nx = (float)(MAZE_COLS * TILE_SIZE) - 1.0f;
        else if (nx >= (float)(MAZE_COLS * TILE_SIZE))
            nx = 1.0f;
        g->x = nx;
        g->y = ny;
    }
}
