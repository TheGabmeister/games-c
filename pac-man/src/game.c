#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Direction vectors: UP, LEFT, DOWN, RIGHT
static const int dir_dx[] = { 0, -1, 0, 1 };
static const int dir_dy[] = { -1, 0, 1, 0 };

// --- Maze helpers ---

int maze_tile_at(int tile_x, int tile_y) {
    if (tile_x < 0 || tile_x >= MAZE_COLS || tile_y < 0 || tile_y >= MAZE_ROWS)
        return TILE_WALL;
    return maze_layout[tile_y][tile_x];
}

bool maze_is_walkable(int tile_x, int tile_y) {
    // Handle tunnel wrapping
    if (tile_y >= 0 && tile_y < MAZE_ROWS) {
        if (tile_x < 0) tile_x += MAZE_COLS;
        if (tile_x >= MAZE_COLS) tile_x -= MAZE_COLS;
    }
    int t = maze_tile_at(tile_x, tile_y);
    return t != TILE_WALL;
}

bool maze_is_walkable_pacman(int tile_x, int tile_y) {
    if (tile_y >= 0 && tile_y < MAZE_ROWS) {
        if (tile_x < 0) tile_x += MAZE_COLS;
        if (tile_x >= MAZE_COLS) tile_x -= MAZE_COLS;
    }
    int t = maze_tile_at(tile_x, tile_y);
    return t != TILE_WALL && t != TILE_GHOST_DOOR;
}

// --- High score ---

void highscore_load(Game *game) {
    FILE *f = fopen("resources/highscore.dat", "rb");
    if (f) {
        fread(&game->high_score, sizeof(int), 1, f);
        fclose(f);
    }
}

void highscore_save(Game *game) {
    if (game->score > game->high_score) {
        game->high_score = game->score;
        FILE *f = fopen("resources/highscore.dat", "wb");
        if (f) {
            fwrite(&game->high_score, sizeof(int), 1, f);
            fclose(f);
        }
    }
}

// --- Initialization ---

static void pacman_init(PacMan *pm) {
    pm->tile_x = 14;
    pm->tile_y = 26;
    pm->px = pm->tile_x * TILE_SIZE + TILE_SIZE / 2.0f;
    pm->py = pm->tile_y * TILE_SIZE + TILE_SIZE / 2.0f;
    pm->dir = DIR_LEFT;
    pm->queued_dir = DIR_NONE;
    pm->speed_pct = PACMAN_SPEED_NORMAL_L1;
    pm->anim_frame = 0;
    pm->anim_timer = 0.0f;
    pm->eat_pause_frames = 0;
}

static void init_dots(Game *game) {
    game->dots_remaining = 0;
    for (int r = 0; r < MAZE_ROWS; r++) {
        for (int c = 0; c < MAZE_COLS; c++) {
            int t = maze_layout[r][c];
            if (t == TILE_DOT || t == TILE_POWER_PELLET) {
                game->dot_eaten[r][c] = false;
                game->dots_remaining++;
            } else {
                game->dot_eaten[r][c] = true;
            }
        }
    }
    game->total_dots = game->dots_remaining;
}

void game_reset_positions(Game *game) {
    pacman_init(&game->pacman);
}

void game_reset_level(Game *game) {
    init_dots(game);
    game_reset_positions(game);
    game->pellet_flash_timer = 0.0f;
    game->pellet_visible = true;
}

void game_init(Game *game) {
    memset(game, 0, sizeof(Game));
    game->state = STATE_TITLE;
    game->level = 1;
    game->score = 0;
    game->lives = 3;
    game->high_score = 0;
    highscore_load(game);
    game_reset_level(game);
}

// --- Input ---

static Direction get_input_direction(void) {
    // Keyboard
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) return DIR_UP;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) return DIR_LEFT;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) return DIR_DOWN;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) return DIR_RIGHT;

    // Gamepad
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

// --- Pac-Man Movement ---

static float pixel_to_tile_center(int tile_coord) {
    return tile_coord * TILE_SIZE + TILE_SIZE / 2.0f;
}

static void pacman_update(Game *game, float dt) {
    PacMan *pm = &game->pacman;

    if (pm->eat_pause_frames > 0) {
        pm->eat_pause_frames--;
        return;
    }

    Direction input = get_input_direction();
    if (input != DIR_NONE) {
        pm->queued_dir = input;
    }

    float speed = pm->speed_pct * BASE_SPEED * TILE_SIZE * dt;
    float center_x = pixel_to_tile_center(pm->tile_x);
    float center_y = pixel_to_tile_center(pm->tile_y);

    // Check if we can turn to queued direction
    if (pm->queued_dir != DIR_NONE) {
        int next_tx = pm->tile_x + dir_dx[pm->queued_dir];
        int next_ty = pm->tile_y + dir_dy[pm->queued_dir];

        bool can_turn = maze_is_walkable_pacman(next_tx, next_ty);

        if (can_turn) {
            // Cornering: snap if close enough to tile center
            bool on_axis;
            if (pm->queued_dir == DIR_UP || pm->queued_dir == DIR_DOWN) {
                on_axis = fabsf(pm->px - center_x) <= CORNER_TOLERANCE;
            } else {
                on_axis = fabsf(pm->py - center_y) <= CORNER_TOLERANCE;
            }

            if (on_axis) {
                // Snap to center on the perpendicular axis
                if (pm->queued_dir == DIR_UP || pm->queued_dir == DIR_DOWN) {
                    pm->px = center_x;
                } else {
                    pm->py = center_y;
                }
                pm->dir = pm->queued_dir;
                pm->queued_dir = DIR_NONE;
            }
        }
    }

    if (pm->dir == DIR_NONE) return;

    // Move in current direction
    float new_px = pm->px + dir_dx[pm->dir] * speed;
    float new_py = pm->py + dir_dy[pm->dir] * speed;

    // Tunnel wrapping
    float maze_left = 0.0f;
    float maze_right = (float)(MAZE_COLS * TILE_SIZE);
    if (new_px < maze_left) new_px += maze_right;
    if (new_px >= maze_right) new_px -= maze_right;

    // Calculate next tile from new position
    int new_tx = (int)(new_px / TILE_SIZE);
    int new_ty = (int)(new_py / TILE_SIZE);
    if (new_tx < 0) new_tx += MAZE_COLS;
    if (new_tx >= MAZE_COLS) new_tx -= MAZE_COLS;

    // Wall collision: check if new position's tile is walkable
    int check_tx = pm->tile_x + dir_dx[pm->dir];
    int check_ty = pm->tile_y + dir_dy[pm->dir];

    bool next_tile_blocked = !maze_is_walkable_pacman(check_tx, check_ty);

    if (next_tile_blocked) {
        // Stop at tile center, don't enter the wall tile
        float stop_x = center_x;
        float stop_y = center_y;

        // Only stop if we'd move past center toward the wall
        bool past_center = false;
        switch (pm->dir) {
            case DIR_RIGHT: past_center = new_px > center_x; break;
            case DIR_LEFT:  past_center = new_px < center_x; break;
            case DIR_DOWN:  past_center = new_py > center_y; break;
            case DIR_UP:    past_center = new_py < center_y; break;
            default: break;
        }

        if (past_center) {
            pm->px = stop_x;
            pm->py = stop_y;
        } else {
            pm->px = new_px;
            pm->py = new_py;
        }
    } else {
        pm->px = new_px;
        pm->py = new_py;
    }

    // Update tile position
    pm->tile_x = (int)(pm->px / TILE_SIZE);
    pm->tile_y = (int)(pm->py / TILE_SIZE);
    if (pm->tile_x < 0) pm->tile_x += MAZE_COLS;
    if (pm->tile_x >= MAZE_COLS) pm->tile_x -= MAZE_COLS;

    // Animation
    pm->anim_timer += dt;
    float anim_speed = 0.07f;
    if (pm->anim_timer >= anim_speed) {
        pm->anim_timer -= anim_speed;
        pm->anim_frame = (pm->anim_frame + 1) % 3;
    }
}

// --- Dots & Scoring ---

static void check_dot_consumption(Game *game) {
    PacMan *pm = &game->pacman;
    int tx = pm->tile_x;
    int ty = pm->tile_y;

    if (tx < 0 || tx >= MAZE_COLS || ty < 0 || ty >= MAZE_ROWS) return;
    if (game->dot_eaten[ty][tx]) return;

    int tile = maze_layout[ty][tx];
    if (tile == TILE_DOT) {
        game->dot_eaten[ty][tx] = true;
        game->score += 10;
        game->dots_remaining--;
        pm->eat_pause_frames = DOT_PAUSE_FRAMES;
        pm->speed_pct = PACMAN_SPEED_EATING_L1;
    } else if (tile == TILE_POWER_PELLET) {
        game->dot_eaten[ty][tx] = true;
        game->score += 50;
        game->dots_remaining--;
        pm->eat_pause_frames = PELLET_PAUSE_FRAMES;
    }

    if (game->score > game->high_score) {
        game->high_score = game->score;
    }
}

// --- Update ---

void game_update(Game *game) {
    float dt = GetFrameTime();

    switch (game->state) {
    case STATE_TITLE:
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
            (IsGamepadAvailable(0) && (
                IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
                IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)))) {
            game->level = 1;
            game->score = 0;
            game->lives = 3;
            game_reset_level(game);
            game->state = STATE_READY;
            game->state_timer = 2.0f;
        }
        break;

    case STATE_READY:
        game->state_timer -= dt;
        if (game->state_timer <= 0.0f) {
            game->state = STATE_PLAYING;
        }
        break;

    case STATE_PLAYING:
        // Pause
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) ||
            (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))) {
            game->state = STATE_PAUSED;
            break;
        }

        pacman_update(game, dt);
        check_dot_consumption(game);

        // Reset speed to normal after eat pause is done
        if (game->pacman.eat_pause_frames == 0) {
            game->pacman.speed_pct = PACMAN_SPEED_NORMAL_L1;
        }

        // Pellet flashing
        game->pellet_flash_timer += dt;
        if (game->pellet_flash_timer >= 0.2f) {
            game->pellet_flash_timer -= 0.2f;
            game->pellet_visible = !game->pellet_visible;
        }

        // Level complete
        if (game->dots_remaining <= 0) {
            game->state = STATE_LEVEL_COMPLETE;
            game->state_timer = 2.0f;
        }
        break;

    case STATE_LEVEL_COMPLETE:
        game->state_timer -= dt;
        if (game->state_timer <= 0.0f) {
            game->level++;
            game_reset_level(game);
            game->state = STATE_READY;
            game->state_timer = 2.0f;
        }
        break;

    case STATE_GAME_OVER:
        game->state_timer -= dt;
        if (game->state_timer <= 0.0f) {
            highscore_save(game);
            game->state = STATE_TITLE;
        }
        break;

    case STATE_PAUSED:
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) ||
            (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))) {
            game->state = STATE_PLAYING;
        }
        break;
    }
}

// --- Drawing ---

static bool is_wall(int c, int r) {
    if (c < 0 || c >= MAZE_COLS || r < 0 || r >= MAZE_ROWS) return false;
    return maze_layout[r][c] == TILE_WALL;
}

static void draw_maze(Game *game) {
    float line_thick = 2.5f;

    for (int r = 0; r < MAZE_ROWS; r++) {
        for (int c = 0; c < MAZE_COLS; c++) {
            int t = maze_layout[r][c];
            float x = MAZE_OFFSET_X + c * TILE_SIZE;
            float y = MAZE_OFFSET_Y + r * TILE_SIZE;

            if (t == TILE_WALL) {
                // Draw wall fill (dark)
                DrawRectangle((int)x, (int)y, TILE_SIZE, TILE_SIZE, (Color){15, 15, 40, 255});

                // Draw neon edges only where wall faces non-wall
                float half = line_thick / 2.0f;
                // Top edge
                if (!is_wall(c, r - 1)) {
                    DrawRectangle((int)x, (int)(y - half), TILE_SIZE, (int)line_thick, COLOR_WALL);
                    DrawRectangle((int)x, (int)(y - half - 2), TILE_SIZE, (int)(line_thick + 4), COLOR_WALL_GLOW);
                }
                // Bottom edge
                if (!is_wall(c, r + 1)) {
                    DrawRectangle((int)x, (int)(y + TILE_SIZE - half), TILE_SIZE, (int)line_thick, COLOR_WALL);
                    DrawRectangle((int)x, (int)(y + TILE_SIZE - half - 2), TILE_SIZE, (int)(line_thick + 4), COLOR_WALL_GLOW);
                }
                // Left edge
                if (!is_wall(c - 1, r)) {
                    DrawRectangle((int)(x - half), (int)y, (int)line_thick, TILE_SIZE, COLOR_WALL);
                    DrawRectangle((int)(x - half - 2), (int)y, (int)(line_thick + 4), TILE_SIZE, COLOR_WALL_GLOW);
                }
                // Right edge
                if (!is_wall(c + 1, r)) {
                    DrawRectangle((int)(x + TILE_SIZE - half), (int)y, (int)line_thick, TILE_SIZE, COLOR_WALL);
                    DrawRectangle((int)(x + TILE_SIZE - half - 2), (int)y, (int)(line_thick + 4), TILE_SIZE, COLOR_WALL_GLOW);
                }

                // Rounded corners where two edges meet
                if (!is_wall(c, r - 1) && !is_wall(c - 1, r)) {
                    DrawCircle((int)x, (int)y, line_thick + 1, COLOR_WALL);
                }
                if (!is_wall(c, r - 1) && !is_wall(c + 1, r)) {
                    DrawCircle((int)(x + TILE_SIZE), (int)y, line_thick + 1, COLOR_WALL);
                }
                if (!is_wall(c, r + 1) && !is_wall(c - 1, r)) {
                    DrawCircle((int)x, (int)(y + TILE_SIZE), line_thick + 1, COLOR_WALL);
                }
                if (!is_wall(c, r + 1) && !is_wall(c + 1, r)) {
                    DrawCircle((int)(x + TILE_SIZE), (int)(y + TILE_SIZE), line_thick + 1, COLOR_WALL);
                }
            } else if (t == TILE_GHOST_DOOR) {
                DrawRectangle((int)x, (int)(y + TILE_SIZE / 2 - 2), TILE_SIZE, 4, COLOR_GHOST_DOOR);
            }
        }
    }
}

static void draw_dots(Game *game) {
    for (int r = 0; r < MAZE_ROWS; r++) {
        for (int c = 0; c < MAZE_COLS; c++) {
            if (game->dot_eaten[r][c]) continue;

            int t = maze_layout[r][c];
            float cx = MAZE_OFFSET_X + c * TILE_SIZE + TILE_SIZE / 2.0f;
            float cy = MAZE_OFFSET_Y + r * TILE_SIZE + TILE_SIZE / 2.0f;

            if (t == TILE_DOT) {
                DrawCircle((int)cx, (int)cy, 3.0f, COLOR_DOT);
            } else if (t == TILE_POWER_PELLET && game->pellet_visible) {
                // Glowing pellet
                DrawCircle((int)cx, (int)cy, 10.0f, (Color){255, 255, 255, 40});
                DrawCircle((int)cx, (int)cy, 7.0f, (Color){255, 255, 255, 80});
                DrawCircle((int)cx, (int)cy, 5.0f, COLOR_PELLET);
            }
        }
    }
}

static void draw_pacman(Game *game) {
    PacMan *pm = &game->pacman;
    float cx = MAZE_OFFSET_X + pm->px;
    float cy = MAZE_OFFSET_Y + pm->py;
    float radius = TILE_SIZE / 2.0f - 1.0f;

    // Mouth angle based on animation frame: 0=closed, 1=half, 2=full
    float mouth_angles[] = {5.0f, 25.0f, 45.0f};
    float mouth = mouth_angles[pm->anim_frame];

    // Direction angle offset
    float dir_angle = 0.0f;
    switch (pm->dir) {
        case DIR_RIGHT: dir_angle = 0.0f; break;
        case DIR_DOWN:  dir_angle = 90.0f; break;
        case DIR_LEFT:  dir_angle = 180.0f; break;
        case DIR_UP:    dir_angle = 270.0f; break;
        default:        dir_angle = 0.0f; break;
    }

    float start_angle = dir_angle + mouth;
    float end_angle = dir_angle + 360.0f - mouth;

    DrawCircleSector((Vector2){cx, cy}, radius, start_angle, end_angle, 32, COLOR_PACMAN);
}

static void draw_hud(Game *game) {
    // Score - top left
    char score_text[32];
    snprintf(score_text, sizeof(score_text), "SCORE: %d", game->score);
    DrawText(score_text, MAZE_OFFSET_X, MAZE_OFFSET_Y + 4, 20, COLOR_TEXT);

    // High score - top center
    char hi_text[32];
    snprintf(hi_text, sizeof(hi_text), "HIGH SCORE: %d", game->high_score);
    int hi_width = MeasureText(hi_text, 20);
    DrawText(hi_text, WINDOW_WIDTH / 2 - hi_width / 2, MAZE_OFFSET_Y + 4, 20, COLOR_TEXT);

    // Level - top right
    char level_text[32];
    snprintf(level_text, sizeof(level_text), "LEVEL: %d", game->level);
    int level_width = MeasureText(level_text, 20);
    DrawText(level_text, MAZE_OFFSET_X + MAZE_WIDTH - level_width, MAZE_OFFSET_Y + 4, 20, COLOR_TEXT);

    // Lives - bottom left
    for (int i = 0; i < game->lives; i++) {
        float lx = MAZE_OFFSET_X + 20 + i * 30;
        float ly = MAZE_OFFSET_Y + MAZE_HEIGHT - 30;
        DrawCircleSector((Vector2){lx, ly}, 10.0f, 30.0f, 330.0f, 16, COLOR_PACMAN);
    }

    // Dots remaining - bottom right
    char dots_text[32];
    snprintf(dots_text, sizeof(dots_text), "DOTS: %d", game->dots_remaining);
    int dots_width = MeasureText(dots_text, 16);
    DrawText(dots_text, MAZE_OFFSET_X + MAZE_WIDTH - dots_width,
             MAZE_OFFSET_Y + MAZE_HEIGHT - 30, 16, COLOR_TEXT);
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground(COLOR_BG);

    switch (game->state) {
    case STATE_TITLE: {
        const char *title = "PAC-MAN";
        int tw = MeasureText(title, 60);
        DrawText(title, WINDOW_WIDTH / 2 - tw / 2, WINDOW_HEIGHT / 2 - 80, 60, COLOR_PACMAN);

        const char *prompt = "PRESS ENTER OR START";
        int pw = MeasureText(prompt, 24);
        DrawText(prompt, WINDOW_WIDTH / 2 - pw / 2, WINDOW_HEIGHT / 2 + 20, 24, COLOR_TEXT);

        char hi_text[32];
        snprintf(hi_text, sizeof(hi_text), "HIGH SCORE: %d", game->high_score);
        int hw = MeasureText(hi_text, 20);
        DrawText(hi_text, WINDOW_WIDTH / 2 - hw / 2, WINDOW_HEIGHT / 2 + 70, 20, COLOR_DOT);
        break;
    }

    case STATE_READY:
        draw_maze(game);
        draw_dots(game);
        draw_pacman(game);
        draw_hud(game);
        {
            const char *ready = "READY!";
            int rw = MeasureText(ready, 30);
            DrawText(ready, WINDOW_WIDTH / 2 - rw / 2,
                     MAZE_OFFSET_Y + 20 * TILE_SIZE, 30, COLOR_READY);
        }
        break;

    case STATE_PLAYING:
        draw_maze(game);
        draw_dots(game);
        draw_pacman(game);
        draw_hud(game);
        break;

    case STATE_LEVEL_COMPLETE: {
        draw_maze(game);
        draw_hud(game);
        // Flash walls effect
        bool flash = ((int)(game->state_timer * 4.0f)) % 2 == 0;
        if (flash) {
            const char *complete = "LEVEL COMPLETE!";
            int cw = MeasureText(complete, 30);
            DrawText(complete, WINDOW_WIDTH / 2 - cw / 2,
                     MAZE_OFFSET_Y + 20 * TILE_SIZE, 30, COLOR_TEXT);
        }
        break;
    }

    case STATE_GAME_OVER: {
        draw_maze(game);
        draw_dots(game);
        draw_hud(game);
        const char *go = "GAME OVER";
        int gow = MeasureText(go, 40);
        DrawText(go, WINDOW_WIDTH / 2 - gow / 2,
                 MAZE_OFFSET_Y + 20 * TILE_SIZE, 40, (Color){255, 0, 0, 255});
        break;
    }

    case STATE_PAUSED: {
        draw_maze(game);
        draw_dots(game);
        draw_pacman(game);
        draw_hud(game);
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 128});
        const char *paused = "PAUSED";
        int pw = MeasureText(paused, 50);
        DrawText(paused, WINDOW_WIDTH / 2 - pw / 2,
                 WINDOW_HEIGHT / 2 - 25, 50, COLOR_TEXT);
        break;
    }
    }

    EndDrawing();
}
