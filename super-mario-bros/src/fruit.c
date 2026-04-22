#include "fruit.h"
#include <stdio.h>

static const int fruit_pts[FRUIT_COUNT] = { 100, 300, 500, 700, 1000, 2000, 3000, 5000 };

static const Color fruit_colors[FRUIT_COUNT] = {
    {255, 0, 0, 255},      {255, 50, 50, 255},    {255, 165, 0, 255},
    {0, 200, 0, 255},      {100, 200, 50, 255},   {255, 255, 0, 255},
    {255, 215, 0, 255},    {150, 200, 255, 255},
};

int fruit_type_for_level(int level) {
    if (level <= 1) return 0;
    if (level <= 2) return 1;
    if (level <= 4) return 2;
    if (level <= 6) return 3;
    if (level <= 8) return 4;
    if (level <= 10) return 5;
    if (level <= 12) return 6;
    return 7;
}

int fruit_points_for_type(int type) {
    return fruit_pts[type];
}

void fruit_spawn(Fruit *f, int level) {
    f->active = true;
    f->fruit_type = fruit_type_for_level(level);
    f->points = fruit_pts[f->fruit_type];
    f->timer = FRUIT_DURATION;
    f->score_display = false;
}

void fruit_update(Fruit *f, float dt) {
    if (f->active) {
        f->timer -= dt;
        if (f->timer <= 0.0f)
            f->active = false;
    }
    if (f->score_display) {
        f->score_display_timer -= dt;
        if (f->score_display_timer <= 0.0f)
            f->score_display = false;
    }
}

void fruit_draw(Fruit *f) {
    float cx = MAZE_OFFSET_X + tile_center_px(FRUIT_TILE_X);
    float cy = MAZE_OFFSET_Y + tile_center_px(FRUIT_TILE_Y);

    if (f->active) {
        Color c = fruit_colors[f->fruit_type];
        switch (f->fruit_type) {
        case 0:
            DrawCircle((int)(cx - 3), (int)(cy + 2), 5, c);
            DrawCircle((int)(cx + 3), (int)(cy + 2), 5, c);
            DrawLineEx((Vector2){cx - 2, cy - 3}, (Vector2){cx + 1, cy - 7}, 2, (Color){0, 150, 0, 255});
            DrawLineEx((Vector2){cx + 2, cy - 3}, (Vector2){cx + 1, cy - 7}, 2, (Color){0, 150, 0, 255});
            break;
        case 1:
            DrawCircle((int)cx, (int)(cy + 1), 7, c);
            DrawTriangle((Vector2){cx - 6, cy - 2}, (Vector2){cx + 6, cy - 2}, (Vector2){cx, cy + 8}, c);
            DrawCircle((int)cx, (int)(cy - 5), 4, (Color){0, 180, 0, 255});
            break;
        case 2:
            DrawCircle((int)cx, (int)cy, 8, c);
            DrawCircle((int)(cx + 1), (int)(cy - 6), 3, (Color){0, 180, 0, 255});
            break;
        case 3:
            DrawCircle((int)cx, (int)cy, 8, c);
            DrawCircle((int)(cx + 2), (int)(cy - 7), 3, (Color){0, 120, 0, 255});
            DrawLineEx((Vector2){cx, cy - 9}, (Vector2){cx, cy - 4}, 2, (Color){100, 60, 20, 255});
            break;
        case 4:
            DrawEllipse((int)cx, (int)cy, 9, 7, c);
            for (int s = -6; s <= 6; s += 4)
                DrawLineEx((Vector2){cx + s, cy - 6}, (Vector2){cx + s, cy + 6}, 1, (Color){50, 120, 20, 255});
            break;
        case 5:
            DrawTriangle((Vector2){cx, cy - 8}, (Vector2){cx - 6, cy + 4}, (Vector2){cx + 6, cy + 4}, c);
            DrawRectangle((int)(cx - 1), (int)(cy + 4), 3, 5, (Color){200, 200, 0, 255});
            break;
        case 6:
            DrawCircle((int)cx, (int)(cy + 3), 7, c);
            DrawRectangle((int)(cx - 5), (int)(cy - 5), 10, 8, c);
            DrawCircle((int)cx, (int)(cy - 5), 5, c);
            DrawCircle((int)cx, (int)(cy + 9), 2, (Color){150, 120, 0, 255});
            break;
        case 7:
            DrawCircle((int)cx, (int)(cy - 4), 5, c);
            DrawCircle((int)cx, (int)(cy - 4), 3, COLOR_BG);
            DrawRectangle((int)(cx - 1), (int)(cy), 3, 10, c);
            DrawRectangle((int)(cx + 1), (int)(cy + 4), 4, 2, c);
            DrawRectangle((int)(cx + 1), (int)(cy + 7), 3, 2, c);
            break;
        }
    }

    if (f->score_display) {
        char pts[16];
        snprintf(pts, sizeof(pts), "%d", f->score_display_value);
        int pw = MeasureText(pts, 16);
        DrawText(pts, (int)(cx - pw / 2), (int)(cy - 8), 16, COLOR_PELLET);
    }
}
