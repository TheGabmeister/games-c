#include "galaxian.h"
#include "platform.h"
#include "game_state.h"
#include "drawing.h"
#include "resources.h"
#include <math.h>
#include <stdio.h>

typedef struct {
    Star stars[NUM_STARS];
    TTF_Font *font_large;
    TTF_Font *font_hud;
    float time_acc;
} GameOverState;

static void gameover_init(void *ctx)
{
    GameOverState *gameover = ctx;
    const char *font_path = res_default_font_path();

    SDL_memset(gameover, 0, sizeof(*gameover));
    if (font_path) {
        gameover->font_large = res_load_font(font_path, FONT_LARGE);
        gameover->font_hud = res_load_font(font_path, FONT_HUD);
    }
    starfield_init(gameover->stars, NUM_STARS);
}

static void gameover_update(void *ctx, float dt)
{
    GameOverState *gameover = ctx;

    gameover->time_acc += dt;
    starfield_update(gameover->stars, NUM_STARS, dt);

    if (is_key_pressed(KEY_ENTER)) {
        game_state_switch(STATE_GAMEPLAY);
        return;
    }
    if (is_key_pressed(KEY_ESCAPE)) {
        request_close();
    }
}

static void draw_centered(TTF_Font *font, const char *text, float y, SDL_Color color)
{
    float tw = 0.0f;

    if (!font) {
        return;
    }

    if (measure_text(font, text, &tw, NULL)) {
        draw_text(font, text, (SCREEN_W - tw) * 0.5f, y, color);
    }
}

static void gameover_draw(void *ctx)
{
    GameOverState *gameover = ctx;

    starfield_draw(gameover->stars, NUM_STARS);

    /* GAME OVER */
    draw_centered(gameover->font_large, "GAME OVER", 200, COL_ESCORT);

    /* Score */
    if (gameover->font_hud) {
        char buf[64];
        snprintf(buf, sizeof(buf), "SCORE  %06d", gx_last_score());
        draw_centered(gameover->font_hud, buf, 300, COL_HUD);

        if (gx_is_new_high()) {
            float pulse = 0.5f + 0.5f * sinf(gameover->time_acc * 4.0f);
            SDL_Color gc = COL_FLAGSHIP;
            gc.a = (Uint8)(pulse * 255);
            draw_centered(gameover->font_hud, "NEW HIGH SCORE!", 350, gc);
        } else {
            snprintf(buf, sizeof(buf), "HI SCORE  %06d", gx_high_score());
            draw_centered(gameover->font_hud, buf, 350, COL_HUD_DIM);
        }

        /* Restart prompt */
        float pulse = 0.5f + 0.5f * sinf(gameover->time_acc * 3.0f);
        SDL_Color pc = COL_HUD;
        pc.a = (Uint8)(pulse * 255);
        draw_centered(gameover->font_hud, "PRESS ENTER TO PLAY AGAIN", 500, pc);
    }
}

static void gameover_cleanup(void *ctx)
{
    (void)ctx;
    /* fonts freed by res_free_all */
}

GameState gx_gameover_state(void)
{
    static GameOverState state;

    return (GameState){
        .ctx = &state,
        .init = gameover_init,
        .update = gameover_update,
        .draw = gameover_draw,
        .cleanup = gameover_cleanup,
    };
}
