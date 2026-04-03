#include "game.h"
#include "platform.h"
#include "input.h"
#include "draw.h"
#include "scene.h"
#include <stdio.h>

// Forward declarations
static Scene title_scene(void);
static Scene play_scene(void);

// ---------------------------------------------------------------------------
// Title Scene
// ---------------------------------------------------------------------------

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
    int title_px = 16 * 8 * 3;           // chars * char_width * scale
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

static Scene title_scene(void)
{
    return (Scene){ title_init, title_update, title_draw, NULL };
}

// ---------------------------------------------------------------------------
// Play Scene
// ---------------------------------------------------------------------------

typedef struct {
    rectangle rect;
    color col;
    float speed;
} Player;

static Player player;

static void play_init(void)
{
    float w = (float)get_window_width();
    float h = (float)get_window_height();

    player = (Player){
        .rect  = { w / 2.0f - 20, h / 2.0f - 20, 40, 40 },
        .col   = COLOR_GREEN,
        .speed = 250.0f,
    };
}

static void play_update(float dt)
{
    if (action_pressed(ACTION_CANCEL)) {
        scene_set(title_scene());
        return;
    }

    // Movement
    if (action_down(ACTION_UP))
        player.rect.y -= player.speed * dt;
    if (action_down(ACTION_DOWN))
        player.rect.y += player.speed * dt;
    if (action_down(ACTION_LEFT))
        player.rect.x -= player.speed * dt;
    if (action_down(ACTION_RIGHT))
        player.rect.x += player.speed * dt;

    // Clamp to window
    float w = (float)get_window_width();
    float h = (float)get_window_height();
    if (player.rect.x < 0) player.rect.x = 0;
    if (player.rect.y < 0) player.rect.y = 0;
    if (player.rect.x + player.rect.w > w) player.rect.x = w - player.rect.w;
    if (player.rect.y + player.rect.h > h) player.rect.y = h - player.rect.h;
}

static void play_draw(void)
{
    clear_background((color){ 20, 20, 30, 255 });
    begin_drawing();

    // Player
    draw_rect_filled(player.rect, player.col);
    draw_rect(player.rect, COLOR_WHITE);

    // HUD
    char buf[64];
    snprintf(buf, sizeof(buf), "FPS: %d", get_fps());
    draw_text(buf, 10, 10, 2.0f, COLOR_YELLOW);
    draw_text("WASD / Arrows: Move  |  ESC: Menu", 10, 40, 1.0f, COLOR_GRAY);

    end_drawing();
}

static void play_cleanup(void) { }

static Scene play_scene(void)
{
    return (Scene){ play_init, play_update, play_draw, play_cleanup };
}

// ---------------------------------------------------------------------------
// Game entry points
// ---------------------------------------------------------------------------

void game_init(void)
{
    scene_set(title_scene());
}

void game_shutdown(void)
{
    scene_cleanup();
}
