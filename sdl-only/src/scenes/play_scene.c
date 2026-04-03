#include "play_scene.h"
#include "title_scene.h"
#include "gameover_scene.h"
#include "../platform.h"
#include "../input.h"
#include "../draw.h"
#include "../collectible.h"
#include "../enemy.h"
#include "../gamestate.h"
#include <stdio.h>

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

    gamestate_reset();

    collectibles_init();
    collectible_spawn(COLLECTIBLE_COIN,   100, 150);
    collectible_spawn(COLLECTIBLE_COIN,   300, 400);
    collectible_spawn(COLLECTIBLE_COIN,   600, 200);
    collectible_spawn(COLLECTIBLE_HEALTH, 500, 450);
    collectible_spawn(COLLECTIBLE_STAR,   400, 100);

    enemies_init();
    enemy_spawn(ENEMY_WANDERER, 200, 100);
    enemy_spawn(ENEMY_WANDERER, 600, 400);
    enemy_spawn(ENEMY_CHASER,   700, 300);
    enemy_spawn(ENEMY_BOUNCER,  100, 500);
    enemy_spawn(ENEMY_BOUNCER,  400, 200);
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

    collectibles_update(player.rect, gamestate_get());
    enemies_update(dt, player.rect, gamestate_get());

    if (gamestate_get()->health <= 0) {
        scene_set(gameover_scene());
        return;
    }
}

static void play_draw(void)
{
    clear_background((color){ 20, 20, 30, 255 });
    begin_drawing();

    // Collectibles & enemies
    collectibles_draw();
    enemies_draw();

    // Player
    draw_rect_filled(player.rect, player.col);
    draw_rect(player.rect, COLOR_WHITE);

    // HUD
    char buf[64];
    GameState *gs = gamestate_get();
    snprintf(buf, sizeof(buf), "FPS: %d  |  Score: %d  |  HP: %d", get_fps(), gs->score, gs->health);
    draw_text(buf, 10, 10, 2.0f, COLOR_YELLOW);
    draw_text("WASD / Arrows: Move  |  ESC: Menu", 10, 40, 1.0f, COLOR_GRAY);

    end_drawing();
}

static void play_cleanup(void) { }

Scene play_scene(void)
{
    return (Scene){ play_init, play_update, play_draw, play_cleanup };
}
