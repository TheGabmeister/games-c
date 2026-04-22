#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Direction vectors: UP, LEFT, DOWN, RIGHT
static const int dir_dx[] = { 0, -1, 0, 1 };
static const int dir_dy[] = { -1, 0, 1, 0 };

// --- Speed tables ---
// Index: 0=Level1, 1=Levels2-4, 2=Levels5-20, 3=Levels21+
static const float pacman_speed_normal[]     = { 0.80f, 0.90f, 1.00f, 0.90f };
static const float pacman_speed_eating[]     = { 0.71f, 0.79f, 0.87f, 0.79f };
static const float pacman_speed_frightened[] = { 0.90f, 0.95f, 1.00f, 0.90f };
static const float ghost_speed_normal[]      = { 0.75f, 0.85f, 0.95f, 0.95f };
static const float ghost_speed_frightened[]  = { 0.50f, 0.55f, 0.60f, 0.60f };
static const float ghost_speed_tunnel[]      = { 0.40f, 0.45f, 0.50f, 0.50f };
static const float ghost_speed_eaten         = 1.50f;

// Scatter/chase timing: [phase][tier] in seconds
// Tiers: 0=Level1, 1=Levels2-4, 2=Levels5+
static const float scatter_chase_times[SCATTER_CHASE_PHASES][3] = {
    { 7.0f,  7.0f,    5.0f    },  // Scatter 1
    { 20.0f, 20.0f,   20.0f   },  // Chase 1
    { 7.0f,  7.0f,    5.0f    },  // Scatter 2
    { 20.0f, 20.0f,   20.0f   },  // Chase 2
    { 5.0f,  5.0f,    5.0f    },  // Scatter 3
    { 20.0f, 1033.0f, 1037.0f },  // Chase 3
    { 5.0f,  1.0f/60, 1.0f/60 },  // Scatter 4
    { 9999.0f, 9999.0f, 9999.0f }, // Chase 4+ (indefinite)
};

// Frightened duration by level (0-indexed internally, capped at 19)
static const float fright_duration[] = {
    6,5,4,3,2,5,2,2,1,5,2,1,1,3,1,1,0,1,0
};
static const int fright_flashes[] = {
    5,5,5,5,5,5,5,5,3,5,5,3,3,5,3,3,0,3,0
};

// Cruise Elroy thresholds: [tier][0]=elroy1_dots, [1]=elroy2_dots
// Tiers: 0=L1, 1=L2, 2=L3-5, 3=L6-8, 4=L9-11, 5=L12-14, 6=L15-18, 7=L19+
static const int elroy_thresholds[][2] = {
    {20,10}, {30,15}, {40,20}, {50,25}, {60,30}, {80,40}, {100,50}, {120,60}
};

// Ghost house dot limits per level: [level_tier][ghost_index] (-1 = immediate)
// Tiers: 0=Level1, 1=Level2, 2=Level3+
static const int ghost_house_dot_limits[3][GHOST_COUNT] = {
    { -1, 0, 30, 60 },   // Level 1: Blinky outside, Pinky immediate, Inky 30, Clyde 60
    { -1, 0,  0, 50 },   // Level 2
    { -1, 0,  0,  0 },   // Level 3+
};

// Global dot counter limits after death
static const int global_dot_limits[GHOST_COUNT] = { -1, 7, 17, 32 };

// Ghost colors
static const Color ghost_colors[GHOST_COUNT] = {
    {255, 0, 0, 255},     // Blinky - Red
    {255, 184, 255, 255},  // Pinky - Pink
    {0, 255, 255, 255},    // Inky - Cyan
    {255, 184, 82, 255},   // Clyde - Orange
};

// Ghost scatter corners (tile coords)
static const int scatter_corners[GHOST_COUNT][2] = {
    {25, 0},   // Blinky - top-right
    {2, 0},    // Pinky - top-left
    {27, 35},  // Inky - bottom-right
    {0, 35},   // Clyde - bottom-left
};

// Ghost home positions inside the house (tile coords)
static const int ghost_home_tiles[GHOST_COUNT][2] = {
    {14, 14},  // Blinky - outside house (start position above door)
    {14, 16},  // Pinky - center of house
    {12, 16},  // Inky - left of house
    {16, 16},  // Clyde - right of house
};

// Fruit points by type index: cherry, strawberry, orange, apple, melon, galaxian, bell, key
static const int fruit_points[FRUIT_COUNT] = { 100, 300, 500, 700, 1000, 2000, 3000, 5000 };

// Fruit colors for procedural drawing
static const Color fruit_colors[FRUIT_COUNT] = {
    {255, 0, 0, 255},      // Cherry - red
    {255, 50, 50, 255},     // Strawberry - red-pink
    {255, 165, 0, 255},     // Orange
    {0, 200, 0, 255},       // Apple - green
    {100, 200, 50, 255},    // Melon - green-yellow
    {255, 255, 0, 255},     // Galaxian - yellow
    {255, 215, 0, 255},     // Bell - gold
    {150, 200, 255, 255},   // Key - light blue
};

static const char *sound_files[SOUND_COUNT] = {
    "resources/dot_a.wav",
    "resources/dot_b.wav",
    "resources/power_pellet.wav",
    "resources/ghost_eaten.wav",
    "resources/fruit.wav",
    "resources/death.wav",
    "resources/extra_life.wav",
    "resources/ready.wav",
    "resources/menu_select.wav",
};

// --- Utility ---

static int speed_tier(int level) {
    if (level <= 1) return 0;
    if (level <= 4) return 1;
    if (level <= 20) return 2;
    return 3;
}

static int scatter_chase_tier(int level) {
    if (level <= 1) return 0;
    if (level <= 4) return 1;
    return 2;
}

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

static int fright_level_index(int level) {
    int idx = level - 1;
    if (idx < 0) idx = 0;
    if (idx >= 19) idx = 18;
    return idx;
}

static Direction direction_opposite(Direction d) {
    if (d == DIR_NONE) return DIR_NONE;
    return (Direction)((d + 2) % 4);
}

static float tile_center_px(int tile) {
    return tile * TILE_SIZE + TILE_SIZE / 2.0f;
}

static int fruit_type_for_level(int level) {
    if (level <= 1) return 0;  // Cherry
    if (level <= 2) return 1;  // Strawberry
    if (level <= 4) return 2;  // Orange
    if (level <= 6) return 3;  // Apple
    if (level <= 8) return 4;  // Melon
    if (level <= 10) return 5; // Galaxian
    if (level <= 12) return 6; // Bell
    return 7;                  // Key
}

// --- Sound ---

void game_load_sounds(Game *game) {
    for (int i = 0; i < SOUND_COUNT; i++)
        game->sounds[i] = LoadSound(sound_files[i]);
}

void game_unload_sounds(Game *game) {
    for (int i = 0; i < SOUND_COUNT; i++)
        UnloadSound(game->sounds[i]);
}

static void play_sound(Game *game, SoundID id) {
    PlaySound(game->sounds[id]);
}

// --- Particles ---

static void spawn_particles(Game *game, float x, float y, Color color, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!game->particles[j].active) {
                Particle *p = &game->particles[j];
                p->active = true;
                p->x = x;
                p->y = y;
                float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
                float speed = (float)GetRandomValue(20, 80);
                p->vx = cosf(angle) * speed;
                p->vy = sinf(angle) * speed;
                p->life = 0.3f + (float)GetRandomValue(0, 20) / 100.0f;
                p->max_life = p->life;
                p->color = color;
                break;
            }
        }
    }
}

static void update_particles(Game *game, float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &game->particles[i];
        if (!p->active) continue;
        p->x += p->vx * dt;
        p->y += p->vy * dt;
        p->life -= dt;
        if (p->life <= 0) p->active = false;
    }
}

static void draw_particles(Game *game) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &game->particles[i];
        if (!p->active) continue;
        float alpha = p->life / p->max_life;
        Color c = p->color;
        c.a = (unsigned char)(alpha * 255);
        float size = 2.0f * alpha;
        DrawCircle((int)p->x, (int)p->y, size, c);
    }
}

// --- Maze helpers ---

int maze_tile_at(int tile_x, int tile_y) {
    if (tile_x < 0 || tile_x >= MAZE_COLS || tile_y < 0 || tile_y >= MAZE_ROWS)
        return TILE_WALL;
    return maze_layout[tile_y][tile_x];
}

static int maze_tile_at_wrapped(int tile_x, int tile_y) {
    if (tile_y >= 0 && tile_y < MAZE_ROWS) {
        if (tile_x < 0) tile_x += MAZE_COLS;
        if (tile_x >= MAZE_COLS) tile_x -= MAZE_COLS;
    }
    return maze_tile_at(tile_x, tile_y);
}

bool maze_is_walkable(int tile_x, int tile_y) {
    int t = maze_tile_at_wrapped(tile_x, tile_y);
    return t != TILE_WALL;
}

bool maze_is_walkable_pacman(int tile_x, int tile_y) {
    int t = maze_tile_at_wrapped(tile_x, tile_y);
    return t != TILE_WALL && t != TILE_GHOST_DOOR;
}

bool maze_is_walkable_ghost(int tile_x, int tile_y, GhostMode mode) {
    int t = maze_tile_at_wrapped(tile_x, tile_y);
    if (t == TILE_WALL) return false;
    if (t == TILE_GHOST_DOOR && mode != GHOST_EXITING && mode != GHOST_EATEN)
        return false;
    return true;
}

bool maze_is_tunnel(int tile_x, int tile_y) {
    int t = maze_tile_at(tile_x, tile_y);
    return t == TILE_TUNNEL;
}

bool maze_is_intersection(int tile_x, int tile_y) {
    int walkable = 0;
    for (int d = 0; d < 4; d++) {
        if (maze_is_walkable(tile_x + dir_dx[d], tile_y + dir_dy[d]))
            walkable++;
    }
    return walkable >= 3;
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

// --- Ghost initialization ---

static void ghost_init(Ghost *g, int index) {
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

static void ghosts_init(Game *game) {
    int tier = ghost_house_tier(game->level);
    for (int i = 0; i < GHOST_COUNT; i++) {
        ghost_init(&game->ghosts[i], i);
        game->ghosts[i].personal_dot_limit = ghost_house_dot_limits[tier][i];
        game->ghosts[i].personal_dot_counter = 0;
        game->ghosts[i].use_personal_counter = !game->use_global_dot_counter;
    }
}

// --- Pac-Man initialization ---

static void pacman_init(PacMan *pm) {
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

// --- Level initialization ---

static void init_dots(Game *game) {
    game->dots_remaining = 0;
    game->dots_eaten_this_level = 0;
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

static void reset_scatter_chase(Game *game) {
    game->global_mode = GLOBAL_SCATTER;
    game->scatter_chase_phase = 0;
    int tier = scatter_chase_tier(game->level);
    game->scatter_chase_timer = scatter_chase_times[0][tier];
}

static void game_reset_level(Game *game) {
    init_dots(game);
    pacman_init(&game->pacman);
    game->use_global_dot_counter = false;
    game->global_dot_counter = 0;
    ghosts_init(game);
    game->pellet_flash_timer = 0.0f;
    game->pellet_visible = true;
    game->frightened_active = false;
    game->frightened_timer = 0.0f;
    game->ghost_eat_combo = 0;
    game->ghost_eaten_pause = false;
    game->no_dot_timer = 0.0f;
    game->no_dot_timeout = (game->level <= 4) ? 4.0f : 3.0f;
    game->level_complete_flash_white = false;
    game->fruit.active = false;
    game->fruit.score_display = false;
    game->fruit_spawned_70 = false;
    game->fruit_spawned_170 = false;
    game->dot_sound_toggle = false;
    game->waka_timer = 0.0f;
    memset(game->particles, 0, sizeof(game->particles));
    reset_scatter_chase(game);
}

static void game_reset_positions(Game *game) {
    pacman_init(&game->pacman);
    game->use_global_dot_counter = true;
    game->global_dot_counter = 0;
    ghosts_init(game);
    game->frightened_active = false;
    game->frightened_timer = 0.0f;
    game->ghost_eat_combo = 0;
    game->ghost_eaten_pause = false;
    game->no_dot_timer = 0.0f;
    reset_scatter_chase(game);
}

void game_init(Game *game) {
    memset(game, 0, sizeof(Game));
    game->state = STATE_TITLE;
    game->level = 1;
    game->score = 0;
    game->lives = 3;
    game->high_score = 0;
    game->extra_life_given = false;
    highscore_load(game);
    game_reset_level(game);
}

// --- Input ---

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

// --- Pac-Man Movement ---

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

    int tier = speed_tier(game->level);
    if (game->frightened_active) {
        pm->speed_pct = pacman_speed_frightened[tier];
    } else if (pm->eat_pause_frames > 0) {
        pm->speed_pct = pacman_speed_eating[tier];
    } else {
        pm->speed_pct = pacman_speed_normal[tier];
    }

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

    // Tunnel wrapping
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

    // Animation: only animate while moving (stop when blocked at wall)
    if (!blocked || !((pm->px == center_x) && (pm->py == center_y))) {
        pm->anim_timer += dt;
        if (pm->anim_timer >= 0.07f) {
            pm->anim_timer -= 0.07f;
            pm->anim_frame = (pm->anim_frame + 1) % 3;
        }
    }
}

// --- Ghost AI ---

static Direction ghost_choose_direction(Ghost *g, int target_x, int target_y) {
    // At each intersection, evaluate all valid directions (excluding reverse)
    // Pick the one that minimizes Euclidean distance to target
    // Tie-breaking: Up > Left > Down > Right
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

static void ghost_compute_target(Game *game, int ghost_idx) {
    Ghost *g = &game->ghosts[ghost_idx];
    PacMan *pm = &game->pacman;

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
        // Overflow bug: when facing up, also shift 4 left
        if (pm->dir == DIR_UP) ahead_x -= 4;
        g->target_x = ahead_x;
        g->target_y = ahead_y;
        break;
    }

    case GHOST_INKY: {
        int ref_x = pm->tile_x + dir_dx[pm->dir] * 2;
        int ref_y = pm->tile_y + dir_dy[pm->dir] * 2;
        if (pm->dir == DIR_UP) ref_x -= 2;
        Ghost *blinky = &game->ghosts[GHOST_BLINKY];
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

static void ghost_update_movement(Game *game, Ghost *g, float dt, int ghost_idx) {
    int tier = speed_tier(game->level);
    float spd;

    switch (g->mode) {
    case GHOST_IN_HOUSE:
        // Bob up and down inside the house
        g->house_bob_timer += dt * 3.0f;
        g->py = tile_center_px(g->home_tile_y) + sinf(g->house_bob_timer) * 4.0f;
        g->tile_y = g->home_tile_y;
        return;

    case GHOST_EXITING: {
        // Move to house center X, then up through door
        float target_px = tile_center_px(GHOST_HOUSE_CENTER_X);
        float target_py = tile_center_px(GHOST_HOUSE_EXIT_Y);
        float exit_speed = ghost_speed_normal[tier] * BASE_SPEED * TILE_SIZE * dt;

        float dx_f = target_px - g->px;
        float dy_f = target_py - g->py;

        // First align X to center
        if (fabsf(g->px - target_px) > 1.0f) {
            g->px += (dx_f > 0 ? 1 : -1) * exit_speed;
        } else {
            g->px = target_px;
            // Then move up
            if (fabsf(g->py - target_py) > 1.0f) {
                g->py -= exit_speed;
            } else {
                g->py = target_py;
                g->tile_x = GHOST_HOUSE_EXIT_X;
                g->tile_y = GHOST_HOUSE_EXIT_Y;
                g->px = tile_center_px(g->tile_x);
                g->py = tile_center_px(g->tile_y);
                g->dir = DIR_LEFT;
                g->mode = (game->global_mode == GLOBAL_CHASE) ? GHOST_CHASE : GHOST_SCATTER;
            }
        }
        g->tile_x = (int)(g->px / TILE_SIZE);
        g->tile_y = (int)(g->py / TILE_SIZE);
        return;
    }

    case GHOST_EATEN: {
        // Return to ghost house entrance at high speed
        float target_px = tile_center_px(GHOST_HOUSE_CENTER_X);
        float target_py = tile_center_px(GHOST_HOUSE_CENTER_Y);
        spd = ghost_speed_eaten * BASE_SPEED * TILE_SIZE * dt;

        float dx_f = target_px - g->px;
        float dy_f = target_py - g->py;
        float dist = sqrtf(dx_f * dx_f + dy_f * dy_f);

        if (dist < spd) {
            g->px = target_px;
            g->py = target_py;
            g->tile_x = GHOST_HOUSE_CENTER_X;
            g->tile_y = GHOST_HOUSE_CENTER_Y;
            g->mode = GHOST_EXITING;
            return;
        }

        // Use tile-based pathfinding toward the house entrance first
        float entrance_px = tile_center_px(GHOST_HOUSE_EXIT_X);
        float entrance_py = tile_center_px(GHOST_HOUSE_EXIT_Y);
        float de = sqrtf((entrance_px - g->px) * (entrance_px - g->px) +
                         (entrance_py - g->py) * (entrance_py - g->py));

        if (de < TILE_SIZE) {
            // Close to entrance, move directly into house
            g->px += (dx_f / dist) * spd;
            g->py += (dy_f / dist) * spd;
            g->tile_x = (int)(g->px / TILE_SIZE);
            g->tile_y = (int)(g->py / TILE_SIZE);
            return;
        }

        // Otherwise use normal pathfinding toward the entrance tile
        ghost_compute_target(game, ghost_idx);
        g->target_x = GHOST_HOUSE_EXIT_X;
        g->target_y = GHOST_HOUSE_EXIT_Y;
        break; // Fall through to normal tile movement below
    }

    case GHOST_FRIGHTENED:
        spd = ghost_speed_frightened[tier];
        break;
    default:
        spd = ghost_speed_normal[tier];
        break;
    }

    // Cruise Elroy speed boost for Blinky
    if (ghost_idx == GHOST_BLINKY && (g->mode == GHOST_CHASE || g->mode == GHOST_SCATTER)) {
        int et = elroy_tier(game->level);
        int e1 = elroy_thresholds[et][0];
        int e2 = elroy_thresholds[et][1];
        if (game->dots_remaining <= e2) {
            spd = ghost_speed_normal[tier] + 0.05f;
        } else if (game->dots_remaining <= e1) {
            spd = ghost_speed_normal[tier] + 0.025f;
        }
    }

    // Tunnel slowdown
    if (maze_is_tunnel(g->tile_x, g->tile_y) && g->mode != GHOST_EATEN) {
        spd = ghost_speed_tunnel[tier];
    }

    float move_speed = spd * BASE_SPEED * TILE_SIZE * dt;
    float center_x = tile_center_px(g->tile_x);
    float center_y = tile_center_px(g->tile_y);

    // Check if at tile center (decision point)
    bool at_center = fabsf(g->px - center_x) < 1.5f && fabsf(g->py - center_y) < 1.5f;

    if (at_center) {
        g->px = center_x;
        g->py = center_y;

        Direction new_dir;
        if (g->mode == GHOST_FRIGHTENED) {
            // Random direction at intersections (excluding reverse)
            Direction reverse = direction_opposite(g->dir);
            Direction candidates[4];
            int count = 0;
            Direction priority[] = { DIR_UP, DIR_LEFT, DIR_DOWN, DIR_RIGHT };
            for (int i = 0; i < 4; i++) {
                Direction d = priority[i];
                if (d == reverse) continue;
                int nx = g->tile_x + dir_dx[d];
                int ny = g->tile_y + dir_dy[d];
                if (maze_is_walkable_ghost(nx, ny, g->mode))
                    candidates[count++] = d;
            }
            if (count > 0)
                new_dir = candidates[GetRandomValue(0, count - 1)];
            else
                new_dir = reverse; // Dead end
        } else {
            ghost_compute_target(game, ghost_idx);
            new_dir = ghost_choose_direction(g, g->target_x, g->target_y);
        }

        if (new_dir != DIR_NONE)
            g->dir = new_dir;
    }

    // Move
    if (g->dir != DIR_NONE) {
        float new_px = g->px + dir_dx[g->dir] * move_speed;
        float new_py = g->py + dir_dy[g->dir] * move_speed;

        // Tunnel wrapping
        float maze_right = (float)(MAZE_COLS * TILE_SIZE);
        if (new_px < 0) new_px += maze_right;
        if (new_px >= maze_right) new_px -= maze_right;

        // Wall check
        int next_tx = g->tile_x + dir_dx[g->dir];
        int next_ty = g->tile_y + dir_dy[g->dir];
        bool next_blocked = !maze_is_walkable_ghost(next_tx, next_ty, g->mode);

        if (next_blocked) {
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

// --- Scatter/Chase timing ---

static void update_scatter_chase(Game *game, float dt) {
    if (game->frightened_active) return;

    game->scatter_chase_timer -= dt;
    if (game->scatter_chase_timer <= 0.0f) {
        game->scatter_chase_phase++;
        if (game->scatter_chase_phase >= SCATTER_CHASE_PHASES)
            game->scatter_chase_phase = SCATTER_CHASE_PHASES - 1;

        game->global_mode = (game->scatter_chase_phase % 2 == 0) ? GLOBAL_SCATTER : GLOBAL_CHASE;

        int tier = scatter_chase_tier(game->level);
        game->scatter_chase_timer = scatter_chase_times[game->scatter_chase_phase][tier];

        // Force reversal for all active ghosts
        for (int i = 0; i < GHOST_COUNT; i++) {
            Ghost *g = &game->ghosts[i];
            if (g->mode == GHOST_CHASE || g->mode == GHOST_SCATTER) {
                g->mode = (game->global_mode == GLOBAL_CHASE) ? GHOST_CHASE : GHOST_SCATTER;
                g->dir = direction_opposite(g->dir);
            }
        }

        // Cruise Elroy: Blinky stays in chase during scatter
        int et = elroy_tier(game->level);
        if (game->dots_remaining <= elroy_thresholds[et][0]) {
            game->ghosts[GHOST_BLINKY].mode = GHOST_CHASE;
        }
    }
}

// --- Frightened mode ---

static void activate_frightened(Game *game) {
    int idx = fright_level_index(game->level);
    float duration = fright_duration[idx];
    int flashes = fright_flashes[idx];

    game->ghost_eat_combo = 0;

    if (duration <= 0.0f) {
        // 0s duration: reverse but don't become frightened
        for (int i = 0; i < GHOST_COUNT; i++) {
            Ghost *g = &game->ghosts[i];
            if (g->mode == GHOST_CHASE || g->mode == GHOST_SCATTER)
                g->dir = direction_opposite(g->dir);
        }
        return;
    }

    game->frightened_active = true;
    game->frightened_timer = duration;
    game->frightened_duration = duration;
    game->frightened_flashes = flashes;

    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &game->ghosts[i];
        if (g->mode == GHOST_CHASE || g->mode == GHOST_SCATTER) {
            g->mode_before_fright = g->mode;
            g->mode = GHOST_FRIGHTENED;
            g->dir = direction_opposite(g->dir);
        }
    }
}

static void update_frightened(Game *game, float dt) {
    if (!game->frightened_active) return;

    game->frightened_timer -= dt;
    if (game->frightened_timer <= 0.0f) {
        game->frightened_active = false;
        for (int i = 0; i < GHOST_COUNT; i++) {
            Ghost *g = &game->ghosts[i];
            if (g->mode == GHOST_FRIGHTENED) {
                g->mode = (game->global_mode == GLOBAL_CHASE) ? GHOST_CHASE : GHOST_SCATTER;
            }
        }
    }
}

// --- Ghost house exit logic ---

static void update_ghost_house(Game *game) {
    for (int i = 1; i < GHOST_COUNT; i++) {
        Ghost *g = &game->ghosts[i];
        if (g->mode != GHOST_IN_HOUSE) continue;

        bool should_exit = false;

        if (game->use_global_dot_counter) {
            if (game->global_dot_counter >= global_dot_limits[i]) {
                should_exit = true;
                if (i == GHOST_CLYDE) {
                    game->use_global_dot_counter = false;
                }
            }
        } else {
            if (g->personal_dot_limit < 0 || g->personal_dot_counter >= g->personal_dot_limit)
                should_exit = true;
        }

        if (should_exit) {
            g->mode = GHOST_EXITING;
            game->no_dot_timer = 0.0f;
            break; // Only release one ghost at a time
        }
    }
}

static void ghost_house_timer_fallback(Game *game) {
    // Force release the next waiting ghost
    for (int i = 1; i < GHOST_COUNT; i++) {
        if (game->ghosts[i].mode == GHOST_IN_HOUSE) {
            game->ghosts[i].mode = GHOST_EXITING;
            game->no_dot_timer = 0.0f;
            break;
        }
    }
}

// --- Dot consumption ---

static void on_dot_eaten(Game *game) {
    game->dots_eaten_this_level++;
    game->no_dot_timer = 0.0f;

    // Update ghost house counters
    if (game->use_global_dot_counter) {
        game->global_dot_counter++;
    } else {
        for (int i = 1; i < GHOST_COUNT; i++) {
            if (game->ghosts[i].mode == GHOST_IN_HOUSE) {
                game->ghosts[i].personal_dot_counter++;
                break; // Only the first waiting ghost counts
            }
        }
    }
}

static void check_dot_consumption(Game *game) {
    PacMan *pm = &game->pacman;
    int tx = pm->tile_x;
    int ty = pm->tile_y;

    if (tx < 0 || tx >= MAZE_COLS || ty < 0 || ty >= MAZE_ROWS) return;
    if (game->dot_eaten[ty][tx]) return;

    int tile = maze_layout[ty][tx];
    int tier = speed_tier(game->level);

    if (tile == TILE_DOT) {
        game->dot_eaten[ty][tx] = true;
        game->score += 10;
        game->dots_remaining--;
        pm->eat_pause_frames = DOT_PAUSE_FRAMES;
        pm->speed_pct = pacman_speed_eating[tier];
        on_dot_eaten(game);
        play_sound(game, game->dot_sound_toggle ? SND_DOT_B : SND_DOT_A);
        game->dot_sound_toggle = !game->dot_sound_toggle;
        spawn_particles(game, MAZE_OFFSET_X + pm->px, MAZE_OFFSET_Y + pm->py, COLOR_DOT, 4);
    } else if (tile == TILE_POWER_PELLET) {
        game->dot_eaten[ty][tx] = true;
        game->score += 50;
        game->dots_remaining--;
        pm->eat_pause_frames = PELLET_PAUSE_FRAMES;
        on_dot_eaten(game);
        activate_frightened(game);
        play_sound(game, SND_POWER_PELLET);
        spawn_particles(game, MAZE_OFFSET_X + pm->px, MAZE_OFFSET_Y + pm->py, COLOR_PELLET, 12);
    }

    if (game->score > game->high_score)
        game->high_score = game->score;

    // Extra life at 10,000
    if (!game->extra_life_given && game->score >= 10000) {
        game->extra_life_given = true;
        game->lives++;
        play_sound(game, SND_EXTRA_LIFE);
    }
}

// --- Collision ---

static void check_ghost_collision(Game *game) {
    PacMan *pm = &game->pacman;

    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &game->ghosts[i];
        if (g->mode == GHOST_IN_HOUSE || g->mode == GHOST_EXITING || g->mode == GHOST_EATEN)
            continue;

        bool same_tile = (pm->tile_x == g->tile_x && pm->tile_y == g->tile_y);

        // Pass-through detection: check if they swapped tiles
        int pm_prev_tx = (int)((pm->px - dir_dx[pm->dir] * pm->speed_pct * BASE_SPEED * TILE_SIZE * GetFrameTime()) / TILE_SIZE);
        int pm_prev_ty = (int)((pm->py - dir_dy[pm->dir] * pm->speed_pct * BASE_SPEED * TILE_SIZE * GetFrameTime()) / TILE_SIZE);
        bool crossed = (pm_prev_tx == g->tile_x && pm_prev_ty == g->tile_y &&
                        g->tile_x == pm_prev_tx && g->tile_y == pm_prev_ty);

        if (same_tile || crossed) {
            if (g->mode == GHOST_FRIGHTENED) {
                // Eat the ghost
                game->ghost_eat_combo++;
                int points = 200;
                for (int c = 1; c < game->ghost_eat_combo; c++) points *= 2;
                game->score += points;
                if (game->score > game->high_score)
                    game->high_score = game->score;

                g->mode = GHOST_EATEN;
                play_sound(game, SND_GHOST_EATEN);

                game->ghost_eaten_pause = true;
                game->ghost_eaten_pause_timer = 1.0f;
                game->ghost_eaten_score_display = points;
                game->ghost_eaten_display_x = g->px;
                game->ghost_eaten_display_y = g->py;
                return;
            } else {
                // Pac-Man dies
                game->state = STATE_DYING;
                game->state_timer = 1.5f;
                game->pacman.death_timer = 0.0f;
                game->pacman.death_frame = 0;
                play_sound(game, SND_DEATH);
                return;
            }
        }
    }
}

// --- Game Update ---

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
            game->extra_life_given = false;
            game_reset_level(game);
            game->state = STATE_READY;
            game->state_timer = 2.0f;
            play_sound(game, SND_MENU_SELECT);
        }
        break;

    case STATE_READY:
        game->state_timer -= dt;
        if (game->state_timer <= 0.0f) {
            game->state = STATE_PLAYING;
            play_sound(game, SND_READY);
        }
        break;

    case STATE_PLAYING: {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) ||
            (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))) {
            game->state = STATE_PAUSED;
            break;
        }

        // Ghost eaten pause
        if (game->ghost_eaten_pause) {
            game->ghost_eaten_pause_timer -= dt;
            if (game->ghost_eaten_pause_timer <= 0.0f)
                game->ghost_eaten_pause = false;
            break;
        }

        pacman_update(game, dt);
        check_dot_consumption(game);

        // Pellet flashing
        game->pellet_flash_timer += dt;
        if (game->pellet_flash_timer >= 0.2f) {
            game->pellet_flash_timer -= 0.2f;
            game->pellet_visible = !game->pellet_visible;
        }

        // Scatter/chase and frightened timers
        update_scatter_chase(game, dt);
        update_frightened(game, dt);

        // Ghost house logic
        update_ghost_house(game);
        game->no_dot_timer += dt;
        if (game->no_dot_timer >= game->no_dot_timeout) {
            ghost_house_timer_fallback(game);
            game->no_dot_timer = 0.0f;
        }

        // Fruit spawning
        if (!game->fruit_spawned_70 && game->dots_eaten_this_level >= 70) {
            game->fruit_spawned_70 = true;
            game->fruit.active = true;
            game->fruit.fruit_type = fruit_type_for_level(game->level);
            game->fruit.points = fruit_points[game->fruit.fruit_type];
            game->fruit.timer = FRUIT_DURATION;
            game->fruit.score_display = false;
        }
        if (!game->fruit_spawned_170 && game->dots_eaten_this_level >= 170) {
            game->fruit_spawned_170 = true;
            game->fruit.active = true;
            game->fruit.fruit_type = fruit_type_for_level(game->level);
            game->fruit.points = fruit_points[game->fruit.fruit_type];
            game->fruit.timer = FRUIT_DURATION;
            game->fruit.score_display = false;
        }

        // Fruit timer
        if (game->fruit.active) {
            game->fruit.timer -= dt;
            if (game->fruit.timer <= 0.0f)
                game->fruit.active = false;
        }
        if (game->fruit.score_display) {
            game->fruit.score_display_timer -= dt;
            if (game->fruit.score_display_timer <= 0.0f)
                game->fruit.score_display = false;
        }

        // Fruit collection
        if (game->fruit.active &&
            game->pacman.tile_x == FRUIT_TILE_X && game->pacman.tile_y == FRUIT_TILE_Y) {
            game->score += game->fruit.points;
            if (game->score > game->high_score)
                game->high_score = game->score;
            game->fruit.score_display = true;
            game->fruit.score_display_timer = FRUIT_SCORE_DISPLAY_TIME;
            game->fruit.score_display_value = game->fruit.points;
            game->fruit.active = false;
            play_sound(game, SND_FRUIT);
            spawn_particles(game,
                MAZE_OFFSET_X + tile_center_px(FRUIT_TILE_X),
                MAZE_OFFSET_Y + tile_center_px(FRUIT_TILE_Y),
                fruit_colors[game->fruit.fruit_type], 8);
        }

        // Particles
        update_particles(game, dt);

        // Ghost movement
        for (int i = 0; i < GHOST_COUNT; i++)
            ghost_update_movement(game, &game->ghosts[i], dt, i);

        // Collision
        check_ghost_collision(game);

        // Level complete
        if (game->dots_remaining <= 0) {
            game->state = STATE_LEVEL_COMPLETE;
            game->state_timer = 2.0f;
        }
        break;
    }

    case STATE_DYING:
        game->state_timer -= dt;
        game->pacman.death_timer += dt;
        game->pacman.death_frame = (int)(game->pacman.death_timer / (1.5f / 11.0f));
        if (game->pacman.death_frame > 10) game->pacman.death_frame = 10;

        if (game->state_timer <= 0.0f) {
            game->lives--;
            if (game->lives > 0) {
                game_reset_positions(game);
                game->state = STATE_READY;
                game->state_timer = 2.0f;
            } else {
                highscore_save(game);
                game->state = STATE_GAME_OVER;
                game->state_timer = 3.0f;
            }
        }
        break;

    case STATE_LEVEL_COMPLETE:
        game->state_timer -= dt;
        game->level_complete_flash_white = ((int)(game->state_timer * 4.0f)) % 2 == 0;
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
            (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)))
            game->state = STATE_PLAYING;
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
    Color wall_color = game->level_complete_flash_white ? COLOR_WALL_WHITE : COLOR_WALL;
    Color glow_color = game->level_complete_flash_white ?
        (Color){255, 255, 255, 80} : COLOR_WALL_GLOW;

    for (int r = 0; r < MAZE_ROWS; r++) {
        for (int c = 0; c < MAZE_COLS; c++) {
            int t = maze_layout[r][c];
            float x = MAZE_OFFSET_X + c * TILE_SIZE;
            float y = MAZE_OFFSET_Y + r * TILE_SIZE;

            if (t == TILE_WALL) {
                DrawRectangle((int)x, (int)y, TILE_SIZE, TILE_SIZE, (Color){15, 15, 40, 255});

                float half = line_thick / 2.0f;
                if (!is_wall(c, r - 1)) {
                    DrawRectangle((int)x, (int)(y - half), TILE_SIZE, (int)line_thick, wall_color);
                    DrawRectangle((int)x, (int)(y - half - 2), TILE_SIZE, (int)(line_thick + 4), glow_color);
                }
                if (!is_wall(c, r + 1)) {
                    DrawRectangle((int)x, (int)(y + TILE_SIZE - half), TILE_SIZE, (int)line_thick, wall_color);
                    DrawRectangle((int)x, (int)(y + TILE_SIZE - half - 2), TILE_SIZE, (int)(line_thick + 4), glow_color);
                }
                if (!is_wall(c - 1, r)) {
                    DrawRectangle((int)(x - half), (int)y, (int)line_thick, TILE_SIZE, wall_color);
                    DrawRectangle((int)(x - half - 2), (int)y, (int)(line_thick + 4), TILE_SIZE, glow_color);
                }
                if (!is_wall(c + 1, r)) {
                    DrawRectangle((int)(x + TILE_SIZE - half), (int)y, (int)line_thick, TILE_SIZE, wall_color);
                    DrawRectangle((int)(x + TILE_SIZE - half - 2), (int)y, (int)(line_thick + 4), TILE_SIZE, glow_color);
                }
                if (!is_wall(c, r - 1) && !is_wall(c - 1, r))
                    DrawCircle((int)x, (int)y, line_thick + 1, wall_color);
                if (!is_wall(c, r - 1) && !is_wall(c + 1, r))
                    DrawCircle((int)(x + TILE_SIZE), (int)y, line_thick + 1, wall_color);
                if (!is_wall(c, r + 1) && !is_wall(c - 1, r))
                    DrawCircle((int)x, (int)(y + TILE_SIZE), line_thick + 1, wall_color);
                if (!is_wall(c, r + 1) && !is_wall(c + 1, r))
                    DrawCircle((int)(x + TILE_SIZE), (int)(y + TILE_SIZE), line_thick + 1, wall_color);
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
                DrawCircle((int)cx, (int)cy, 10.0f, (Color){255, 255, 255, 40});
                DrawCircle((int)cx, (int)cy, 7.0f, (Color){255, 255, 255, 80});
                DrawCircle((int)cx, (int)cy, 5.0f, COLOR_PELLET);
            }
        }
    }
}

static void draw_pacman(Game *game) {
    PacMan *pm = &game->pacman;

    if (game->state == STATE_DYING) {
        // Death animation: open wide then collapse
        float cx = MAZE_OFFSET_X + pm->px;
        float cy = MAZE_OFFSET_Y + pm->py;
        float radius = TILE_SIZE / 2.0f - 1.0f;
        float progress = pm->death_frame / 10.0f;
        float mouth = 45.0f + progress * 135.0f; // Opens to 180 degrees
        float shrink = 1.0f - progress * 0.8f;
        if (shrink < 0.1f) shrink = 0.1f;

        float dir_angle = 270.0f; // Death always faces up
        float start_angle = dir_angle + mouth;
        float end_angle = dir_angle + 360.0f - mouth;

        if (end_angle <= start_angle) return;
        DrawCircleSector((Vector2){cx, cy}, radius * shrink, start_angle, end_angle, 32, COLOR_PACMAN);
        return;
    }

    float cx = MAZE_OFFSET_X + pm->px;
    float cy = MAZE_OFFSET_Y + pm->py;
    float radius = TILE_SIZE / 2.0f - 1.0f;

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

static void draw_ghost(Game *game, Ghost *g) {
    float cx = MAZE_OFFSET_X + g->px;
    float cy = MAZE_OFFSET_Y + g->py;
    float r = TILE_SIZE / 2.0f - 1.0f;
    Color body_color;

    if (g->mode == GHOST_EATEN) {
        // Eyes only
        float eye_offset = 3.0f;
        float eye_r = 3.0f;
        float pupil_r = 1.5f;

        float edx = 0, edy = 0;
        switch (g->dir) {
            case DIR_UP:    edy = -2; break;
            case DIR_DOWN:  edy = 2; break;
            case DIR_LEFT:  edx = -2; break;
            case DIR_RIGHT: edx = 2; break;
            default: break;
        }

        DrawCircle((int)(cx - eye_offset), (int)(cy - 2), eye_r, WHITE);
        DrawCircle((int)(cx + eye_offset), (int)(cy - 2), eye_r, WHITE);
        DrawCircle((int)(cx - eye_offset + edx), (int)(cy - 2 + edy), pupil_r, (Color){33, 33, 222, 255});
        DrawCircle((int)(cx + eye_offset + edx), (int)(cy - 2 + edy), pupil_r, (Color){33, 33, 222, 255});
        return;
    }

    if (g->mode == GHOST_FRIGHTENED) {
        // Flashing: check if we should show white
        bool flash_white = false;
        if (game->frightened_timer < 2.0f && game->frightened_timer > 0.0f) {
            float flash_cycle = 0.28f;
            float in_cycle = fmodf(game->frightened_timer, flash_cycle);
            flash_white = in_cycle < 0.14f;
        }
        body_color = flash_white ? COLOR_FRIGHT_FLASH : COLOR_FRIGHTENED;
    } else {
        body_color = g->color;
    }

    // Ghost body: semicircle top + rectangle bottom + wavy bottom
    DrawCircle((int)cx, (int)(cy - 2), r, body_color);
    DrawRectangle((int)(cx - r), (int)(cy - 2), (int)(r * 2), (int)(r + 2), body_color);

    // Wavy bottom
    for (int w = 0; w < 3; w++) {
        float wx = cx - r + (r * 2.0f / 3.0f) * w + (r / 3.0f);
        float wy = cy + r;
        DrawCircle((int)wx, (int)wy, r / 3.0f, body_color);
    }

    if (g->mode == GHOST_FRIGHTENED) {
        // Frightened face: simple dots for eyes, wavy mouth
        Color face_color = (body_color.r == 255) ? COLOR_FRIGHTENED : WHITE;
        DrawCircle((int)(cx - 3), (int)(cy - 3), 2.0f, face_color);
        DrawCircle((int)(cx + 3), (int)(cy - 3), 2.0f, face_color);
        // Wavy mouth
        for (int m = -4; m <= 4; m += 2) {
            float my = (m % 4 == 0) ? cy + 3 : cy + 1;
            DrawCircle((int)(cx + m), (int)my, 1.0f, face_color);
        }
    } else {
        // Normal eyes
        float eye_offset = 3.0f;
        float eye_r = 3.5f;
        float pupil_r = 2.0f;

        float edx = 0, edy = 0;
        switch (g->dir) {
            case DIR_UP:    edy = -1.5f; break;
            case DIR_DOWN:  edy = 1.5f; break;
            case DIR_LEFT:  edx = -1.5f; break;
            case DIR_RIGHT: edx = 1.5f; break;
            default: break;
        }

        DrawCircle((int)(cx - eye_offset), (int)(cy - 3), eye_r, WHITE);
        DrawCircle((int)(cx + eye_offset), (int)(cy - 3), eye_r, WHITE);
        DrawCircle((int)(cx - eye_offset + edx), (int)(cy - 3 + edy), pupil_r, (Color){33, 33, 222, 255});
        DrawCircle((int)(cx + eye_offset + edx), (int)(cy - 3 + edy), pupil_r, (Color){33, 33, 222, 255});
    }
}

static void draw_ghosts(Game *game) {
    for (int i = 0; i < GHOST_COUNT; i++)
        draw_ghost(game, &game->ghosts[i]);
}

static void draw_fruit(Game *game) {
    float cx = MAZE_OFFSET_X + tile_center_px(FRUIT_TILE_X);
    float cy = MAZE_OFFSET_Y + tile_center_px(FRUIT_TILE_Y);

    if (game->fruit.active) {
        Color c = fruit_colors[game->fruit.fruit_type];
        int ft = game->fruit.fruit_type;

        switch (ft) {
        case 0: // Cherry: two circles with stem
            DrawCircle((int)(cx - 3), (int)(cy + 2), 5, c);
            DrawCircle((int)(cx + 3), (int)(cy + 2), 5, c);
            DrawLineEx((Vector2){cx - 2, cy - 3}, (Vector2){cx + 1, cy - 7}, 2, (Color){0, 150, 0, 255});
            DrawLineEx((Vector2){cx + 2, cy - 3}, (Vector2){cx + 1, cy - 7}, 2, (Color){0, 150, 0, 255});
            break;
        case 1: // Strawberry: triangle-ish
            DrawCircle((int)cx, (int)(cy + 1), 7, c);
            DrawTriangle((Vector2){cx - 6, cy - 2}, (Vector2){cx + 6, cy - 2}, (Vector2){cx, cy + 8}, c);
            DrawCircle((int)cx, (int)(cy - 5), 4, (Color){0, 180, 0, 255});
            break;
        case 2: // Orange: circle
            DrawCircle((int)cx, (int)cy, 8, c);
            DrawCircle((int)(cx + 1), (int)(cy - 6), 3, (Color){0, 180, 0, 255});
            break;
        case 3: // Apple: circle with leaf
            DrawCircle((int)cx, (int)cy, 8, c);
            DrawCircle((int)(cx + 2), (int)(cy - 7), 3, (Color){0, 120, 0, 255});
            DrawLineEx((Vector2){cx, cy - 9}, (Vector2){cx, cy - 4}, 2, (Color){100, 60, 20, 255});
            break;
        case 4: // Melon: oval
            DrawEllipse((int)cx, (int)cy, 9, 7, c);
            for (int s = -6; s <= 6; s += 4)
                DrawLineEx((Vector2){cx + s, cy - 6}, (Vector2){cx + s, cy + 6}, 1, (Color){50, 120, 20, 255});
            break;
        case 5: // Galaxian: flag shape
            DrawTriangle((Vector2){cx, cy - 8}, (Vector2){cx - 6, cy + 4}, (Vector2){cx + 6, cy + 4}, c);
            DrawRectangle((int)(cx - 1), (int)(cy + 4), 3, 5, (Color){200, 200, 0, 255});
            break;
        case 6: // Bell
            DrawCircle((int)cx, (int)(cy + 3), 7, c);
            DrawRectangle((int)(cx - 5), (int)(cy - 5), 10, 8, c);
            DrawCircle((int)cx, (int)(cy - 5), 5, c);
            DrawCircle((int)cx, (int)(cy + 9), 2, (Color){150, 120, 0, 255});
            break;
        case 7: // Key
            DrawCircle((int)cx, (int)(cy - 4), 5, c);
            DrawCircle((int)cx, (int)(cy - 4), 3, COLOR_BG);
            DrawRectangle((int)(cx - 1), (int)(cy), 3, 10, c);
            DrawRectangle((int)(cx + 1), (int)(cy + 4), 4, 2, c);
            DrawRectangle((int)(cx + 1), (int)(cy + 7), 3, 2, c);
            break;
        }
    }

    // Score popup
    if (game->fruit.score_display) {
        char pts[16];
        snprintf(pts, sizeof(pts), "%d", game->fruit.score_display_value);
        int pw = MeasureText(pts, 16);
        DrawText(pts, (int)(cx - pw / 2), (int)(cy - 8), 16, COLOR_PELLET);
    }
}

static void draw_hud(Game *game) {
    char score_text[32];
    snprintf(score_text, sizeof(score_text), "SCORE: %d", game->score);
    DrawText(score_text, MAZE_OFFSET_X, MAZE_OFFSET_Y + 4, 20, COLOR_TEXT);

    char hi_text[32];
    snprintf(hi_text, sizeof(hi_text), "HIGH SCORE: %d", game->high_score);
    int hi_width = MeasureText(hi_text, 20);
    DrawText(hi_text, WINDOW_WIDTH / 2 - hi_width / 2, MAZE_OFFSET_Y + 4, 20, COLOR_TEXT);

    char level_text[32];
    snprintf(level_text, sizeof(level_text), "LEVEL: %d", game->level);
    int level_width = MeasureText(level_text, 20);
    DrawText(level_text, MAZE_OFFSET_X + MAZE_WIDTH - level_width, MAZE_OFFSET_Y + 4, 20, COLOR_TEXT);

    for (int i = 0; i < game->lives; i++) {
        float lx = MAZE_OFFSET_X + 20 + i * 30;
        float ly = MAZE_OFFSET_Y + MAZE_HEIGHT - 30;
        DrawCircleSector((Vector2){lx, ly}, 10.0f, 30.0f, 330.0f, 16, COLOR_PACMAN);
    }
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
        draw_fruit(game);
        draw_ghosts(game);
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
        draw_fruit(game);
        draw_ghosts(game);
        draw_pacman(game);
        draw_particles(game);
        draw_hud(game);

        // Frightened vignette
        if (game->frightened_active) {
            DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                         (Color){0, 0, 80, (unsigned char)(30 + 20 * sinf(game->frightened_timer * 4.0f))});
        }

        // Ghost eaten score popup
        if (game->ghost_eaten_pause) {
            char pts[16];
            snprintf(pts, sizeof(pts), "%d", game->ghost_eaten_score_display);
            int pw = MeasureText(pts, 16);
            DrawText(pts,
                     (int)(MAZE_OFFSET_X + game->ghost_eaten_display_x - pw / 2),
                     (int)(MAZE_OFFSET_Y + game->ghost_eaten_display_y - 8),
                     16, (Color){0, 255, 255, 255});
        }
        break;

    case STATE_DYING:
        draw_maze(game);
        draw_dots(game);
        draw_fruit(game);
        draw_pacman(game);
        draw_particles(game);
        draw_hud(game);
        break;

    case STATE_LEVEL_COMPLETE:
        draw_maze(game);
        draw_hud(game);
        break;

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
        draw_ghosts(game);
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
