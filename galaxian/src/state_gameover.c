#include "galaxian.h"
#include "platform.h"
#include "game_state.h"
#include "drawing.h"
#include "resources.h"
#include <math.h>
#include <stdio.h>

static Star stars[NUM_STARS];
static TTF_Font *font_large;
static TTF_Font *font_hud;
static float time_acc;

void gameover_init(void)
{
    font_large = res_load_font(FONT_PATH, FONT_LARGE);
    font_hud   = res_load_font(FONT_PATH, FONT_HUD);
    time_acc   = 0;
    starfield_init(stars, NUM_STARS);
}

void gameover_update(float dt)
{
    time_acc += dt;
    starfield_update(stars, NUM_STARS, dt);

    if (is_key_pressed(KEY_ENTER)) {
        game_state_switch(STATE_GAMEPLAY);
        return;
    }
    if (is_key_pressed(KEY_ESCAPE)) {
        request_close();
    }
}

static void draw_centered(TTF_Font *f, const char *text, float y, SDL_Color c)
{
    if (!f) return;
    float tw = draw_text(f, text, -9999, -9999, c);
    /* Overdraw background band to hide the measure pass */
    draw_rect(0, y - 2, (float)SCREEN_W, 50, COL_BG);
    draw_text(f, text, (SCREEN_W - tw) * 0.5f, y, c);
}

void gameover_draw(void)
{
    starfield_draw(stars, NUM_STARS);

    /* GAME OVER */
    draw_centered(font_large, "GAME OVER", 200, COL_ESCORT);

    /* Score */
    if (font_hud) {
        char buf[64];
        snprintf(buf, sizeof(buf), "SCORE  %06d", gx_last_score());
        draw_centered(font_hud, buf, 300, COL_HUD);

        if (gx_is_new_high()) {
            float pulse = 0.5f + 0.5f * sinf(time_acc * 4.0f);
            SDL_Color gc = COL_FLAGSHIP;
            gc.a = (Uint8)(pulse * 255);
            draw_centered(font_hud, "NEW HIGH SCORE!", 350, gc);
        } else {
            snprintf(buf, sizeof(buf), "HI SCORE  %06d", gx_high_score());
            draw_centered(font_hud, buf, 350, COL_HUD_DIM);
        }

        /* Restart prompt */
        float pulse = 0.5f + 0.5f * sinf(time_acc * 3.0f);
        SDL_Color pc = COL_HUD;
        pc.a = (Uint8)(pulse * 255);
        draw_centered(font_hud, "PRESS ENTER TO PLAY AGAIN", 500, pc);
    }
}

void gameover_cleanup(void)
{
    /* fonts freed by res_free_all */
}
