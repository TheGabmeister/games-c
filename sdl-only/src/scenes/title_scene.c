#include "title_scene.h"
#include "play_scene.h"
#include "../platform.h"
#include "../input.h"
#include "../draw.h"

static void title_init(void) { }

static void title_update(float dt)
{
    (void)dt;

    if (action_pressed(ACTION_CONFIRM))
        scene_set(play_scene());

    if (action_pressed(ACTION_CANCEL))
        request_close();
}

static void title_draw(void)
{
    int w = get_window_width();

    clear_background(COLOR_DARKGRAY);
    begin_drawing();

    // Title
    const char *title = "2D GAME TEMPLATE";
    int title_px = 16 * 8 * 3;
    draw_text(title, (w - title_px) / 2.0f, 180, 3.0f, COLOR_WHITE);

    // Subtitle
    const char *sub = "Press ENTER to Play";
    int sub_px = 19 * 8 * 2;
    draw_text(sub, (w - sub_px) / 2.0f, 300, 2.0f, COLOR_YELLOW);

    // Quit hint
    const char *quit = "ESC to Quit";
    int quit_px = 11 * 8 * 2;
    draw_text(quit, (w - quit_px) / 2.0f, 340, 2.0f, COLOR_GRAY);

    end_drawing();
}

Scene title_scene(void)
{
    return (Scene){ title_init, title_update, title_draw, NULL };
}
