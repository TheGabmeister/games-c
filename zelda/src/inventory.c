#include "inventory.h"
#include "game.h"
#include "input.h"
#include "raylib.h"

static const char *item_names[ITEM_COUNT] = {
    [ITEM_NONE]      = "---",
    [ITEM_BOOMERANG] = "BOOMERANG",
    [ITEM_BOW]       = "BOW",
    [ITEM_BOMB]      = "BOMB",
    [ITEM_CANDLE]    = "CANDLE",
};

static const Color item_colors[ITEM_COUNT] = {
    [ITEM_NONE]      = { 40, 40, 40, 255 },
    [ITEM_BOOMERANG] = { 60, 160, 220, 255 },
    [ITEM_BOW]       = { 180, 120, 60, 255 },
    [ITEM_BOMB]      = { 80, 80, 80, 255 },
    [ITEM_CANDLE]    = { 220, 160, 40, 255 },
};

#define GRID_COLS   5
#define GRID_ROWS   1
#define CELL_SIZE   80
#define CELL_PAD    12
#define GRID_X      ((WINDOW_WIDTH - (GRID_COLS * (CELL_SIZE + CELL_PAD) - CELL_PAD)) / 2)
#define GRID_Y      (PLAY_AREA_Y + 200)

static const ItemID grid_items[GRID_ROWS][GRID_COLS] = {
    { ITEM_BOOMERANG, ITEM_BOW, ITEM_BOMB, ITEM_CANDLE, ITEM_NONE },
};

void pause_screen_update(PauseState *state, Inventory *inventory) {
    if (IsKeyPressed(KEY_LEFT) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) {
        state->cursor_x--;
        if (state->cursor_x < 0) state->cursor_x = GRID_COLS - 1;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) {
        state->cursor_x++;
        if (state->cursor_x >= GRID_COLS) state->cursor_x = 0;
    }

    if (input_confirm() || input_attack()) {
        ItemID item = grid_items[state->cursor_y][state->cursor_x];
        if (item == ITEM_NONE) return;

        bool has_item = false;
        if (item == ITEM_BOMB) {
            has_item = inventory->bomb_capacity > 0;
        } else {
            has_item = (inventory->items & (1 << item)) != 0;
        }
        if (has_item) {
            inventory->equipped = item;
        }
    }
}

void pause_screen_draw(const PauseState *state, const Game *game) {
    const Inventory *inventory = &game->player.inventory;
    int sword_tier = inventory->sword_tier;
    DrawRectangle(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT,
                  (Color){ 0, 0, 0, 200 });

    DrawText("INVENTORY", GRID_X, GRID_Y - 60, 30, WHITE);

    const char *sword_names[] = { "NONE", "WOODEN", "WHITE", "MASTER" };
    const char *sname = (sword_tier >= 0 && sword_tier <= 3) ? sword_names[sword_tier] : "?";
    DrawText(TextFormat("SWORD: %s", sname), GRID_X, GRID_Y - 30, 20, YELLOW);

    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            int x = GRID_X + col * (CELL_SIZE + CELL_PAD);
            int y = GRID_Y + row * (CELL_SIZE + CELL_PAD);

            ItemID item = grid_items[row][col];
            bool has_item = false;
            if (item == ITEM_BOMB) {
                has_item = inventory->bomb_capacity > 0;
            } else if (item != ITEM_NONE) {
                has_item = (inventory->items & (1 << item)) != 0;
            }

            Color bg = has_item ? item_colors[item] : (Color){ 30, 30, 30, 255 };
            DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, bg);

            if (item == inventory->equipped && has_item) {
                DrawRectangleLines(x + 2, y + 2, CELL_SIZE - 4, CELL_SIZE - 4, GOLD);
                DrawRectangleLines(x + 3, y + 3, CELL_SIZE - 6, CELL_SIZE - 6, GOLD);
            }

            if (state->cursor_x == col && state->cursor_y == row) {
                DrawRectangleLines(x, y, CELL_SIZE, CELL_SIZE, WHITE);
                DrawRectangleLines(x - 1, y - 1, CELL_SIZE + 2, CELL_SIZE + 2, WHITE);
            }

            if (item != ITEM_NONE) {
                Color text_c = has_item ? WHITE : (Color){ 80, 80, 80, 255 };
                DrawText(item_names[item], x + 4, y + CELL_SIZE - 18, 14, text_c);
            }
        }
    }

    ItemID sel = grid_items[state->cursor_y][state->cursor_x];
    if (sel != ITEM_NONE) {
        DrawText(item_names[sel], GRID_X, GRID_Y + GRID_ROWS * (CELL_SIZE + CELL_PAD) + 20,
                 24, WHITE);
    }

    DrawText("ARROWS/ENTER to equip  |  P to resume",
             GRID_X, GRID_Y + GRID_ROWS * (CELL_SIZE + CELL_PAD) + 60, 16, LIGHTGRAY);

    if (game->in_dungeon) {
        const DungeonState *ds = &game->dungeon;
        int dmap_x = 640, dmap_y = GRID_Y;
        int dmap_w = 320, dmap_h = 160;
        int dcell_w = dmap_w / DUNGEON_MAX_COLS;
        int dcell_h = dmap_h / DUNGEON_MAX_ROWS;

        DrawRectangle(dmap_x, dmap_y, dmap_w, dmap_h, (Color){ 10, 10, 10, 255 });
        DrawRectangleLines(dmap_x, dmap_y, dmap_w, dmap_h, GRAY);
        DrawText("DUNGEON MAP", dmap_x, dmap_y - 24, 18, WHITE);

        for (int ry = 0; ry < DUNGEON_MAX_ROWS; ry++) {
            for (int rx = 0; rx < DUNGEON_MAX_COLS; rx++) {
                uint64_t bit = dungeon_room_bit(rx, ry);
                bool show = false;
                if (ds->has_map && (ds->rooms_exist & bit)) show = true;
                else if (ds->rooms_visited & bit) show = true;
                if (show) {
                    int draw_ry = DUNGEON_MAX_ROWS - 1 - ry;
                    DrawRectangle(dmap_x + rx * dcell_w + 1, dmap_y + draw_ry * dcell_h + 1,
                                  dcell_w - 2, dcell_h - 2, (Color){ 60, 60, 100, 255 });
                }
            }
        }

        if (ds->has_compass && ds->boss_room_x >= 0) {
            int boss_draw_ry = DUNGEON_MAX_ROWS - 1 - ds->boss_room_y;
            DrawRectangle(dmap_x + ds->boss_room_x * dcell_w + 3,
                          dmap_y + boss_draw_ry * dcell_h + 3,
                          dcell_w - 6, dcell_h - 6, (Color){ 200, 40, 40, 255 });
        }

        int cur_draw_ry = DUNGEON_MAX_ROWS - 1 - ds->room_y;
        DrawRectangle(dmap_x + ds->room_x * dcell_w, dmap_y + cur_draw_ry * dcell_h,
                      dcell_w, dcell_h, GREEN);
    }
}
