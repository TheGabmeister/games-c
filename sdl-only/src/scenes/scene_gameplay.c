#include "scene_gameplay.h"
#include "scene_title.h"
#include "scene_gameover.h"
#include "../platform.h"
#include "../input.h"
#include "../draw.h"
#include "../collision.h"
#include "../collectible.h"
#include "../enemy.h"
#include "../gamestate.h"
#include <math.h>
#include <stdio.h>

typedef struct {
    rectangle rect;
    CollisionFilter filter;
    color col;
    float speed;
} Player;

static Player player;

static void gameplay_init(void)
{
    float w = (float)get_window_width();
    float h = (float)get_window_height();

    player = (Player){
        .rect   = { w / 2.0f - 20, h / 2.0f - 20, 40, 40 },
        .filter = { COLLISION_LAYER_PLAYER, COLLISION_LAYER_ENEMY | COLLISION_LAYER_COLLECTIBLE },
        .col    = COLOR_GREEN,
        .speed  = 250.0f,
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

static void gameplay_update(float dt)
{
    if (action_pressed(ACTION_CANCEL)) {
        scene_set(scene_title());
        return;
    }

    // Movement (normalized so diagonals aren't faster)
    float mx = 0.0f, my = 0.0f;
    if (action_down(ACTION_UP))    my -= 1.0f;
    if (action_down(ACTION_DOWN))  my += 1.0f;
    if (action_down(ACTION_LEFT))  mx -= 1.0f;
    if (action_down(ACTION_RIGHT)) mx += 1.0f;

    float move_len = sqrtf(mx * mx + my * my);
    if (move_len > 0.0f) {
        float inv_len = 1.0f / move_len;
        player.rect.x += mx * inv_len * player.speed * dt;
        player.rect.y += my * inv_len * player.speed * dt;
    }

    // Clamp to window
    float w = (float)get_window_width();
    float h = (float)get_window_height();
    if (player.rect.x < 0) player.rect.x = 0;
    if (player.rect.y < 0) player.rect.y = 0;
    if (player.rect.x + player.rect.w > w) player.rect.x = w - player.rect.w;
    if (player.rect.y + player.rect.h > h) player.rect.y = h - player.rect.h;

    collectibles_update(player.rect, player.filter, gamestate_get());
    enemies_update(dt, player.rect, player.filter, gamestate_get());

    if (gamestate_get()->health <= 0) {
        scene_set(scene_gameover());
        return;
    }
}

static void gameplay_draw(void)
{
    begin_drawing((color){ 20, 20, 30, 255 });

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

static void gameplay_cleanup(void) { }

Scene scene_gameplay(void)
{
    return (Scene){ gameplay_init, gameplay_update, gameplay_draw, gameplay_cleanup };
}
