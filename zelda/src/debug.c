#include "debug.h"
#include "game.h"
#include "raylib.h"

bool debug_enabled = false;

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
    DrawText(TextFormat("player state: %d", game->player.state), 32, y, 20, YELLOW);
    y += 24;
    DrawText(TextFormat("pos: %.0f, %.0f", game->player.pos.x, game->player.pos.y), 32, y, 20, YELLOW);
    y += 24;
    DrawText(TextFormat("hp: %d/%d", game->player.health, game->player.max_health), 32, y, 20, YELLOW);
}
