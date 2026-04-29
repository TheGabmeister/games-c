#include "debug.h"
#include "game.h"
#include "enemy/enemy.h"
#include "projectile.h"
#include "raylib.h"

bool debug_enabled = false;

static const char *enemy_type_names[] = { "slime", "bat", "snake", "rock_spit", "spear_thr", "dragon" };
static const char *enemy_state_names[] = { "idle", "move", "charge", "dead" };

void debug_toggle(void) {
    debug_enabled = !debug_enabled;
}

void debug_draw_overlay(void) {
    if (!debug_enabled) return;

    DrawRectangle(0, 0, 220, 20, (Color){ 0, 0, 0, 180 });
    DrawText("[DEBUG] F3 to toggle", 4, 2, 16, YELLOW);
}

void debug_draw_game(const Game *game) {
    if (!debug_enabled) return;

    int y = 160;
    int alive = 0;
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].active) alive++;
    }
    DrawText(TextFormat("enemies: %d/%d", alive, game->enemy_count), 32, y, 20, YELLOW);
    y += 24;
    DrawText(TextFormat("player state: %d  hp: %d/%d",
             game->player.state, game->player.health, game->player.max_health),
             32, y, 20, YELLOW);
    y += 24;

    for (int i = 0; i < game->enemy_count; i++) {
        const Enemy *e = &game->enemies[i];
        if (!e->active) continue;

        DrawRectangleLines((int)e->pos.x, (int)e->pos.y, TILE_SIZE, TILE_SIZE, YELLOW);

        const char *tname = (e->type < ENEMY_TYPE_COUNT) ? enemy_type_names[e->type] : "?";
        const char *sname = (e->state <= ESTATE_DEAD) ? enemy_state_names[e->state] : "?";
        DrawText(TextFormat("%s %s (%.0f,%.0f)", tname, sname, e->pos.x, e->pos.y),
                 32, y, 16, YELLOW);
        y += 18;
    }

    int proj_alive = 0;
    for (int i = 0; i < game->projectile_count; i++) {
        if (game->projectiles[i].active) proj_alive++;
    }
    if (proj_alive > 0) {
        DrawText(TextFormat("projectiles: %d", proj_alive), 32, y, 20, YELLOW);
        y += 24;
    }
}
