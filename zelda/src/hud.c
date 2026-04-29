#include "hud.h"
#include "game.h"

void hud_draw(const Game *game) {
    const Player *player = &game->player;

    DrawRectangle(0, 0, WINDOW_WIDTH, HUD_HEIGHT, BLACK);
    DrawRectangle(0, HUD_HEIGHT, WINDOW_WIDTH, HUD_DIVIDER, (Color){ 80, 80, 80, 255 });

    DrawText(TextFormat("RUPEES: %d", player->inventory.rupees), 32, 32, 20, WHITE);
    DrawText(TextFormat("KEYS: %d", player->inventory.keys), 32, 64, 20, WHITE);
    DrawText(TextFormat("BOMBS: %d/%d", player->inventory.bombs, player->inventory.bomb_capacity), 32, 96, 20, WHITE);
    DrawText(TextFormat("ARROWS: %d/%d", player->inventory.arrows, player->inventory.arrow_capacity), 32, 128, 20, WHITE);

    const char *equip_name = "---";
    switch (player->inventory.equipped) {
        case ITEM_BOOMERANG: equip_name = "BOOMERANG"; break;
        case ITEM_BOW:       equip_name = "BOW"; break;
        case ITEM_BOMB:      equip_name = "BOMB"; break;
        case ITEM_CANDLE:    equip_name = "CANDLE"; break;
        default: break;
    }
    DrawText(TextFormat("B: %s", equip_name), 32, 160, 20, YELLOW);

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

    if (game->in_dungeon) {
        int cell_w = map_w / DUNGEON_MAX_COLS;
        int cell_h = map_h / DUNGEON_MAX_ROWS;
        const DungeonState *ds = &game->dungeon;

        for (int ry = 0; ry < DUNGEON_MAX_ROWS; ry++) {
            for (int rx = 0; rx < DUNGEON_MAX_COLS; rx++) {
                uint64_t bit = dungeon_room_bit(rx, ry);
                bool show = false;
                if (ds->has_map && (ds->rooms_exist & bit)) show = true;
                else if (ds->rooms_visited & bit) show = true;

                if (show) {
                    int draw_ry = DUNGEON_MAX_ROWS - 1 - ry;
                    DrawRectangle(map_x + rx * cell_w + 1, map_y + draw_ry * cell_h + 1,
                                  cell_w - 2, cell_h - 2, (Color){ 60, 60, 80, 255 });
                }
            }
        }

        if (ds->has_compass && ds->boss_room_x >= 0) {
            int boss_draw_ry = DUNGEON_MAX_ROWS - 1 - ds->boss_room_y;
            DrawRectangle(map_x + ds->boss_room_x * cell_w + 2,
                          map_y + boss_draw_ry * cell_h + 2,
                          cell_w - 4, cell_h - 4, (Color){ 200, 40, 40, 255 });
        }

        int cur_draw_ry = DUNGEON_MAX_ROWS - 1 - ds->room_y;
        DrawRectangle(map_x + ds->room_x * cell_w, map_y + cur_draw_ry * cell_h,
                      cell_w, cell_h, GREEN);
    } else {
        int cell_w = map_w / OVERWORLD_COLS;
        int cell_h = map_h / OVERWORLD_ROWS;
        int cx = map_x + game->screen_x * cell_w;
        int cy = map_y + game->screen_y * cell_h;
        DrawRectangle(cx, cy, cell_w, cell_h, GREEN);
    }
}
