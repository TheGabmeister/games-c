#include "enemies.h"
#include "mushroom.h"

// --- Spider ---

void spider_try_spawn(Spider *spider, float *timer, int level, float dt) {
    if (spider->active) return;

    float interval = 8.0f - level * 0.5f;
    if (interval < 3.0f) interval = 3.0f;

    *timer += dt;
    if (*timer < interval) return;
    *timer = 0;

    spider->active = true;
    bool from_left = GetRandomValue(0, 1) == 0;
    spider->h_dir = from_left ? DIR_RIGHT : DIR_LEFT;
    spider->x = from_left ? -CELL_SIZE : (float)(GRID_COLS * CELL_SIZE + GRID_OFFSET_X);
    spider->y = row_to_px(GetRandomValue(PLAYER_AREA_TOP_ROW, GRID_ROWS - 1));
    spider->vy = SPIDER_SPEED * (GetRandomValue(0, 1) == 0 ? 1.0f : -1.0f);
    spider->change_timer = (float)GetRandomValue(3, 8) / 10.0f;
}

void spider_update(Spider *spider, MushroomGrid *grid, float dt) {
    if (!spider->active) return;

    float dir = (spider->h_dir == DIR_RIGHT) ? 1.0f : -1.0f;
    spider->x += dir * SPIDER_SPEED * dt;
    spider->y += spider->vy * dt;

    spider->change_timer -= dt;
    if (spider->change_timer <= 0) {
        spider->vy = -spider->vy;
        spider->change_timer = (float)GetRandomValue(3, 8) / 10.0f;
    }

    float min_y = row_to_px(PLAYER_AREA_TOP_ROW);
    float max_y = row_to_px(GRID_ROWS - 1);
    if (spider->y < min_y) { spider->y = min_y; spider->vy = SPIDER_SPEED; }
    if (spider->y > max_y) { spider->y = max_y; spider->vy = -SPIDER_SPEED; }

    int sc = px_to_col(spider->x + CELL_SIZE / 2);
    int sr = px_to_row(spider->y + CELL_SIZE / 2);
    if (sc >= 0 && sc < GRID_COLS && sr >= 0 && sr < GRID_ROWS) {
        if (MUSH_IS_PRESENT(grid->cells[sr][sc])) {
            grid->cells[sr][sc] = MUSH_EMPTY;
        }
    }

    if (spider->x < -CELL_SIZE * 2 || spider->x > WINDOW_WIDTH + CELL_SIZE * 2) {
        spider->active = false;
    }
}

void spider_draw(Spider *spider) {
    if (!spider->active) return;
    float cx = spider->x + CELL_SIZE / 2.0f;
    float cy = spider->y + CELL_SIZE / 2.0f;
    DrawPoly((Vector2){cx, cy}, 8, CELL_SIZE / 2.0f, 0, COLOR_SPIDER);

    DrawLine((int)(cx - 8), (int)(cy - 4), (int)(cx - 12), (int)(cy - 8), COLOR_SPIDER);
    DrawLine((int)(cx + 8), (int)(cy - 4), (int)(cx + 12), (int)(cy - 8), COLOR_SPIDER);
    DrawLine((int)(cx - 7), (int)(cy + 2), (int)(cx - 11), (int)(cy + 6), COLOR_SPIDER);
    DrawLine((int)(cx + 7), (int)(cy + 2), (int)(cx + 11), (int)(cy + 6), COLOR_SPIDER);
}

// --- Flea ---

void flea_try_spawn(Flea *flea, MushroomGrid *grid) {
    if (flea->active) return;

    int count = mushroom_count_in_player_area(grid);
    if (count >= FLEA_SPAWN_THRESHOLD) return;

    flea->active = true;
    flea->col = GetRandomValue(0, GRID_COLS - 1);
    flea->x = col_to_px(flea->col);
    flea->y = row_to_px(0);
    flea->hp = 2;
    flea->fast = false;
    flea->last_row = 0;
}

void flea_update(Flea *flea, MushroomGrid *grid, float dt) {
    if (!flea->active) return;

    float speed = flea->fast ? FLEA_FAST_SPEED : FLEA_SPEED;
    flea->y += speed * dt;

    int cur_row = px_to_row(flea->y + CELL_SIZE / 2);
    if (cur_row != flea->last_row && cur_row >= 0 && cur_row < GRID_ROWS) {
        if (GetRandomValue(0, 2) == 0 && !MUSH_IS_PRESENT(grid->cells[cur_row][flea->col])) {
            grid->cells[cur_row][flea->col] = MUSH_MAKE(MUSHROOM_MAX_HP);
        }
        flea->last_row = cur_row;
    }

    if (flea->y > WINDOW_HEIGHT) {
        flea->active = false;
    }
}

void flea_draw(Flea *flea) {
    if (!flea->active) return;
    DrawRectangle((int)flea->x + 4, (int)flea->y, CELL_SIZE - 8, CELL_SIZE, COLOR_FLEA);
    DrawRectangle((int)flea->x + 2, (int)flea->y + 4, CELL_SIZE - 4, CELL_SIZE - 8, COLOR_FLEA);
}

// --- Scorpion ---

void scorpion_try_spawn(Scorpion *scorpion, float *timer, int level, float dt) {
    if (scorpion->active) return;

    float interval = 15.0f - level * 1.0f;
    if (interval < 5.0f) interval = 5.0f;

    *timer += dt;
    if (*timer < interval) return;
    *timer = 0;

    scorpion->active = true;
    bool from_left = GetRandomValue(0, 1) == 0;
    scorpion->h_dir = from_left ? DIR_RIGHT : DIR_LEFT;
    scorpion->x = from_left ? -(float)CELL_SIZE : (float)(GRID_COLS * CELL_SIZE + GRID_OFFSET_X);
    scorpion->row = GetRandomValue(3, PLAYER_AREA_TOP_ROW - 3);
    scorpion->y = row_to_px(scorpion->row);
}

void scorpion_update(Scorpion *scorpion, MushroomGrid *grid, float dt) {
    if (!scorpion->active) return;

    float dir = (scorpion->h_dir == DIR_RIGHT) ? 1.0f : -1.0f;
    scorpion->x += dir * SCORPION_SPEED * dt;

    int sc = px_to_col(scorpion->x + CELL_SIZE / 2);
    if (sc >= 0 && sc < GRID_COLS) {
        mushroom_poison(grid, sc, scorpion->row);
    }

    if (scorpion->x < -CELL_SIZE * 2 || scorpion->x > WINDOW_WIDTH + CELL_SIZE * 2) {
        scorpion->active = false;
    }
}

void scorpion_draw(Scorpion *scorpion) {
    if (!scorpion->active) return;
    DrawRectangle((int)scorpion->x + 2, (int)scorpion->y + 4, CELL_SIZE - 4, CELL_SIZE - 8, COLOR_SCORPION);

    float tail_x = (scorpion->h_dir == DIR_RIGHT)
        ? scorpion->x - 4
        : scorpion->x + CELL_SIZE + 4;
    DrawCircle((int)tail_x, (int)(scorpion->y + 4), 3, COLOR_SCORPION);
}
