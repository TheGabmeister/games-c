#include "galaxian.h"
#include "platform.h"
#include "game_state.h"
#include "drawing.h"
#include "resources.h"
#include <math.h>
#include <stdio.h>

static Star stars[NUM_STARS];
static TTF_Font *font_title;
static TTF_Font *font_hud;
static TTF_Font *font_small;
static float time_acc;

/* Small attract-mode enemies */
#define ATTRACT_COUNT 12
static float attract_x[ATTRACT_COUNT];
static float attract_y[ATTRACT_COUNT];
static float attract_sway;

void menu_init(void)
{
    font_title = res_load_font(FONT_PATH, FONT_TITLE);
    font_hud   = res_load_font(FONT_PATH, FONT_HUD);
    font_small = res_load_font(FONT_PATH, FONT_SMALL);
    time_acc   = 0;
    attract_sway = 0;
    starfield_init(stars, NUM_STARS);

    /* Place attract enemies in a small grid */
    for (int i = 0; i < ATTRACT_COUNT; i++) {
        int c = i % 6;
        int r = i / 6;
        attract_x[i] = 180.0f + c * 48.0f;
        attract_y[i] = 340.0f + r * 40.0f;
    }
}

void menu_update(float dt)
{
    time_acc += dt;
    attract_sway += dt * 1.0f;
    starfield_update(stars, NUM_STARS, dt);

    if (is_key_pressed(KEY_ENTER)) {
        game_state_switch(STATE_GAMEPLAY);
        return;
    }
    if (is_key_pressed(KEY_ESCAPE)) {
        request_close();
    }
}

/* Draw a small diamond (attract drone) */
static void draw_attract_enemy(float cx, float cy, SDL_Color col)
{
    float s = 8.0f;
    draw_line(cx, cy - s, cx + s, cy, col);
    draw_line(cx + s, cy, cx, cy + s, col);
    draw_line(cx, cy + s, cx - s, cy, col);
    draw_line(cx - s, cy, cx, cy - s, col);
}

void menu_draw(void)
{
    starfield_draw(stars, NUM_STARS);

    /* Attract enemies */
    float sx = sinf(attract_sway) * 20.0f;
    for (int i = 0; i < ATTRACT_COUNT; i++) {
        SDL_Color c = (i < 6) ? COL_DRONE : COL_RAIDER;
        c.a = 120;
        draw_attract_enemy(attract_x[i] + sx, attract_y[i], c);
    }

    /* Title */
    if (font_title) {
        float pulse = 0.85f + 0.15f * sinf(time_acc * 2.5f);
        SDL_Color tc = COL_PLAYER;
        tc.a = (Uint8)(pulse * 255);
        float tw = draw_text(font_title, "GALAXIAN", 0, 0, tc);
        /* Re-draw centered (draw_text returns width) */
        /* Hack: draw off-screen to measure, then draw centered */
        float tx = (SCREEN_W - tw) * 0.5f;
        /* Clear the off-screen draw and redo */
        draw_rect(0, 140, (float)SCREEN_W, 70, COL_BG);
        draw_text(font_title, "GALAXIAN", tx, 150, tc);
    }

    /* High score */
    if (font_hud) {
        char buf[64];
        snprintf(buf, sizeof(buf), "HI SCORE  %06d", gx_high_score());
        float tw = draw_text(font_hud, buf, 0, 0, COL_HUD);
        draw_rect(0, 250, (float)SCREEN_W, 30, COL_BG);
        draw_text(font_hud, buf, (SCREEN_W - tw) * 0.5f, 260, COL_HUD_DIM);
    }

    /* Start prompt */
    if (font_hud) {
        float pulse = 0.5f + 0.5f * sinf(time_acc * 3.0f);
        SDL_Color pc = COL_HUD;
        pc.a = (Uint8)(pulse * 255);
        const char *msg = "PRESS ENTER TO START";
        float tw = draw_text(font_hud, msg, 0, 0, pc);
        draw_rect(0, 560, (float)SCREEN_W, 30, COL_BG);
        draw_text(font_hud, msg, (SCREEN_W - tw) * 0.5f, 570, pc);
    }

    /* Controls hint */
    if (font_small) {
        SDL_Color dc = COL_HUD_DIM;
        dc.a = 160;
        const char *ctrl = "ARROWS/WASD: MOVE   SPACE: FIRE   ESC: QUIT";
        float tw = draw_text(font_small, ctrl, 0, 0, dc);
        draw_rect(0, 700, (float)SCREEN_W, 24, COL_BG);
        draw_text(font_small, ctrl, (SCREEN_W - tw) * 0.5f, 710, dc);
    }
}

void menu_cleanup(void)
{
    /* fonts freed by res_free_all */
}
