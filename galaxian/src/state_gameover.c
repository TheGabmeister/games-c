#include "galaxian.h"
#include "platform.h"
#include "game_state.h"
#include "drawing.h"
#include <math.h>
#include <stdio.h>

typedef struct {
    Star stars[NUM_STARS];
    float time_acc;
} GameOverState;

static void gameover_init(void *ctx)
{
    GameOverState *gameover = ctx;

    SDL_memset(gameover, 0, sizeof(*gameover));
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

static void draw_centered(const char *text, float y, SDL_Color color)
{
    float tw = 0.0f;
    if (measure_text(text, &tw, NULL)) {
        draw_text(text, (SCREEN_W - tw) * 0.5f, y, color);
    }
}

static void gameover_draw(void *ctx)
{
    GameOverState *gameover = ctx;

    starfield_draw(gameover->stars, NUM_STARS);

    /* GAME OVER */
    draw_centered("GAME OVER", 200, COL_ESCORT);

    /* Score */
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "SCORE  %06d", gx_last_score());
        draw_centered(buf, 300, COL_HUD);

        if (gx_is_new_high()) {
            float pulse = 0.5f + 0.5f * sinf(gameover->time_acc * 4.0f);
            SDL_Color gc = COL_FLAGSHIP;
            gc.a = (Uint8)(pulse * 255);
            draw_centered("NEW HIGH SCORE!", 350, gc);
        } else {
            snprintf(buf, sizeof(buf), "HI SCORE  %06d", gx_high_score());
            draw_centered(buf, 350, COL_HUD_DIM);
        }

        /* Restart prompt */
        float pulse = 0.5f + 0.5f * sinf(gameover->time_acc * 3.0f);
        SDL_Color pc = COL_HUD;
        pc.a = (Uint8)(pulse * 255);
        draw_centered("PRESS ENTER TO PLAY AGAIN", 500, pc);
    }
}

static void gameover_cleanup(void *ctx)
{
    (void)ctx;
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
