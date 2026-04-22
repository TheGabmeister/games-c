#include "ghost.h"

static const float ghost_speed_normal[]      = { 0.75f, 0.85f, 0.95f, 0.95f };
static const float ghost_speed_frightened[]  = { 0.50f, 0.55f, 0.60f, 0.60f };
static const float ghost_speed_tunnel[]      = { 0.40f, 0.45f, 0.50f, 0.50f };
static const float ghost_speed_eaten         = 1.50f;

static const int elroy_thresholds[][2] = {
    {20,10}, {30,15}, {40,20}, {50,25}, {60,30}, {80,40}, {100,50}, {120,60}
};

static const int ghost_house_dot_limits[3][GHOST_COUNT] = {
    { -1, 0, 30, 60 },
    { -1, 0,  0, 50 },
    { -1, 0,  0,  0 },
};

static const Color ghost_colors[GHOST_COUNT] = {
    {255, 0, 0, 255},
    {255, 184, 255, 255},
    {0, 255, 255, 255},
    {255, 184, 82, 255},
};

static const int scatter_corners[GHOST_COUNT][2] = {
    {25, 0}, {2, 0}, {27, 35}, {0, 35},
};

static const int ghost_home_tiles[GHOST_COUNT][2] = {
    {14, 14}, {14, 16}, {12, 16}, {16, 16},
};

static int elroy_tier(int level) {
    if (level <= 1) return 0;
    if (level <= 2) return 1;
    if (level <= 5) return 2;
    if (level <= 8) return 3;
    if (level <= 11) return 4;
    if (level <= 14) return 5;
    if (level <= 18) return 6;
    return 7;
}

static int ghost_house_tier(int level) {
    if (level <= 1) return 0;
    if (level <= 2) return 1;
    return 2;
}

void ghost_init_single(Ghost *g, int index) {
    g->color = ghost_colors[index];
    g->scatter_target_x = scatter_corners[index][0];
    g->scatter_target_y = scatter_corners[index][1];
    g->home_tile_x = ghost_home_tiles[index][0];
    g->home_tile_y = ghost_home_tiles[index][1];
    g->tile_x = g->home_tile_x;
    g->tile_y = g->home_tile_y;
    g->px = tile_center_px(g->tile_x);
    g->py = tile_center_px(g->tile_y);
    g->house_bob_timer = 0.0f;
    g->target_x = g->scatter_target_x;
    g->target_y = g->scatter_target_y;
    g->mode_before_fright = GHOST_SCATTER;

    if (index == GHOST_BLINKY) {
        g->mode = GHOST_SCATTER;
        g->dir = DIR_LEFT;
    } else {
        g->mode = GHOST_IN_HOUSE;
        g->dir = DIR_DOWN;
    }
}

void ghost_init_all(Ghost ghosts[GHOST_COUNT], int level, bool use_global_dot_counter) {
    int tier = ghost_house_tier(level);
    for (int i = 0; i < GHOST_COUNT; i++) {
        ghost_init_single(&ghosts[i], i);
        ghosts[i].personal_dot_limit = ghost_house_dot_limits[tier][i];
        ghosts[i].personal_dot_counter = 0;
        ghosts[i].use_personal_counter = !use_global_dot_counter;
    }
}

// --- AI ---

static Direction ghost_choose_direction(Ghost *g, int target_x, int target_y) {
    Direction best = DIR_NONE;
    float best_dist = 1e18f;
    Direction reverse = direction_opposite(g->dir);

    Direction priority[] = { DIR_UP, DIR_LEFT, DIR_DOWN, DIR_RIGHT };
    for (int i = 0; i < 4; i++) {
        Direction d = priority[i];
        if (d == reverse) continue;

        int nx = g->tile_x + dir_dx[d];
        int ny = g->tile_y + dir_dy[d];
        if (!maze_is_walkable_ghost(nx, ny, g->mode)) continue;

        float dx_f = (float)(nx - target_x);
        float dy_f = (float)(ny - target_y);
        float dist = dx_f * dx_f + dy_f * dy_f;
        if (dist < best_dist) {
            best_dist = dist;
            best = d;
        }
    }
    return best;
}

void ghost_compute_target(Ghost *g, int ghost_idx, PacMan *pm, Ghost ghosts[GHOST_COUNT]) {
    if (g->mode == GHOST_SCATTER) {
        g->target_x = g->scatter_target_x;
        g->target_y = g->scatter_target_y;
        return;
    }
    if (g->mode != GHOST_CHASE) return;

    switch (ghost_idx) {
    case GHOST_BLINKY:
        g->target_x = pm->tile_x;
        g->target_y = pm->tile_y;
        break;
    case GHOST_PINKY: {
        int ahead_x = pm->tile_x + dir_dx[pm->dir] * 4;
        int ahead_y = pm->tile_y + dir_dy[pm->dir] * 4;
        if (pm->dir == DIR_UP) ahead_x -= 4;
        g->target_x = ahead_x;
        g->target_y = ahead_y;
        break;
    }
    case GHOST_INKY: {
        int ref_x = pm->tile_x + dir_dx[pm->dir] * 2;
        int ref_y = pm->tile_y + dir_dy[pm->dir] * 2;
        if (pm->dir == DIR_UP) ref_x -= 2;
        Ghost *blinky = &ghosts[GHOST_BLINKY];
        int vec_x = ref_x - blinky->tile_x;
        int vec_y = ref_y - blinky->tile_y;
        g->target_x = ref_x + vec_x;
        g->target_y = ref_y + vec_y;
        break;
    }
    case GHOST_CLYDE: {
        float dx_f = (float)(g->tile_x - pm->tile_x);
        float dy_f = (float)(g->tile_y - pm->tile_y);
        float dist = sqrtf(dx_f * dx_f + dy_f * dy_f);
        if (dist > 8.0f) {
            g->target_x = pm->tile_x;
            g->target_y = pm->tile_y;
        } else {
            g->target_x = g->scatter_target_x;
            g->target_y = g->scatter_target_y;
        }
        break;
    }
    }
}

// --- Movement ---

void ghost_update_movement(Ghost *g, int ghost_idx, int level, int dots_remaining,
                           GlobalMode global_mode, PacMan *pm, Ghost ghosts[GHOST_COUNT], float dt) {
    int tier = speed_tier(level);
    float spd;

    switch (g->mode) {
    case GHOST_IN_HOUSE:
        g->house_bob_timer += dt * 3.0f;
        g->py = tile_center_px(g->home_tile_y) + sinf(g->house_bob_timer) * 4.0f;
        g->tile_y = g->home_tile_y;
        return;

    case GHOST_EXITING: {
        float target_px = tile_center_px(GHOST_HOUSE_CENTER_X);
        float target_py = tile_center_px(GHOST_HOUSE_EXIT_Y);
        float exit_speed = ghost_speed_normal[tier] * BASE_SPEED * TILE_SIZE * dt;
        float dx_f = target_px - g->px;

        if (fabsf(g->px - target_px) > 1.0f) {
            g->px += (dx_f > 0 ? 1 : -1) * exit_speed;
        } else {
            g->px = target_px;
            if (fabsf(g->py - target_py) > 1.0f) {
                g->py -= exit_speed;
            } else {
                g->py = target_py;
                g->tile_x = GHOST_HOUSE_EXIT_X;
                g->tile_y = GHOST_HOUSE_EXIT_Y;
                g->px = tile_center_px(g->tile_x);
                g->py = tile_center_px(g->tile_y);
                g->dir = DIR_LEFT;
                g->mode = (global_mode == GLOBAL_CHASE) ? GHOST_CHASE : GHOST_SCATTER;
            }
        }
        g->tile_x = (int)(g->px / TILE_SIZE);
        g->tile_y = (int)(g->py / TILE_SIZE);
        return;
    }

    case GHOST_EATEN: {
        bool at_exit_column = (g->tile_x == GHOST_HOUSE_EXIT_X &&
                               g->tile_y >= GHOST_HOUSE_EXIT_Y &&
                               g->tile_y <= GHOST_HOUSE_CENTER_Y);
        if (at_exit_column) {
            float center_y_target = tile_center_px(GHOST_HOUSE_CENTER_Y);
            float move = ghost_speed_eaten * BASE_SPEED * TILE_SIZE * dt;
            g->px = tile_center_px(GHOST_HOUSE_EXIT_X);
            g->py += move;
            if (g->py >= center_y_target) {
                g->py = center_y_target;
                g->tile_x = GHOST_HOUSE_CENTER_X;
                g->tile_y = GHOST_HOUSE_CENTER_Y;
                g->mode = GHOST_EXITING;
            }
            g->tile_y = (int)(g->py / TILE_SIZE);
            return;
        }
        g->target_x = GHOST_HOUSE_EXIT_X;
        g->target_y = GHOST_HOUSE_EXIT_Y;
        spd = ghost_speed_eaten;
        break;
    }

    case GHOST_FRIGHTENED:
        spd = ghost_speed_frightened[tier];
        break;
    default:
        spd = ghost_speed_normal[tier];
        break;
    }

    // Cruise Elroy
    if (ghost_idx == GHOST_BLINKY && (g->mode == GHOST_CHASE || g->mode == GHOST_SCATTER)) {
        int et = elroy_tier(level);
        if (dots_remaining <= elroy_thresholds[et][1])
            spd = ghost_speed_normal[tier] + 0.05f;
        else if (dots_remaining <= elroy_thresholds[et][0])
            spd = ghost_speed_normal[tier] + 0.025f;
    }

    if (maze_is_tunnel(g->tile_x, g->tile_y) && g->mode != GHOST_EATEN)
        spd = ghost_speed_tunnel[tier];

    float move_speed = spd * BASE_SPEED * TILE_SIZE * dt;
    float center_x = tile_center_px(g->tile_x);
    float center_y = tile_center_px(g->tile_y);

    bool at_center = fabsf(g->px - center_x) < 1.5f && fabsf(g->py - center_y) < 1.5f;

    if (at_center) {
        g->px = center_x;
        g->py = center_y;

        Direction new_dir;
        if (g->mode == GHOST_FRIGHTENED) {
            Direction reverse = direction_opposite(g->dir);
            Direction candidates[4];
            int count = 0;
            Direction priority[] = { DIR_UP, DIR_LEFT, DIR_DOWN, DIR_RIGHT };
            for (int i = 0; i < 4; i++) {
                if (priority[i] == reverse) continue;
                int nx = g->tile_x + dir_dx[priority[i]];
                int ny = g->tile_y + dir_dy[priority[i]];
                if (maze_is_walkable_ghost(nx, ny, g->mode))
                    candidates[count++] = priority[i];
            }
            new_dir = (count > 0) ? candidates[GetRandomValue(0, count - 1)] : reverse;
        } else {
            ghost_compute_target(g, ghost_idx, pm, ghosts);
            new_dir = ghost_choose_direction(g, g->target_x, g->target_y);
        }

        if (new_dir != DIR_NONE)
            g->dir = new_dir;
    }

    if (g->dir != DIR_NONE) {
        float new_px = g->px + dir_dx[g->dir] * move_speed;
        float new_py = g->py + dir_dy[g->dir] * move_speed;

        float maze_right = (float)(MAZE_COLS * TILE_SIZE);
        if (new_px < 0) new_px += maze_right;
        if (new_px >= maze_right) new_px -= maze_right;

        int next_tx = g->tile_x + dir_dx[g->dir];
        int next_ty = g->tile_y + dir_dy[g->dir];
        if (!maze_is_walkable_ghost(next_tx, next_ty, g->mode)) {
            bool past = false;
            switch (g->dir) {
                case DIR_RIGHT: past = new_px > center_x; break;
                case DIR_LEFT:  past = new_px < center_x; break;
                case DIR_DOWN:  past = new_py > center_y; break;
                case DIR_UP:    past = new_py < center_y; break;
                default: break;
            }
            if (past) { new_px = center_x; new_py = center_y; }
        }
        g->px = new_px;
        g->py = new_py;
    }

    g->tile_x = (int)(g->px / TILE_SIZE);
    g->tile_y = (int)(g->py / TILE_SIZE);
    if (g->tile_x < 0) g->tile_x += MAZE_COLS;
    if (g->tile_x >= MAZE_COLS) g->tile_x -= MAZE_COLS;
}

// --- Drawing ---

void ghost_draw(Ghost *g, float frightened_timer) {
    float cx = MAZE_OFFSET_X + g->px;
    float cy = MAZE_OFFSET_Y + g->py;
    float r = TILE_SIZE / 2.0f - 1.0f;

    if (g->mode == GHOST_EATEN) {
        float eye_offset = 3.0f, eye_r = 3.0f, pupil_r = 1.5f;
        float edx = 0, edy = 0;
        switch (g->dir) {
            case DIR_UP: edy = -2; break; case DIR_DOWN: edy = 2; break;
            case DIR_LEFT: edx = -2; break; case DIR_RIGHT: edx = 2; break;
            default: break;
        }
        DrawCircle((int)(cx - eye_offset), (int)(cy - 2), eye_r, WHITE);
        DrawCircle((int)(cx + eye_offset), (int)(cy - 2), eye_r, WHITE);
        DrawCircle((int)(cx - eye_offset + edx), (int)(cy - 2 + edy), pupil_r, (Color){33, 33, 222, 255});
        DrawCircle((int)(cx + eye_offset + edx), (int)(cy - 2 + edy), pupil_r, (Color){33, 33, 222, 255});
        return;
    }

    Color body_color;
    if (g->mode == GHOST_FRIGHTENED) {
        bool flash_white = false;
        if (frightened_timer < 2.0f && frightened_timer > 0.0f)
            flash_white = fmodf(frightened_timer, 0.28f) < 0.14f;
        body_color = flash_white ? COLOR_FRIGHT_FLASH : COLOR_FRIGHTENED;
    } else {
        body_color = g->color;
    }

    DrawCircle((int)cx, (int)(cy - 2), r, body_color);
    DrawRectangle((int)(cx - r), (int)(cy - 2), (int)(r * 2), (int)(r + 2), body_color);
    for (int w = 0; w < 3; w++) {
        float wx = cx - r + (r * 2.0f / 3.0f) * w + (r / 3.0f);
        DrawCircle((int)wx, (int)(cy + r), r / 3.0f, body_color);
    }

    if (g->mode == GHOST_FRIGHTENED) {
        Color face = (body_color.r == 255) ? COLOR_FRIGHTENED : WHITE;
        DrawCircle((int)(cx - 3), (int)(cy - 3), 2.0f, face);
        DrawCircle((int)(cx + 3), (int)(cy - 3), 2.0f, face);
        for (int m = -4; m <= 4; m += 2)
            DrawCircle((int)(cx + m), (int)((m % 4 == 0) ? cy + 3 : cy + 1), 1.0f, face);
    } else {
        float eye_offset = 3.0f, eye_r = 3.5f, pupil_r = 2.0f;
        float edx = 0, edy = 0;
        switch (g->dir) {
            case DIR_UP: edy = -1.5f; break; case DIR_DOWN: edy = 1.5f; break;
            case DIR_LEFT: edx = -1.5f; break; case DIR_RIGHT: edx = 1.5f; break;
            default: break;
        }
        DrawCircle((int)(cx - eye_offset), (int)(cy - 3), eye_r, WHITE);
        DrawCircle((int)(cx + eye_offset), (int)(cy - 3), eye_r, WHITE);
        DrawCircle((int)(cx - eye_offset + edx), (int)(cy - 3 + edy), pupil_r, (Color){33, 33, 222, 255});
        DrawCircle((int)(cx + eye_offset + edx), (int)(cy - 3 + edy), pupil_r, (Color){33, 33, 222, 255});
    }
}

void ghost_draw_all(Ghost ghosts[GHOST_COUNT], float frightened_timer) {
    for (int i = 0; i < GHOST_COUNT; i++)
        ghost_draw(&ghosts[i], frightened_timer);
}
