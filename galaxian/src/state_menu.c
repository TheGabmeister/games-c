#include "galaxian.h"
#include "platform.h"
#include "game_state.h"
#include "drawing.h"
#include <math.h>
#include <stdio.h>

/* Small attract-mode enemies */
#define ATTRACT_COUNT 12
typedef struct {
    Star stars[NUM_STARS];
    float time_acc;
    float attract_x[ATTRACT_COUNT];
    float attract_y[ATTRACT_COUNT];
    float attract_sway;
} MenuState;

static void menu_init(void *ctx)
{
    MenuState *menu = ctx;

    SDL_memset(menu, 0, sizeof(*menu));
    starfield_init(menu->stars, NUM_STARS);

    /* Place attract enemies in a small grid */
    for (int i = 0; i < ATTRACT_COUNT; i++) {
        int c = i % 6;
        int r = i / 6;
        menu->attract_x[i] = 180.0f + c * 48.0f;
        menu->attract_y[i] = 340.0f + r * 40.0f;
    }
}

static void menu_update(void *ctx, float dt)
{
    MenuState *menu = ctx;

    menu->time_acc += dt;
    menu->attract_sway += dt * 1.0f;
    starfield_update(menu->stars, NUM_STARS, dt);

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

static void menu_draw(void *ctx)
{
    MenuState *menu = ctx;

    starfield_draw(menu->stars, NUM_STARS);

    /* Attract enemies */
    float sx = sinf(menu->attract_sway) * 20.0f;
    for (int i = 0; i < ATTRACT_COUNT; i++) {
        SDL_Color c = (i < 6) ? COL_DRONE : COL_RAIDER;
        c.a = 120;
        draw_attract_enemy(menu->attract_x[i] + sx, menu->attract_y[i], c);
    }

    /* Title */
    {
        float pulse = 0.85f + 0.15f * sinf(menu->time_acc * 2.5f);
        float tw = 0.0f;
        SDL_Color tc = COL_PLAYER;
        tc.a = (Uint8)(pulse * 255);
        if (measure_text("GALAXIAN", &tw, NULL)) {
            draw_text("GALAXIAN", (SCREEN_W - tw) * 0.5f, 150, tc);
        }
    }

    /* High score */
    {
        char buf[64];
        float tw = 0.0f;
        snprintf(buf, sizeof(buf), "HI SCORE  %06d", gx_high_score());
        if (measure_text(buf, &tw, NULL)) {
            draw_text(buf, (SCREEN_W - tw) * 0.5f, 260, COL_HUD_DIM);
        }
    }

    /* Start prompt */
    {
        float pulse = 0.5f + 0.5f * sinf(menu->time_acc * 3.0f);
        float tw = 0.0f;
        SDL_Color pc = COL_HUD;
        pc.a = (Uint8)(pulse * 255);
        const char *msg = "PRESS ENTER TO START";
        if (measure_text(msg, &tw, NULL)) {
            draw_text(msg, (SCREEN_W - tw) * 0.5f, 570, pc);
        }
    }

    /* Controls hint */
    {
        SDL_Color dc = COL_HUD_DIM;
        float tw = 0.0f;
        dc.a = 160;
        const char *ctrl = "ARROWS/WASD: MOVE   SPACE: FIRE   ESC: QUIT";
        if (measure_text(ctrl, &tw, NULL)) {
            draw_text(ctrl, (SCREEN_W - tw) * 0.5f, 710, dc);
        }
    }
}

static void menu_cleanup(void *ctx)
{
    (void)ctx;
}

GameState gx_menu_state(void)
{
    static MenuState state;

    return (GameState){
        .ctx = &state,
        .init = menu_init,
        .update = menu_update,
        .draw = menu_draw,
        .cleanup = menu_cleanup,
    };
}
