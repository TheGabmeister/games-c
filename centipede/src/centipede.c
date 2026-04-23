#include "centipede.h"

static void centipede_step(Segment *seg, MushroomGrid *grid) {
    if (seg->diving) {
        seg->row++;
        if (seg->row >= GRID_ROWS) {
            seg->row = GRID_ROWS - 1;
            seg->diving = false;
        }
        seg->x = col_to_px(seg->col);
        seg->y = row_to_px(seg->row);
        return;
    }

    int next_col = seg->col + (seg->h_dir == DIR_RIGHT ? 1 : -1);
    bool blocked = (next_col < 0 || next_col >= GRID_COLS);
    if (!blocked && seg->row >= 0 && seg->row < GRID_ROWS) {
        blocked = MUSH_IS_PRESENT(grid->cells[seg->row][next_col]);
    }

    if (blocked) {
        seg->h_dir = (seg->h_dir == DIR_RIGHT) ? DIR_LEFT : DIR_RIGHT;
        seg->row++;
        if (seg->row >= GRID_ROWS) {
            seg->row = 0;
        }
        if (seg->row >= 0 && seg->row < GRID_ROWS &&
            seg->col >= 0 && seg->col < GRID_COLS &&
            MUSH_IS_PRESENT(grid->cells[seg->row][seg->col]) &&
            MUSH_IS_POISONED(grid->cells[seg->row][seg->col])) {
            seg->diving = true;
        }
    } else {
        if (seg->row >= 0 && seg->row < GRID_ROWS &&
            MUSH_IS_PRESENT(grid->cells[seg->row][next_col]) &&
            MUSH_IS_POISONED(grid->cells[seg->row][next_col])) {
            seg->diving = true;
        }
        seg->col = next_col;
    }

    seg->x = col_to_px(seg->col);
    seg->y = row_to_px(seg->row);
}

void centipede_init(Segment segments[], int level) {
    for (int i = 0; i < MAX_SEGMENTS; i++) {
        segments[i].active = false;
        segments[i].next = -1;
        segments[i].prev = -1;
        segments[i].move_timer = 0;
        segments[i].diving = false;
    }

    int count = INITIAL_SEGMENTS;
    int extra_heads = (level - 1) / 3;
    if (extra_heads > count - 1) extra_heads = count - 1;

    int chain_len = count - extra_heads;

    segments[0].active = true;
    segments[0].is_head = true;
    segments[0].col = 0;
    segments[0].row = 0;
    segments[0].h_dir = DIR_RIGHT;
    segments[0].x = col_to_px(0);
    segments[0].y = row_to_px(0);
    segments[0].next = (chain_len > 1) ? 1 : -1;
    segments[0].prev = -1;
    segments[0].move_timer = 0;
    segments[0].diving = false;

    for (int i = 1; i < chain_len; i++) {
        segments[i].active = true;
        segments[i].is_head = false;
        segments[i].col = -i;
        segments[i].row = 0;
        segments[i].h_dir = DIR_RIGHT;
        segments[i].x = col_to_px(-i);
        segments[i].y = row_to_px(0);
        segments[i].prev = i - 1;
        segments[i].next = (i < chain_len - 1) ? i + 1 : -1;
        segments[i].move_timer = 0;
        segments[i].diving = false;
    }

    for (int i = 0; i < extra_heads; i++) {
        int idx = chain_len + i;
        segments[idx].active = true;
        segments[idx].is_head = true;
        segments[idx].col = GetRandomValue(0, GRID_COLS - 1);
        segments[idx].row = 0;
        segments[idx].h_dir = (GetRandomValue(0, 1) == 0) ? DIR_LEFT : DIR_RIGHT;
        segments[idx].x = col_to_px(segments[idx].col);
        segments[idx].y = row_to_px(0);
        segments[idx].prev = -1;
        segments[idx].next = -1;
        segments[idx].move_timer = (float)i * 0.2f;
        segments[idx].diving = false;
    }
}

void centipede_update(Segment segments[], MushroomGrid *grid, float speed, float dt) {
    float step_interval = CELL_SIZE / speed;

    for (int i = 0; i < MAX_SEGMENTS; i++) {
        if (!segments[i].active) continue;

        segments[i].move_timer += dt;
        while (segments[i].move_timer >= step_interval) {
            segments[i].move_timer -= step_interval;
            centipede_step(&segments[i], grid);
        }
    }
}

void centipede_draw(Segment segments[]) {
    for (int i = 0; i < MAX_SEGMENTS; i++) {
        if (!segments[i].active) continue;
        if (segments[i].col < 0 || segments[i].col >= GRID_COLS) continue;

        float cx = segments[i].x + CELL_SIZE / 2.0f;
        float cy = segments[i].y + CELL_SIZE / 2.0f;
        float radius = CELL_SIZE / 2.0f - 1;

        Color color = segments[i].is_head ? COLOR_HEAD : COLOR_BODY;
        DrawCircle((int)cx, (int)cy, radius, color);

        if (segments[i].is_head) {
            float eye_off = 3.0f;
            float eye_r = 2.0f;
            DrawCircle((int)(cx - eye_off), (int)(cy - eye_off), eye_r, WHITE);
            DrawCircle((int)(cx + eye_off), (int)(cy - eye_off), eye_r, WHITE);
        }
    }
}

int centipede_active_count(Segment segments[]) {
    int count = 0;
    for (int i = 0; i < MAX_SEGMENTS; i++) {
        if (segments[i].active) count++;
    }
    return count;
}

bool centipede_hit_segment(Segment segments[], int idx, MushroomGrid *grid) {
    if (idx < 0 || idx >= MAX_SEGMENTS || !segments[idx].active) return false;

    bool was_head = segments[idx].is_head;

    if (segments[idx].col >= 0 && segments[idx].col < GRID_COLS &&
        segments[idx].row >= 0 && segments[idx].row < GRID_ROWS &&
        !MUSH_IS_PRESENT(grid->cells[segments[idx].row][segments[idx].col])) {
        grid->cells[segments[idx].row][segments[idx].col] = MUSH_MAKE(MUSHROOM_MAX_HP);
    }

    if (segments[idx].next != -1) {
        int next_idx = segments[idx].next;
        segments[next_idx].is_head = true;
        segments[next_idx].prev = -1;
        segments[next_idx].h_dir = (segments[next_idx].h_dir == DIR_RIGHT) ? DIR_LEFT : DIR_RIGHT;
    }

    if (segments[idx].prev != -1) {
        segments[segments[idx].prev].next = -1;
    }

    segments[idx].active = false;
    segments[idx].next = -1;
    segments[idx].prev = -1;

    return was_head;
}
