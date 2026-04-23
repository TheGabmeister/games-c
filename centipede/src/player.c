#include "player.h"

void player_init(Player *p) {
    p->col = PLAYER_START_COL;
    p->row = PLAYER_START_ROW;
    p->x = col_to_px(p->col);
    p->y = row_to_px(p->row);
    p->alive = true;
}

void player_update(Player *p, Dart *dart, MushroomGrid *grid, float dt) {
    if (!p->alive) return;

    float speed = PLAYER_SPEED * dt;
    float nx = p->x;
    float ny = p->y;

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  nx -= speed;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) nx += speed;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    ny -= speed;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))   ny += speed;

    float min_x = col_to_px(0);
    float max_x = col_to_px(GRID_COLS - 1);
    float min_y = row_to_px(PLAYER_AREA_TOP_ROW);
    float max_y = row_to_px(GRID_ROWS - 1);

    if (nx < min_x) nx = min_x;
    if (nx > max_x) nx = max_x;
    if (ny < min_y) ny = min_y;
    if (ny > max_y) ny = max_y;

    int new_col = px_to_col(nx + CELL_SIZE / 2);
    int new_row = px_to_row(ny + CELL_SIZE / 2);

    if (new_col < 0) new_col = 0;
    if (new_col >= GRID_COLS) new_col = GRID_COLS - 1;
    if (new_row < PLAYER_AREA_TOP_ROW) new_row = PLAYER_AREA_TOP_ROW;
    if (new_row >= GRID_ROWS) new_row = GRID_ROWS - 1;

    if (mushroom_is_blocked(grid, new_col, new_row)) {
        if (new_col != p->col && new_row != p->row) {
            // diagonal blocked, keep old position
        } else if (new_col != p->col) {
            nx = p->x;
        } else if (new_row != p->row) {
            ny = p->y;
        } else {
            nx = p->x;
            ny = p->y;
        }
    }

    p->x = nx;
    p->y = ny;
    p->col = px_to_col(p->x + CELL_SIZE / 2);
    p->row = px_to_row(p->y + CELL_SIZE / 2);

    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_Z)) && !dart->active) {
        dart->x = p->x + CELL_SIZE / 2 - DART_WIDTH / 2;
        dart->y = p->y;
        dart->active = true;
    }
}

void dart_update(Dart *dart, float dt) {
    if (!dart->active) return;
    dart->y -= DART_SPEED * dt;
    if (dart->y + DART_HEIGHT < GRID_OFFSET_Y) {
        dart->active = false;
    }
}

void player_draw(Player *p) {
    if (!p->alive) return;
    float cx = p->x + CELL_SIZE / 2;
    float bot = p->y + CELL_SIZE;
    Vector2 v1 = { cx, p->y + 2 };
    Vector2 v2 = { p->x + 3, bot };
    Vector2 v3 = { p->x + CELL_SIZE - 3, bot };
    DrawTriangle(v1, v2, v3, COLOR_PLAYER);
}

void dart_draw(Dart *dart) {
    if (!dart->active) return;
    DrawRectangle((int)dart->x, (int)dart->y, DART_WIDTH, DART_HEIGHT, COLOR_DART);
}
