#include "hud.h"
#include "game_config.h"

void hud_draw(const Player *player) {
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

    DrawRectangleLines(800, 48, 160, 128, GRAY);
    DrawText("MAP", 860, 100, 20, GRAY);
}
