#include "pacman.h"

static const float speed_normal[]     = { 0.80f, 0.90f, 1.00f, 0.90f };
static const float speed_eating[]     = { 0.71f, 0.79f, 0.87f, 0.79f };
static const float speed_frightened[] = { 0.90f, 0.95f, 1.00f, 0.90f };

static Direction get_input_direction(void) {
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) return DIR_UP;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) return DIR_LEFT;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) return DIR_DOWN;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) return DIR_RIGHT;

    if (IsGamepadAvailable(0)) {
        float gx = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float gy = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        float deadzone = 0.3f;
        if (fabsf(gx) > fabsf(gy)) {
            if (gx < -deadzone) return DIR_LEFT;
            if (gx > deadzone) return DIR_RIGHT;
        } else {
            if (gy < -deadzone) return DIR_UP;
            if (gy > deadzone) return DIR_DOWN;
        }
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) return DIR_UP;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) return DIR_LEFT;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) return DIR_DOWN;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) return DIR_RIGHT;
    }
    return DIR_NONE;
}

void pacman_init(PacMan *pm) {
    pm->tile_x = 14;
    pm->tile_y = 26;
    pm->px = tile_center_px(pm->tile_x);
    pm->py = tile_center_px(pm->tile_y);
    pm->dir = DIR_LEFT;
    pm->queued_dir = DIR_NONE;
    pm->speed_pct = 0.80f;
    pm->anim_frame = 0;
    pm->anim_timer = 0.0f;
    pm->eat_pause_frames = 0;
    pm->death_timer = 0.0f;
    pm->death_frame = 0;
}

void pacman_update(PacMan *pm, int level, bool frightened_active, float dt) {
    if (pm->eat_pause_frames > 0) {
        pm->eat_pause_frames--;
        return;
    }

    Direction input = get_input_direction();
    if (input != DIR_NONE)
        pm->queued_dir = input;

    int tier = speed_tier(level);
    if (frightened_active)
        pm->speed_pct = speed_frightened[tier];
    else
        pm->speed_pct = speed_normal[tier];

    float speed = pm->speed_pct * BASE_SPEED * TILE_SIZE * dt;
    float center_x = tile_center_px(pm->tile_x);
    float center_y = tile_center_px(pm->tile_y);

    if (pm->queued_dir != DIR_NONE) {
        int next_tx = pm->tile_x + dir_dx[pm->queued_dir];
        int next_ty = pm->tile_y + dir_dy[pm->queued_dir];
        if (maze_is_walkable_pacman(next_tx, next_ty)) {
            bool on_axis;
            if (pm->queued_dir == DIR_UP || pm->queued_dir == DIR_DOWN)
                on_axis = fabsf(pm->px - center_x) <= CORNER_TOLERANCE;
            else
                on_axis = fabsf(pm->py - center_y) <= CORNER_TOLERANCE;

            if (on_axis) {
                if (pm->queued_dir == DIR_UP || pm->queued_dir == DIR_DOWN)
                    pm->px = center_x;
                else
                    pm->py = center_y;
                pm->dir = pm->queued_dir;
                pm->queued_dir = DIR_NONE;
            }
        }
    }

    if (pm->dir == DIR_NONE) return;

    float new_px = pm->px + dir_dx[pm->dir] * speed;
    float new_py = pm->py + dir_dy[pm->dir] * speed;

    float maze_right = (float)(MAZE_COLS * TILE_SIZE);
    if (new_px < 0) new_px += maze_right;
    if (new_px >= maze_right) new_px -= maze_right;

    int check_tx = pm->tile_x + dir_dx[pm->dir];
    int check_ty = pm->tile_y + dir_dy[pm->dir];
    bool blocked = !maze_is_walkable_pacman(check_tx, check_ty);

    if (blocked) {
        bool past_center = false;
        switch (pm->dir) {
            case DIR_RIGHT: past_center = new_px > center_x; break;
            case DIR_LEFT:  past_center = new_px < center_x; break;
            case DIR_DOWN:  past_center = new_py > center_y; break;
            case DIR_UP:    past_center = new_py < center_y; break;
            default: break;
        }
        if (past_center) {
            pm->px = center_x;
            pm->py = center_y;
        } else {
            pm->px = new_px;
            pm->py = new_py;
        }
    } else {
        pm->px = new_px;
        pm->py = new_py;
    }

    pm->tile_x = (int)(pm->px / TILE_SIZE);
    pm->tile_y = (int)(pm->py / TILE_SIZE);
    if (pm->tile_x < 0) pm->tile_x += MAZE_COLS;
    if (pm->tile_x >= MAZE_COLS) pm->tile_x -= MAZE_COLS;

    if (!blocked || !((pm->px == center_x) && (pm->py == center_y))) {
        pm->anim_timer += dt;
        if (pm->anim_timer >= 0.07f) {
            pm->anim_timer -= 0.07f;
            pm->anim_frame = (pm->anim_frame + 1) % 3;
        }
    }
}

void pacman_draw(PacMan *pm, bool dying) {
    float cx = MAZE_OFFSET_X + pm->px;
    float cy = MAZE_OFFSET_Y + pm->py;
    float radius = TILE_SIZE / 2.0f - 1.0f;

    if (dying) {
        float progress = pm->death_frame / 10.0f;
        float mouth = 45.0f + progress * 135.0f;
        float shrink = 1.0f - progress * 0.8f;
        if (shrink < 0.1f) shrink = 0.1f;
        float start_angle = 270.0f + mouth;
        float end_angle = 270.0f + 360.0f - mouth;
        if (end_angle <= start_angle) return;
        DrawCircleSector((Vector2){cx, cy}, radius * shrink, start_angle, end_angle, 32, COLOR_PACMAN);
        return;
    }

    float mouth_angles[] = {5.0f, 25.0f, 45.0f};
    float mouth = mouth_angles[pm->anim_frame];

    float dir_angle = 0.0f;
    switch (pm->dir) {
        case DIR_RIGHT: dir_angle = 0.0f; break;
        case DIR_DOWN:  dir_angle = 90.0f; break;
        case DIR_LEFT:  dir_angle = 180.0f; break;
        case DIR_UP:    dir_angle = 270.0f; break;
        default:        dir_angle = 180.0f; break;
    }

    DrawCircleSector((Vector2){cx, cy}, radius, dir_angle + mouth, dir_angle + 360.0f - mouth, 32, COLOR_PACMAN);
}
