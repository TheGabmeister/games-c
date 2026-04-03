#include "gameover_scene.h"
#include "title_scene.h"
#include "play_scene.h"
#include "../platform.h"
#include "../input.h"
#include "../draw.h"
#include "../gamestate.h"
#include <stdio.h>

static void gameover_init(void) { }

static void gameover_update(float dt)
{
    (void)dt;

    if (action_pressed(ACTION_CONFIRM))
        scene_set(play_scene());

    if (action_pressed(ACTION_CANCEL))
        scene_set(title_scene());
}

static void gameover_draw(void)
{
    int w = get_window_width();

    clear_background((color){ 40, 10, 10, 255 });
    begin_drawing();

    const char *title = "GAME OVER";
    int title_px = 9 * 8 * 4;
    draw_text(title, (w - title_px) / 2.0f, 160, 4.0f, COLOR_RED);

    char buf[64];
    GameState *gs = gamestate_get();
    snprintf(buf, sizeof(buf), "Final Score: %d", gs->score);
    int score_px = (int)strlen(buf) * 8 * 2;
    draw_text(buf, (w - score_px) / 2.0f, 260, 2.0f, COLOR_YELLOW);

    const char *retry = "Press ENTER to Retry";
    int retry_px = 20 * 8 * 2;
    draw_text(retry, (w - retry_px) / 2.0f, 340, 2.0f, COLOR_WHITE);

    const char *quit = "ESC for Title";
    int quit_px = 13 * 8 * 2;
    draw_text(quit, (w - quit_px) / 2.0f, 380, 2.0f, COLOR_GRAY);

    end_drawing();
}

Scene gameover_scene(void)
{
    return (Scene){ gameover_init, gameover_update, gameover_draw, NULL };
}
