#include "galaxian.h"
#include "particles.h"
#include "platform.h"
#include "game_state.h"
#include "drawing.h"
#include "resources.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ==== State data ==== */

static Player   player;
static Enemy    enemies[MAX_ENEMIES];
static Bullet   pbullet;          /* single player bullet */
static Bullet   ebullets[MAX_ENEMY_BULLETS];
static Particle particles[MAX_PARTICLES];
static Star     stars[NUM_STARS];

static int   score;
static int   lives;
static int   stage;
static bool  extra_life_given;
static bool  swarm_mode;

static float stage_intro_timer;
static float stage_clear_timer;
static float attack_timer;
static float formation_phase;     /* sway oscillation */

static float shake_timer;
static float shake_str;

/* convoy tracking */
static bool  convoy_active;
static int   convoy_flag;
static int   convoy_esc[2];
static int   convoy_esc_cnt;
static int   convoy_esc_dead;

static TTF_Font *font_hud;
static TTF_Font *font_large;

/* ==== Helpers ==== */

static void enemy_size(EnemyType t, float *w, float *h)
{
    switch (t) {
    case ENEMY_DRONE:    *w = DRONE_SIZE;  *h = DRONE_SIZE;  break;
    case ENEMY_RAIDER:   *w = RAIDER_W;    *h = RAIDER_H;    break;
    case ENEMY_ESCORT:   *w = ESCORT_SIZE; *h = ESCORT_SIZE;  break;
    case ENEMY_FLAGSHIP: *w = FLAGSHIP_SIZE;*h = FLAGSHIP_SIZE;break;
    }
}

static SDL_Color enemy_color(EnemyType t)
{
    switch (t) {
    case ENEMY_DRONE:    return COL_DRONE;
    case ENEMY_RAIDER:   return COL_RAIDER;
    case ENEMY_ESCORT:   return COL_ESCORT;
    case ENEMY_FLAGSHIP: return COL_FLAGSHIP;
    }
    return COL_DRONE;
}

static SDL_Color enemy_glow(EnemyType t)
{
    switch (t) {
    case ENEMY_DRONE:    return COL_DRONE_GLOW;
    case ENEMY_RAIDER:   return COL_RAIDER_GLOW;
    case ENEMY_ESCORT:   return COL_ESCORT_GLOW;
    case ENEMY_FLAGSHIP: return COL_FLAG_GLOW;
    }
    return COL_DRONE_GLOW;
}

static int enemy_particle_count(EnemyType t)
{
    switch (t) {
    case ENEMY_DRONE:    return 12;
    case ENEMY_RAIDER:   return 16;
    case ENEMY_ESCORT:   return 18;
    case ENEMY_FLAGSHIP: return 28;
    }
    return 12;
}

static int score_for_enemy(EnemyType t, bool in_flight)
{
    switch (t) {
    case ENEMY_DRONE:    return in_flight ? 60  : 30;
    case ENEMY_RAIDER:   return in_flight ? 80  : 40;
    case ENEMY_ESCORT:   return in_flight ? 100 : 50;
    case ENEMY_FLAGSHIP: return in_flight ? 150 : 60;
    }
    return 30;
}

static int count_alive(void)
{
    int n = 0;
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].alive) n++;
    return n;
}

static bool all_drones_raiders_dead(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        if (enemies[i].type == ENEMY_DRONE || enemies[i].type == ENEMY_RAIDER)
            return false;
    }
    return true;
}

static float sway_x(void)
{
    return sinf(formation_phase) * FORM_SWAY;
}

/* ==== Stage / reset ==== */

static void setup_stage(void)
{
    gx_formation_setup(enemies);
    stage_intro_timer = STAGE_INTRO_TIME;
    stage_clear_timer = 0;
    attack_timer = gx_first_attack_delay(stage);
    swarm_mode = false;
    convoy_active = false;

    pbullet.active = false;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
        ebullets[i].active = false;
    particles_clear(particles, MAX_PARTICLES);
    shake_timer = 0;
    shake_str = 0;
}

static void reset_run(void)
{
    score = 0;
    lives = PLAYER_LIVES;
    stage = 1;
    extra_life_given = false;

    player.pos = (vector2){ PLAYER_SPAWN_X, PLAYER_SPAWN_Y };
    player.alive = true;
    player.invulnerable = false;
    player.invuln_timer = 0;
    player.respawn_timer = 0;

    formation_phase = 0;
    starfield_init(stars, NUM_STARS);
    setup_stage();
}

/* ==== Enemy bullet firing ==== */

static void fire_enemy_bullet(Enemy *e)
{
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (ebullets[i].active) continue;
        Bullet *b = &ebullets[i];
        b->active = true;
        b->pos = e->pos;
        /* Slight X aim toward player */
        float dx = player.pos.x - e->pos.x;
        float dist = fabsf(dx);
        float vx = (dist > 1.0f) ? (dx / dist) * 30.0f : 0.0f;
        DiffParams dp = gx_difficulty(stage);
        b->vel = (vector2){ vx, dp.bullet_speed };
        e->shots_left--;
        e->next_fire_t += 0.35f;
        return;
    }
}

/* ==== Player death ==== */

static void kill_player(void)
{
    player.alive = false;
    lives--;

    /* Effects */
    particles_spawn(particles, MAX_PARTICLES, player.pos, 32,
                    COL_PLAYER, 250.0f, 4.0f);
    shake_timer = SHAKE_DEATH_TIME;
    shake_str   = 8.0f;

    /* Clear enemy bullets */
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
        ebullets[i].active = false;
    pbullet.active = false;

    /* Swarm mode ends on death */
    swarm_mode = false;

    if (lives > 0) {
        player.respawn_timer = RESPAWN_DELAY;
        /* Return all diving enemies to formation */
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].alive) continue;
            if (enemies[i].state == ENEMY_DIVING || enemies[i].state == ENEMY_RETURNING) {
                enemies[i].state = ENEMY_IN_FORMATION;
                enemies[i].pos = enemies[i].home;
                enemies[i].path_t = 0;
                enemies[i].convoy = false;
            }
        }
        convoy_active = false;
    } else {
        /* Game over after death sequence */
        player.respawn_timer = DEATH_SEQ_TIME;
    }
}

/* ==== Update sub-systems ==== */

static void handle_input(float dt)
{
    if (is_key_pressed(KEY_ESCAPE)) {
        game_state_switch(STATE_MENU);
        return;
    }
    if (!player.alive || player.respawn_timer > 0) return;

    if (is_key_down(KEY_LEFT) || is_key_down(KEY_A))
        player.pos.x -= PLAYER_SPEED * dt;
    if (is_key_down(KEY_RIGHT) || is_key_down(KEY_D))
        player.pos.x += PLAYER_SPEED * dt;

    float hw = PLAYER_W * 0.5f;
    if (player.pos.x < FIELD_LEFT + hw)  player.pos.x = FIELD_LEFT + hw;
    if (player.pos.x > FIELD_RIGHT - hw) player.pos.x = FIELD_RIGHT - hw;

    if (is_key_pressed(KEY_SPACE) && !pbullet.active) {
        pbullet.active = true;
        pbullet.pos = (vector2){ player.pos.x, player.pos.y - PLAYER_H * 0.5f };
        pbullet.vel = (vector2){ 0, -PBULLET_SPEED };
    }
}

static void update_player(float dt)
{
    if (!player.alive) {
        if (player.respawn_timer > 0) {
            player.respawn_timer -= dt;
            if (player.respawn_timer <= 0) {
                if (lives > 0) {
                    player.alive = true;
                    player.pos = (vector2){ PLAYER_SPAWN_X, PLAYER_SPAWN_Y };
                    player.invulnerable = true;
                    player.invuln_timer = INVULN_TIME;
                } else {
                    /* Transition to game over */
                    gx_set_last_score(score);
                    gx_set_new_high(score >= gx_high_score() && score > 0);
                    game_state_switch(STATE_GAME_OVER);
                }
            }
        }
        return;
    }
    if (player.invulnerable) {
        player.invuln_timer -= dt;
        if (player.invuln_timer <= 0)
            player.invulnerable = false;
    }
}

static void update_enemies(float dt)
{
    formation_phase += gx_sway_speed(stage) * dt;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive) continue;

        e->shimmer += dt * 3.0f;

        switch (e->state) {
        case ENEMY_IN_FORMATION:
            e->pos.x = e->home.x + sway_x();
            e->pos.y = e->home.y;
            break;

        case ENEMY_DIVING:
            e->path_t += dt / e->path_dur;
            if (e->path_t >= 1.0f) {
                e->state = ENEMY_RETURNING;
                gx_return_path(e);
                e->path_t = 0;
                /* If this was a convoy escort, check if flagship finished */
            } else {
                e->pos = bezier_eval(&e->path, e->path_t);
                /* Fire? */
                if (e->shots_left > 0 && e->path_t >= e->next_fire_t &&
                    e->pos.y < ENEMY_FIRE_LINE && player.alive) {
                    fire_enemy_bullet(e);
                }
            }
            break;

        case ENEMY_RETURNING:
            e->path_t += dt / e->path_dur;
            if (e->path_t >= 1.0f) {
                e->state = ENEMY_IN_FORMATION;
                e->pos = e->home;
                e->path_t = 0;
                e->convoy = false;

                /* Check if convoy is fully returned */
                if (convoy_active && i == convoy_flag) {
                    convoy_active = false;
                }
            } else {
                e->pos = bezier_eval(&e->path, e->path_t);
            }
            break;

        case ENEMY_DEAD:
            break;
        }
    }
}

static void update_attacks(float dt)
{
    if (stage_intro_timer > 0 || !player.alive || stage_clear_timer > 0) return;

    attack_timer -= dt;
    if (attack_timer > 0) return;

    DiffParams dp = gx_difficulty(stage);
    attack_timer = swarm_mode ? dp.attack_interval * 0.35f : dp.attack_interval;

    /* Count current divers */
    int divers = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive &&
            (enemies[i].state == ENEMY_DIVING || enemies[i].state == ENEMY_RETURNING))
            divers++;
    }
    if (divers >= dp.max_divers) return;

    /* Try convoy first */
    if (!convoy_active) {
        int fi, ei[2], ec;
        if (gx_pick_convoy(enemies, &fi, ei, &ec)) {
            convoy_active = true;
            convoy_flag = fi;
            convoy_esc_cnt = ec;
            convoy_esc_dead = 0;
            for (int j = 0; j < ec; j++) {
                convoy_esc[j] = ei[j];
                enemies[ei[j]].convoy = true;
            }

            /* Launch flagship */
            Enemy *f = &enemies[fi];
            f->state    = ENEMY_DIVING;
            f->path_t   = 0;
            f->path_dur = 2.3f;
            f->shots_left = dp.shots_per_dive;
            f->next_fire_t = 0.3f;
            gx_dive_path(f, PATH_WIDE_LOOP);

            /* Launch escorts */
            for (int j = 0; j < ec; j++) {
                Enemy *esc = &enemies[ei[j]];
                esc->state    = ENEMY_DIVING;
                esc->path_t   = 0;
                esc->path_dur = 2.0f;
                esc->shots_left = dp.shots_per_dive;
                esc->next_fire_t = 0.3f;
                gx_dive_path(esc, (j == 0) ? PATH_HOOK_LEFT : PATH_HOOK_RIGHT);
            }
            return;
        }
    }

    /* Solo dive */
    int idx = gx_pick_solo_diver(enemies);
    if (idx < 0) return;

    Enemy *e = &enemies[idx];
    e->state  = ENEMY_DIVING;
    e->path_t = 0;

    switch (e->type) {
    case ENEMY_DRONE:    e->path_dur = 1.8f; break;
    case ENEMY_RAIDER:   e->path_dur = 2.0f; break;
    case ENEMY_ESCORT:   e->path_dur = 2.0f; break;
    case ENEMY_FLAGSHIP: e->path_dur = 2.3f; break;
    }

    e->shots_left  = dp.shots_per_dive;
    e->next_fire_t = 0.3f;

    DivePathType pt;
    if (e->pos.x < SCREEN_W * 0.5f)
        pt = PATH_HOOK_LEFT;
    else
        pt = PATH_HOOK_RIGHT;
    if (rand() % 3 == 0)
        pt = PATH_CENTER_S;

    gx_dive_path(e, pt);
}

static void update_bullets(float dt)
{
    /* Player bullet */
    if (pbullet.active) {
        pbullet.pos.x += pbullet.vel.x * dt;
        pbullet.pos.y += pbullet.vel.y * dt;
        if (pbullet.pos.y < FIELD_TOP - PBULLET_H)
            pbullet.active = false;
    }

    /* Enemy bullets */
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!ebullets[i].active) continue;
        Bullet *b = &ebullets[i];
        b->pos.x += b->vel.x * dt;
        b->pos.y += b->vel.y * dt;
        if (b->pos.y > SCREEN_H + 20 || b->pos.x < -20 || b->pos.x > SCREEN_W + 20)
            b->active = false;
    }
}

static void check_collisions(void)
{
    float sw = sway_x();

    /* Player bullet vs enemies */
    if (pbullet.active) {
        SDL_FRect br = {
            pbullet.pos.x - PBULLET_W * 0.5f,
            pbullet.pos.y - PBULLET_H * 0.5f,
            PBULLET_W, PBULLET_H
        };
        for (int i = 0; i < MAX_ENEMIES; i++) {
            Enemy *e = &enemies[i];
            if (!e->alive) continue;

            float ew, eh;
            enemy_size(e->type, &ew, &eh);
            ew *= ENEMY_HB_SCALE;
            eh *= ENEMY_HB_SCALE;

            float ex = e->pos.x;
            float ey = e->pos.y;
            if (e->state == ENEMY_IN_FORMATION) {
                ex = e->home.x + sw;
                ey = e->home.y;
            }

            SDL_FRect er = { ex - ew*0.5f, ey - eh*0.5f, ew, eh };
            if (!SDL_HasRectIntersectionFloat(&br, &er)) continue;

            /* Hit! */
            bool in_flight = (e->state == ENEMY_DIVING || e->state == ENEMY_RETURNING);
            pbullet.active = false;

            /* Scoring */
            int pts;
            if (convoy_active && (int)i == convoy_flag && in_flight) {
                pts = gx_convoy_score(convoy_esc_cnt, convoy_esc_dead);
                convoy_active = false;
                /* Free remaining convoy escorts */
                for (int j = 0; j < convoy_esc_cnt; j++) {
                    if (convoy_esc[j] >= 0 && enemies[convoy_esc[j]].alive)
                        enemies[convoy_esc[j]].convoy = false;
                }
            } else {
                pts = score_for_enemy(e->type, in_flight);
            }

            /* Check if this was a convoy escort */
            if (convoy_active) {
                for (int j = 0; j < convoy_esc_cnt; j++) {
                    if (convoy_esc[j] == (int)i) {
                        convoy_esc_dead++;
                        convoy_esc[j] = -1;
                        break;
                    }
                }
            }

            e->alive = false;
            e->state = ENEMY_DEAD;
            score += pts;
            if (score > gx_high_score())
                gx_set_high_score(score);

            /* Particles */
            int pc = enemy_particle_count(e->type);
            float spd = (e->type == ENEMY_FLAGSHIP) ? 200.0f : 150.0f;
            particles_spawn(particles, MAX_PARTICLES, (vector2){ex, ey},
                            pc, enemy_color(e->type), spd, 3.0f);

            /* Flagship shake */
            if (e->type == ENEMY_FLAGSHIP && shake_timer <= 0) {
                shake_timer = SHAKE_FLAG_TIME;
                shake_str   = 4.0f;
            }
            break;
        }
    }

    /* Enemy bullets / enemy bodies vs player */
    if (!player.alive || player.invulnerable) return;

    float phw = PLAYER_W * PLAYER_HB_SCALE * 0.5f;
    float phh = PLAYER_H * PLAYER_HB_SCALE * 0.5f;
    SDL_FRect pr = {
        player.pos.x - phw, player.pos.y - phh,
        phw * 2, phh * 2
    };

    /* Enemy bullets */
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!ebullets[i].active) continue;
        SDL_FRect br = {
            ebullets[i].pos.x - EBULLET_W*0.5f,
            ebullets[i].pos.y - EBULLET_H*0.5f,
            EBULLET_W, EBULLET_H
        };
        if (SDL_HasRectIntersectionFloat(&pr, &br)) {
            kill_player();
            return;
        }
    }

    /* Enemy bodies */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive) continue;
        if (e->state == ENEMY_DEAD) continue;

        float ew, eh;
        enemy_size(e->type, &ew, &eh);
        float ex = e->pos.x;
        float ey = e->pos.y;
        if (e->state == ENEMY_IN_FORMATION) {
            ex = e->home.x + sw;
            ey = e->home.y;
        }

        SDL_FRect er = { ex - ew*0.5f, ey - eh*0.5f, ew, eh };
        if (SDL_HasRectIntersectionFloat(&pr, &er)) {
            /* Kill both */
            e->alive = false;
            e->state = ENEMY_DEAD;
            bool in_flight = (e->state == ENEMY_DIVING || e->state == ENEMY_RETURNING);
            score += score_for_enemy(e->type, in_flight);
            particles_spawn(particles, MAX_PARTICLES, (vector2){ex, ey},
                            enemy_particle_count(e->type), enemy_color(e->type), 150.0f, 3.0f);
            kill_player();
            return;
        }
    }
}

static void check_stage_clear(void)
{
    if (stage_clear_timer > 0 || stage_intro_timer > 0) return;
    if (count_alive() == 0) {
        stage_clear_timer = STAGE_CLEAR_TIME;
    }
}

static void check_swarm(void)
{
    if (swarm_mode) return;
    if (stage_clear_timer > 0 || stage_intro_timer > 0) return;
    int alive = count_alive();
    if (alive > 0 && (alive <= SWARM_THRESHOLD || all_drones_raiders_dead())) {
        swarm_mode = true;
    }
}

static void update_shake(float dt)
{
    if (shake_timer > 0) {
        shake_timer -= dt;
        if (shake_timer < 0) shake_timer = 0;
    }
}

/* ==== Drawing ==== */

static void draw_player_ship(float ox, float oy)
{
    if (!player.alive) return;
    if (player.invulnerable) {
        float t = player.invuln_timer * INVULN_FLASH_HZ;
        if (((int)t) % 2 == 0) return; /* flash off */
    }

    float x = player.pos.x + ox;
    float y = player.pos.y + oy;
    float hw = PLAYER_W * 0.5f;
    float hh = PLAYER_H * 0.5f;

    /* Glow layer */
    draw_triangle(x, y - hh - 2, x - hw - 3, y + hh + 2,
                  x + hw + 3, y + hh + 2, COL_PLAYER_GLOW);

    /* Main body */
    draw_triangle(x, y - hh, x - hw, y + hh, x + hw, y + hh, COL_PLAYER);

    /* Wing bars */
    draw_line(x - hw - 4, y + 4, x - hw + 6, y + 4, COL_PLAYER);
    draw_line(x + hw - 6, y + 4, x + hw + 4, y + 4, COL_PLAYER);

    /* Thruster glow */
    float flicker = 2.0f + sinf(formation_phase * 8.0f) * 1.0f;
    draw_circle(x, y + hh + 2, flicker, COL_PLAYER_GLOW);
}

static void draw_drone(float cx, float cy, float shim)
{
    float s = DRONE_SIZE * 0.5f;
    float pulse = 1.0f + sinf(shim) * 0.08f;
    float sp = s * pulse;

    /* Glow */
    SDL_Color g = COL_DRONE_GLOW;
    draw_line(cx, cy - sp - 2, cx + sp + 2, cy, g);
    draw_line(cx + sp + 2, cy, cx, cy + sp + 2, g);
    draw_line(cx, cy + sp + 2, cx - sp - 2, cy, g);
    draw_line(cx - sp - 2, cy, cx, cy - sp - 2, g);

    /* Outline */
    SDL_Color c = COL_DRONE;
    draw_line(cx, cy - sp, cx + sp, cy, c);
    draw_line(cx + sp, cy, cx, cy + sp, c);
    draw_line(cx, cy + sp, cx - sp, cy, c);
    draw_line(cx - sp, cy, cx, cy - sp, c);

    /* Inner core */
    draw_circle(cx, cy, 2.0f, COL_DRONE_GLOW);
}

static void draw_raider(float cx, float cy, float shim)
{
    float hw = RAIDER_W * 0.5f;
    float hh = RAIDER_H * 0.5f;
    float pulse = 1.0f + sinf(shim) * 0.06f;
    hw *= pulse;
    hh *= pulse;

    /* Kite: top, right, bottom, left */
    float tx = cx, ty = cy - hh;
    float rx = cx + hw, ry = cy - hh * 0.3f;
    float bx = cx, by = cy + hh;
    float lx = cx - hw, ly = cy - hh * 0.3f;

    SDL_Color g = COL_RAIDER_GLOW;
    draw_line(tx, ty - 1, rx + 1, ry, g);
    draw_line(rx + 1, ry, bx, by + 1, g);
    draw_line(bx, by + 1, lx - 1, ly, g);
    draw_line(lx - 1, ly, tx, ty - 1, g);

    SDL_Color c = COL_RAIDER;
    draw_line(tx, ty, rx, ry, c);
    draw_line(rx, ry, bx, by, c);
    draw_line(bx, by, lx, ly, c);
    draw_line(lx, ly, tx, ty, c);
}

static void draw_escort(float cx, float cy, float shim)
{
    float r = ESCORT_SIZE * 0.5f;
    float pulse = 1.0f + sinf(shim) * 0.07f;
    r *= pulse;

    /* Hexagon */
    SDL_Color g = COL_ESCORT_GLOW;
    draw_circle_outline(cx, cy, r + 2, g);

    SDL_Color c = COL_ESCORT;
    float pts[6][2];
    for (int i = 0; i < 6; i++) {
        float a = (float)i * (float)M_PI / 3.0f - (float)M_PI / 6.0f;
        pts[i][0] = cx + cosf(a) * r;
        pts[i][1] = cy + sinf(a) * r;
    }
    for (int i = 0; i < 6; i++) {
        int j = (i + 1) % 6;
        draw_line(pts[i][0], pts[i][1], pts[j][0], pts[j][1], c);
    }

    /* Center dot */
    draw_circle(cx, cy, 3.0f, COL_ESCORT);
}

static void draw_flagship_shape(float cx, float cy, float shim)
{
    float r = FLAGSHIP_SIZE * 0.5f;
    float pulse = 1.0f + sinf(shim) * 0.06f;
    r *= pulse;

    /* Outer glow ring */
    draw_circle_outline(cx, cy, r + 3, COL_FLAG_GLOW);

    /* Inner ring */
    draw_circle_outline(cx, cy, r, COL_FLAGSHIP);

    /* Core */
    draw_circle(cx, cy, r * 0.45f, COL_FLAGSHIP);

    /* Crown rays (upward) */
    SDL_Color c = COL_FLAGSHIP;
    draw_line(cx, cy - r, cx, cy - r - 6, c);
    draw_line(cx - 5, cy - r + 2, cx - 7, cy - r - 4, c);
    draw_line(cx + 5, cy - r + 2, cx + 7, cy - r - 4, c);
}

static void draw_enemies_all(float ox, float oy)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        const Enemy *e = &enemies[i];
        if (!e->alive) continue;

        float ex = e->pos.x + ox;
        float ey = e->pos.y + oy;

        switch (e->type) {
        case ENEMY_DRONE:    draw_drone(ex, ey, e->shimmer);    break;
        case ENEMY_RAIDER:   draw_raider(ex, ey, e->shimmer);   break;
        case ENEMY_ESCORT:   draw_escort(ex, ey, e->shimmer);   break;
        case ENEMY_FLAGSHIP: draw_flagship_shape(ex, ey, e->shimmer); break;
        }
    }
}

static void draw_bullets(float ox, float oy)
{
    /* Player bullet */
    if (pbullet.active) {
        float bx = pbullet.pos.x + ox;
        float by = pbullet.pos.y + oy;
        /* Glow */
        draw_rect(bx - PBULLET_W * 0.5f - 1, by - PBULLET_H * 0.5f - 1,
                  PBULLET_W + 2, PBULLET_H + 2, COL_PBULLET_GLOW);
        /* Core */
        draw_rect(bx - PBULLET_W * 0.5f, by - PBULLET_H * 0.5f,
                  PBULLET_W, PBULLET_H, COL_PBULLET);
    }

    /* Enemy bullets */
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!ebullets[i].active) continue;
        float bx = ebullets[i].pos.x + ox;
        float by = ebullets[i].pos.y + oy;
        /* Glow */
        draw_rect(bx - EBULLET_W * 0.5f - 1, by - EBULLET_H * 0.5f - 1,
                  EBULLET_W + 2, EBULLET_H + 2, COL_EBULLET_GLOW);
        /* Core */
        draw_rect(bx - EBULLET_W * 0.5f, by - EBULLET_H * 0.5f,
                  EBULLET_W, EBULLET_H, COL_EBULLET);
        /* Tail spark */
        draw_circle(bx, by - EBULLET_H * 0.5f - 2, 2.0f, COL_EBULLET_GLOW);
    }
}

static void draw_hud(void)
{
    if (!font_hud) return;
    char buf[64];

    /* Top HUD */
    snprintf(buf, sizeof(buf), "SCORE %06d", score);
    draw_text(font_hud, buf, 12, 12, COL_HUD);

    snprintf(buf, sizeof(buf), "HI %06d", gx_high_score());
    float tw = draw_text(font_hud, buf, -9999, -9999, COL_HUD);
    draw_rect(SCREEN_W - tw - 16, 10, tw + 8, 28, COL_BG);
    draw_text(font_hud, buf, SCREEN_W - tw - 12, 12, COL_HUD_DIM);

    /* Bottom HUD — lives (mini ships) */
    for (int i = 0; i < lives; i++) {
        float lx = 20.0f + i * 22.0f;
        float ly = SCREEN_H - 30.0f;
        draw_triangle(lx, ly - 6, lx - 6, ly + 6, lx + 6, ly + 6, COL_PLAYER);
    }

    /* Bottom HUD — stage number */
    snprintf(buf, sizeof(buf), "STAGE %d", stage);
    tw = draw_text(font_hud, buf, -9999, -9999, COL_HUD);
    draw_rect(SCREEN_W - tw - 16, SCREEN_H - 38, tw + 8, 28, COL_BG);
    draw_text(font_hud, buf, SCREEN_W - tw - 12, SCREEN_H - 36, COL_HUD_DIM);
}

/* ==== Public callbacks ==== */

void gameplay_init(void)
{
    font_hud   = res_load_font(FONT_PATH, FONT_HUD);
    font_large = res_load_font(FONT_PATH, FONT_LARGE);
    srand((unsigned)SDL_GetTicks());
    reset_run();
}

void gameplay_update(float dt)
{
    /* Cap delta to avoid physics explosions on lag/breakpoints */
    if (dt > 0.1f) dt = 0.1f;

    starfield_update(stars, NUM_STARS, dt);
    particles_update(particles, MAX_PARTICLES, dt);
    update_shake(dt);

    /* Stage intro */
    if (stage_intro_timer > 0) {
        stage_intro_timer -= dt;
        return;
    }

    /* Stage clear delay */
    if (stage_clear_timer > 0) {
        stage_clear_timer -= dt;
        if (stage_clear_timer <= 0) {
            stage++;
            setup_stage();
        }
        return;
    }

    handle_input(dt);
    update_player(dt);
    update_enemies(dt);
    update_attacks(dt);
    update_bullets(dt);
    check_collisions();
    check_stage_clear();
    check_swarm();

    /* Extra life */
    if (!extra_life_given && score >= EXTRA_LIFE_AT) {
        extra_life_given = true;
        lives++;
    }

    /* High score live update */
    if (score > gx_high_score())
        gx_set_high_score(score);
}

void gameplay_draw(void)
{
    /* Shake offset */
    float ox = 0, oy = 0;
    if (shake_timer > 0) {
        float frac = shake_timer / SHAKE_DEATH_TIME;
        float mag = shake_str * frac;
        ox = ((rand() % 200) / 100.0f - 1.0f) * mag;
        oy = ((rand() % 200) / 100.0f - 1.0f) * mag;
    }

    starfield_draw(stars, NUM_STARS);
    draw_enemies_all(ox, oy);
    draw_player_ship(ox, oy);
    draw_bullets(ox, oy);
    particles_draw(particles, MAX_PARTICLES, ox, oy);

    /* HUD (no shake) */
    draw_hud();

    /* Stage intro overlay */
    if (stage_intro_timer > 0 && font_large) {
        char buf[32];
        snprintf(buf, sizeof(buf), "STAGE %d", stage);
        float tw = draw_text(font_large, buf, -9999, -9999, COL_HUD);
        draw_rect(0, SCREEN_H * 0.5f - 30, (float)SCREEN_W, 60, COL_BG);
        draw_text(font_large, buf, (SCREEN_W - tw) * 0.5f,
                  SCREEN_H * 0.5f - 20, COL_HUD);
    }

    /* Stage clear overlay */
    if (stage_clear_timer > 0 && font_large) {
        const char *msg = "STAGE CLEAR";
        float tw = draw_text(font_large, msg, -9999, -9999, COL_FLAGSHIP);
        draw_rect(0, SCREEN_H * 0.5f - 30, (float)SCREEN_W, 60, COL_BG);
        draw_text(font_large, msg, (SCREEN_W - tw) * 0.5f,
                  SCREEN_H * 0.5f - 20, COL_FLAGSHIP);
    }
}

void gameplay_cleanup(void)
{
    /* Fonts freed by res_free_all */
}
