#include "mushroom.h"

void mushroom_init(MushroomGrid *grid) {
    memset(grid->cells, MUSH_EMPTY, sizeof(grid->cells));
    int placed = 0;
    while (placed < INITIAL_MUSHROOM_COUNT) {
        int col = GetRandomValue(0, GRID_COLS - 1);
        int row = GetRandomValue(1, PLAYER_AREA_TOP_ROW - 1);
        if (!MUSH_IS_PRESENT(grid->cells[row][col])) {
            grid->cells[row][col] = MUSH_MAKE(MUSHROOM_MAX_HP);
            placed++;
        }
    }
}

int mushroom_hit(MushroomGrid *grid, int col, int row) {
    if (col < 0 || col >= GRID_COLS || row < 0 || row >= GRID_ROWS) return 0;
    unsigned char v = grid->cells[row][col];
    if (!MUSH_IS_PRESENT(v)) return 0;

    int hp = MUSH_HP(v);
    hp--;
    if (hp <= 0) {
        grid->cells[row][col] = MUSH_EMPTY;
        return MUSHROOM_DESTROY_PTS;
    }

    bool poisoned = MUSH_IS_POISONED(v);
    grid->cells[row][col] = poisoned ? (unsigned char)(hp + 4) : (unsigned char)hp;
    return 0;
}

void mushroom_poison(MushroomGrid *grid, int col, int row) {
    if (col < 0 || col >= GRID_COLS || row < 0 || row >= GRID_ROWS) return;
    unsigned char v = grid->cells[row][col];
    if (MUSH_IS_PRESENT(v) && !MUSH_IS_POISONED(v)) {
        grid->cells[row][col] = MUSH_POISON(v);
    }
}

bool mushroom_is_blocked(MushroomGrid *grid, int col, int row) {
    if (col < 0 || col >= GRID_COLS || row < 0 || row >= GRID_ROWS) return true;
    return MUSH_IS_PRESENT(grid->cells[row][col]);
}

int mushroom_count_in_player_area(MushroomGrid *grid) {
    int count = 0;
    for (int r = PLAYER_AREA_TOP_ROW; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (MUSH_IS_PRESENT(grid->cells[r][c])) count++;
        }
    }
    return count;
}

void mushroom_draw(MushroomGrid *grid) {
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            unsigned char v = grid->cells[r][c];
            if (!MUSH_IS_PRESENT(v)) continue;

            int hp = MUSH_HP(v);
            bool poisoned = MUSH_IS_POISONED(v);
            float px = col_to_px(c);
            float py = row_to_px(r);

            Color color;
            if (poisoned) {
                color = COLOR_MUSHROOM_POISON;
            } else {
                switch (hp) {
                    case 4: color = COLOR_MUSHROOM_4; break;
                    case 3: color = COLOR_MUSHROOM_3; break;
                    case 2: color = COLOR_MUSHROOM_2; break;
                    default: color = COLOR_MUSHROOM_1; break;
                }
            }

            int shrink = (MUSHROOM_MAX_HP - hp) * 1;
            DrawRectangle((int)px + shrink, (int)py + shrink,
                          CELL_SIZE - shrink * 2, CELL_SIZE - shrink * 2, color);
        }
    }
}
