#include "galaxian.h"
#include "drawing.h"
#include <math.h>
#include <stdlib.h>

/* ==== Session state ==== */

static int s_high_score = 0;
static int s_last_score = 0;
static bool s_new_high  = false;

int  gx_high_score(void)        { return s_high_score; }
void gx_set_high_score(int s)   { s_high_score = s; }
int  gx_last_score(void)        { return s_last_score; }
void gx_set_last_score(int s)   { s_last_score = s; }
bool gx_is_new_high(void)       { return s_new_high; }
void gx_set_new_high(bool v)    { s_new_high = v; }

/* ==== Difficulty table ==== */

static const DiffParams DIFF_TABLE[] = {
    /* stage 1 */ { 3.00f, 170.0f, 2, 1 },
    /* stage 2 */ { 2.55f, 185.0f, 2, 1 },
    /* stage 3 */ { 2.20f, 200.0f, 3, 2 },
    /* stage 4 */ { 1.90f, 220.0f, 3, 2 },
    /* stage 5+*/ { 1.65f, 240.0f, 4, 2 },
};

DiffParams gx_difficulty(int stage)
{
    int idx = stage - 1;
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    return DIFF_TABLE[idx];
}

float gx_first_attack_delay(int stage)
{
    float d = 1.25f - 0.05f * (stage - 1);
    if (d < 0.8f) d = 0.8f;
    return d;
}

float gx_sway_speed(int stage)
{
    return (stage > 3) ? 1.6f : 1.2f;
}

/* ==== Formation setup ==== */

/*  Row 0: cols 4-5   Flagship  (2)
 *  Row 1: cols 2-7   Escort    (6)
 *  Row 2: cols 1-8   Raider    (8)
 *  Row 3: cols 0-9   Drone    (10)
 *  Row 4: cols 0-9   Drone    (10)
 *  Row 5: cols 0-9   Drone    (10)
 */
typedef struct { int row; int col_start; int col_end; EnemyType type; } RowDef;

static const RowDef ROW_DEFS[] = {
    { 0, 4, 5, ENEMY_FLAGSHIP },
    { 1, 2, 7, ENEMY_ESCORT   },
    { 2, 1, 8, ENEMY_RAIDER   },
    { 3, 0, 9, ENEMY_DRONE    },
    { 4, 0, 9, ENEMY_DRONE    },
    { 5, 0, 9, ENEMY_DRONE    },
};

void gx_formation_setup(Enemy enemies[])
{
    int idx = 0;
    for (int r = 0; r < 6; r++) {
        const RowDef *rd = &ROW_DEFS[r];
        for (int c = rd->col_start; c <= rd->col_end; c++) {
            Enemy *e = &enemies[idx++];
            e->type      = rd->type;
            e->state     = ENEMY_IN_FORMATION;
            e->row       = rd->row;
            e->col       = c;
            e->home.x    = FORM_X_OFF + c * FORM_HSPACE;
            e->home.y    = FORM_TOP_Y + rd->row * FORM_VSPACE;
            e->pos       = e->home;
            e->path_t    = 0;
            e->path_dur  = 0;
            e->shimmer   = (float)(rand() % 628) / 100.0f; /* 0..2π */
            e->shots_left = 0;
            e->next_fire_t = 0;
            e->alive     = true;
            e->convoy    = false;
        }
    }
}

/* ==== Bezier ==== */

vector2 bezier_eval(const BezierPath *p, float t)
{
    float u  = 1.0f - t;
    float u2 = u * u;
    float u3 = u2 * u;
    float t2 = t * t;
    float t3 = t2 * t;
    return (vector2){
        u3 * p->p0.x + 3*u2*t * p->p1.x + 3*u*t2 * p->p2.x + t3 * p->p3.x,
        u3 * p->p0.y + 3*u2*t * p->p1.y + 3*u*t2 * p->p2.y + t3 * p->p3.y,
    };
}

void gx_dive_path(Enemy *e, DivePathType type)
{
    float sx = e->pos.x;
    float sy = e->pos.y;
    /* small random jitter so paths feel alive */
    float jx = (float)((rand() % 30) - 15);
    float jy = (float)((rand() % 20) - 10);

    e->path.p0 = (vector2){ sx, sy };

    switch (type) {
    case PATH_HOOK_LEFT:
        e->path.p1 = (vector2){ sx - 120 + jx, sy + 100 + jy };
        e->path.p2 = (vector2){ 300 + jx,      500 + jy };
        e->path.p3 = (vector2){ sx + 40,        820 };
        break;
    case PATH_HOOK_RIGHT:
        e->path.p1 = (vector2){ sx + 120 + jx, sy + 100 + jy };
        e->path.p2 = (vector2){ 300 + jx,      500 + jy };
        e->path.p3 = (vector2){ sx - 40,        820 };
        break;
    case PATH_CENTER_S:
        e->path.p1 = (vector2){ sx - 70 + jx,  sy + 160 + jy };
        e->path.p2 = (vector2){ sx + 70 + jx,  sy + 420 + jy };
        e->path.p3 = (vector2){ sx,              820 };
        break;
    case PATH_WIDE_LOOP:
        e->path.p1 = (vector2){ sx - 150 + jx, sy + 130 + jy };
        e->path.p2 = (vector2){ 300 + jx,      560 + jy };
        e->path.p3 = (vector2){ sx + 50,        820 };
        break;
    default:
        e->path.p1 = e->path.p0;
        e->path.p2 = e->path.p0;
        e->path.p3 = (vector2){ sx, 820 };
        break;
    }
}

void gx_return_path(Enemy *e)
{
    float hx = e->home.x;
    float hy = e->home.y;
    float ox = (float)((rand() % 60) - 30);

    e->path.p0 = (vector2){ hx + ox, -40 };
    e->path.p1 = (vector2){ hx + ox * 0.5f, 30 };
    e->path.p2 = (vector2){ hx, hy - 50 };
    e->path.p3 = (vector2){ hx, hy };
    e->path_dur = 1.2f;
}

/* ==== Attack selection ==== */

int gx_pick_solo_diver(const Enemy enemies[])
{
    /* Prefer outer-edge enemies in formation */
    int best = -1;
    int best_dist = -1;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        if (enemies[i].state != ENEMY_IN_FORMATION) continue;
        if (enemies[i].convoy) continue;
        int center = FORM_COLS / 2;
        int dist = abs(enemies[i].col - center);
        if (dist > best_dist) {
            best_dist = dist;
            best = i;
        }
    }
    return best;
}

bool gx_pick_convoy(const Enemy enemies[], int *flag_idx,
                    int escorts[2], int *esc_count)
{
    /* Find a living flagship in formation */
    int fi = -1;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive && enemies[i].type == ENEMY_FLAGSHIP &&
            enemies[i].state == ENEMY_IN_FORMATION) {
            fi = i;
            break;
        }
    }
    if (fi < 0) return false;

    /* Find up to 2 living escorts in formation (nearest by column) */
    int fc = enemies[fi].col;
    int found = 0;
    int eidx[2] = {-1, -1};

    for (int dist = 0; dist <= FORM_COLS && found < 2; dist++) {
        for (int side = 0; side < 2 && found < 2; side++) {
            int tc = fc + (side == 0 ? -dist : dist);
            if (tc < 0 || tc >= FORM_COLS) continue;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].alive && enemies[i].type == ENEMY_ESCORT &&
                    enemies[i].state == ENEMY_IN_FORMATION &&
                    !enemies[i].convoy && enemies[i].col == tc) {
                    eidx[found++] = i;
                    break;
                }
            }
        }
    }

    if (found == 0) return false; /* flagship prefers convoy with escorts */

    *flag_idx  = fi;
    *esc_count = found;
    escorts[0] = eidx[0];
    escorts[1] = (found > 1) ? eidx[1] : -1;
    return true;
}

int gx_convoy_score(int esc_count, int esc_killed)
{
    if (esc_count == 1) return 200;
    if (esc_count == 2) {
        if (esc_killed >= 2) return 800;
        return 300;
    }
    return 150; /* fallback: normal in-flight */
}

/* ==== Starfield ==== */

void starfield_init(Star stars[], int n)
{
    for (int i = 0; i < n; i++) {
        int layer = i % 3;
        stars[i].x = (float)(rand() % SCREEN_W);
        stars[i].y = (float)(rand() % SCREEN_H);
        stars[i].speed = 15.0f + layer * 18.0f;
        stars[i].brightness = 0.25f + layer * 0.3f;
    }
}

void starfield_update(Star stars[], int n, float dt)
{
    for (int i = 0; i < n; i++) {
        stars[i].y += stars[i].speed * dt;
        if (stars[i].y > SCREEN_H) {
            stars[i].y -= SCREEN_H;
            stars[i].x = (float)(rand() % SCREEN_W);
        }
    }
}

void starfield_draw(const Star stars[], int n)
{
    SDL_Renderer *r = get_renderer();
    for (int i = 0; i < n; i++) {
        Uint8 a = (Uint8)(stars[i].brightness * 255);
        SDL_SetRenderDrawColor(r, 200, 210, 255, a);
        float sz = (stars[i].speed > 40.0f) ? 2.0f : 1.0f;
        SDL_FRect dot = { stars[i].x, stars[i].y, sz, sz };
        SDL_RenderFillRect(r, &dot);
    }
}
