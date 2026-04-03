#include "demo_scene.h"
#include "config.h"
#include "draw.h"
#include "game.h"
#include "input.h"
#include "types.h"
#include <SDL3/SDL.h>

/* ---- world data ---- */

#define PLAYER_SIZE     32.0f
#define PLAYER_SPEED    200.0f
#define WORLD_W         2000.0f
#define WORLD_H         1500.0f
#define NUM_WALLS       8
#define NUM_COINS       12

typedef struct {
    Vec2 pos;
    bool collected;
} Coin;

static Vec2 player_pos;
static int  score;

static Rect walls[NUM_WALLS];
static Coin coins[NUM_COINS];

static float fps_timer;
static int   fps_frames;
static int   fps_display;

/* ---- helpers ---- */

static Rect player_rect(void)
{
    return rect_make(
        player_pos.x - PLAYER_SIZE * 0.5f,
        player_pos.y - PLAYER_SIZE * 0.5f,
        PLAYER_SIZE, PLAYER_SIZE
    );
}

static bool collides_wall(Rect r)
{
    for (int i = 0; i < NUM_WALLS; i++) {
        if (rect_overlaps(r, walls[i])) return true;
    }
    return false;
}

/* ---- scene callbacks ---- */

static bool demo_init(void)
{
    player_pos = vec2(WORLD_W * 0.5f, WORLD_H * 0.5f);
    score = 0;
    fps_timer = 0;
    fps_frames = 0;
    fps_display = 0;

    /* Border walls */
    walls[0] = rect_make(0, 0, WORLD_W, 16);                  /* top */
    walls[1] = rect_make(0, WORLD_H - 16, WORLD_W, 16);       /* bottom */
    walls[2] = rect_make(0, 0, 16, WORLD_H);                  /* left */
    walls[3] = rect_make(WORLD_W - 16, 0, 16, WORLD_H);       /* right */

    /* Interior walls */
    walls[4] = rect_make(300, 300, 400, 24);
    walls[5] = rect_make(800, 200, 24, 500);
    walls[6] = rect_make(400, 800, 600, 24);
    walls[7] = rect_make(1200, 400, 24, 600);

    /* Scatter coins */
    float coin_positions[][2] = {
        {200, 200}, {500, 150}, {700, 400}, {350, 600},
        {900, 300}, {1100, 200}, {1400, 500}, {1600, 700},
        {600, 900}, {1000, 800}, {1300, 1000}, {1500, 1200},
    };
    for (int i = 0; i < NUM_COINS; i++) {
        coins[i].pos = vec2(coin_positions[i][0], coin_positions[i][1]);
        coins[i].collected = false;
    }

    return true;
}

static void demo_shutdown(void)
{
    /* nothing to free */
}

static void demo_update(float dt)
{
    if (input_key_pressed(SDL_SCANCODE_ESCAPE)) {
        game_quit();
        return;
    }

    /* Movement */
    Vec2 dir = {0, 0};
    if (input_key_down(SDL_SCANCODE_W) || input_key_down(SDL_SCANCODE_UP))    dir.y -= 1;
    if (input_key_down(SDL_SCANCODE_S) || input_key_down(SDL_SCANCODE_DOWN))  dir.y += 1;
    if (input_key_down(SDL_SCANCODE_A) || input_key_down(SDL_SCANCODE_LEFT))  dir.x -= 1;
    if (input_key_down(SDL_SCANCODE_D) || input_key_down(SDL_SCANCODE_RIGHT)) dir.x += 1;
    dir = vec2_normalize(dir);

    Vec2 velocity = vec2_scale(dir, PLAYER_SPEED * dt);

    /* Move X then Y separately for wall sliding */
    Vec2 new_pos = player_pos;

    new_pos.x += velocity.x;
    Rect test = rect_make(new_pos.x - PLAYER_SIZE * 0.5f,
                           new_pos.y - PLAYER_SIZE * 0.5f,
                           PLAYER_SIZE, PLAYER_SIZE);
    if (collides_wall(test)) {
        new_pos.x = player_pos.x;
    }

    new_pos.y += velocity.y;
    test = rect_make(new_pos.x - PLAYER_SIZE * 0.5f,
                     new_pos.y - PLAYER_SIZE * 0.5f,
                     PLAYER_SIZE, PLAYER_SIZE);
    if (collides_wall(test)) {
        new_pos.y = player_pos.y;
    }

    player_pos = new_pos;

    /* Coin pickup */
    Rect pr = player_rect();
    for (int i = 0; i < NUM_COINS; i++) {
        if (coins[i].collected) continue;
        Rect cr = rect_make(coins[i].pos.x - 8, coins[i].pos.y - 8, 16, 16);
        if (rect_overlaps(pr, cr)) {
            coins[i].collected = true;
            score++;
        }
    }

    /* FPS counter */
    fps_timer += dt;
    fps_frames++;
    if (fps_timer >= 1.0f) {
        fps_display = fps_frames;
        fps_frames = 0;
        fps_timer -= 1.0f;
    }
}

static void demo_render(void)
{
    draw_clear(((Color){20, 20, 30, 255}));

    /* Camera follows player */
    Camera cam = {.pos = player_pos, .zoom = 1.0f};
    draw_set_camera(cam);

    /* Grid (visual reference) */
    Color grid_color = {40, 40, 55, 255};
    float grid_step = 100.0f;
    for (float x = 0; x <= WORLD_W; x += grid_step) {
        draw_line(vec2(x, 0), vec2(x, WORLD_H), grid_color);
    }
    for (float y = 0; y <= WORLD_H; y += grid_step) {
        draw_line(vec2(0, y), vec2(WORLD_W, y), grid_color);
    }

    /* Walls */
    for (int i = 0; i < NUM_WALLS; i++) {
        draw_rect_fill(walls[i], COLOR_GRAY);
    }

    /* Coins */
    for (int i = 0; i < NUM_COINS; i++) {
        if (coins[i].collected) continue;
        draw_circle_fill(coins[i].pos, 8, COLOR_YELLOW);
        draw_circle(coins[i].pos, 8, ((Color){200, 180, 30, 255}));
    }

    /* Player */
    draw_rect_fill(player_rect(), COLOR_BLUE);
    draw_rect(player_rect(), ((Color){80, 140, 255, 255}));

    /* ---- HUD (screen-space) ---- */
    draw_text(vec2(10, 10), 2.0f, COLOR_WHITE, "Score: %d / %d", score, NUM_COINS);
    draw_text(vec2(10, 36), 1.0f, COLOR_GRAY, "FPS: %d", fps_display);
    draw_text(vec2(10, WINDOW_HEIGHT - 20), 1.0f, COLOR_GRAY,
              "WASD/Arrows: Move   ESC: Quit");
}

const Scene demo_scene = {
    .init     = demo_init,
    .shutdown = demo_shutdown,
    .update   = demo_update,
    .render   = demo_render,
};
