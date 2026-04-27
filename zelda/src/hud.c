#include "hud.h"
#include "game_config.h"

void hud_draw(const Player *player, int screen_x, int screen_y) {
    DrawRectangle(0, 0, WINDOW_WIDTH, HUD_HEIGHT, BLACK);
    DrawRectangle(0, HUD_HEIGHT, WINDOW_WIDTH, HUD_DIVIDER, (Color){ 80, 80, 80, 255 });

    DrawText(TextFormat("RUPEES: %d", player->inventory.rupees), 32, 32, 20, WHITE);
    DrawText(TextFormat("KEYS: %d", player->inventory.keys), 32, 64, 20, WHITE);
    DrawText(TextFormat("BOMBS: %d/%d", player->inventory.bombs, player->inventory.bomb_capacity), 32, 96, 20, WHITE);

    int hearts = player->max_health / 2;
    int filled = player->health;
    for (int i = 0; i < hearts; i++) {
        int x = 560 + i * 36;
        int y = 32;
        int half_index = i * 2;
        Color c;
        if (filled > half_index + 1)     c = RED;
        else if (filled > half_index)    c = MAROON;
        else                             c = DARKGRAY;
        DrawRectangle(x, y, 28, 28, c);
    }

    int map_x = 800, map_y = 48;
    int map_w = 160, map_h = 128;
    DrawRectangle(map_x, map_y, map_w, map_h, (Color){ 20, 20, 20, 255 });
    DrawRectangleLines(map_x, map_y, map_w, map_h, GRAY);

    int cell_w = map_w / OVERWORLD_COLS;
    int cell_h = map_h / OVERWORLD_ROWS;
    int cx = map_x + screen_x * cell_w;
    int cy = map_y + screen_y * cell_h;
    DrawRectangle(cx, cy, cell_w, cell_h, GREEN);
}
