#include "world.h"

#include "raylib.h"
#include <stdio.h>
#include <string.h>

static const char *builtin_level[GRID_ROWS] = {
    "..............E...............",
    "..............E...............",
    "..............E...............",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    "..............................",
    ".P....$.....$.....$...........",
    "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSS",
};

static void set_error(char *error, int error_size, const char *message) {
    if (error != NULL && error_size > 0) {
        snprintf(error, (size_t)error_size, "%s", message);
    }
}

bool world_tile_is_climbable(TileID tile, bool exit_revealed) {
    return tile == TILE_LADDER || (tile == TILE_EXIT_LADDER && exit_revealed);
}

bool world_has_support(const World *world, int r, int c) {
    return world_tile_is_support(world_tile_at(world, r + 1, c), world->all_gold_collected);
}

bool world_can_enter(const World *world, int r, int c) {
    return world_in_bounds(r, c) && world_tile_is_passable(world_tile_at(world, r, c), world->all_gold_collected);
}

bool world_can_step_side(const World *world, int r, int c) {
    TileID tile = world_tile_at(world, r, c);
    return world_can_enter(world, r, c) &&
           (world_tile_is_climbable(tile, world->all_gold_collected) ||
            tile == TILE_ROPE ||
            world_has_support(world, r, c));
}

static bool actor_can_move_between(const World *world, int from_r, int from_c, int to_r, int to_c) {
    if (!world_in_bounds(from_r, from_c) || !world_can_enter(world, to_r, to_c)) {
        return false;
    }

    int dr = to_r - from_r;
    int dc = to_c - from_c;
    TileID from_tile = world_tile_at(world, from_r, from_c);
    bool on_climb = world_tile_is_climbable(from_tile, world->all_gold_collected);
    bool on_rope = from_tile == TILE_ROPE;
    bool falling = !on_climb && !on_rope && !world_has_support(world, from_r, from_c);

    if (falling) {
        return dr == 1 && dc == 0;
    }
    if (on_climb && dc == 0 && (dr == -1 || dr == 1)) {
        return true;
    }
    if (on_rope && dr == 1 && dc == 0) {
        return true;
    }
    if (dr == 0 && (dc == -1 || dc == 1)) {
        if (on_climb) {
            return world_can_step_side(world, to_r, to_c);
        }
        return world_can_enter(world, to_r, to_c);
    }
    return false;
}

static PursuitDir dir_from_delta(int dr, int dc) {
    if (dc < 0) return PURSUE_LEFT;
    if (dc > 0) return PURSUE_RIGHT;
    if (dr < 0) return PURSUE_UP;
    if (dr > 0) return PURSUE_DOWN;
    return PURSUE_NONE;
}

static TileID tile_from_char(char ch, bool *is_player, bool *is_guard, bool *ok) {
    *is_player = false;
    *is_guard = false;
    *ok = true;

    switch (ch) {
        case '.': return TILE_EMPTY;
        case 'B': return TILE_BRICK;
        case 'S': return TILE_SOLID;
        case 'H': return TILE_LADDER;
        case '-': return TILE_ROPE;
        case '$': return TILE_GOLD;
        case 'E': return TILE_EXIT_LADDER;
        case 'T': return TILE_TRAPDOOR;
        case 'P':
            *is_player = true;
            return TILE_EMPTY;
        case 'G':
            *is_guard = true;
            return TILE_EMPTY;
        default:
            *ok = false;
            return TILE_EMPTY;
    }
}

static bool parse_lines(World *world, const char *lines[GRID_ROWS], int level_index, char *error, int error_size) {
    int player_count = 0;
    int exit_top_count = 0;

    world_clear(world);

    for (int r = 0; r < GRID_ROWS; r++) {
        if ((int)strlen(lines[r]) != GRID_COLS) {
            char message[96];
            snprintf(message, sizeof(message), "row %d must be exactly %d chars", r + 1, GRID_COLS);
            set_error(error, error_size, message);
            return false;
        }

        for (int c = 0; c < GRID_COLS; c++) {
            bool is_player = false;
            bool is_guard = false;
            bool ok = false;
            TileID tile = tile_from_char(lines[r][c], &is_player, &is_guard, &ok);

            if (!ok) {
                char message[96];
                snprintf(message, sizeof(message), "invalid tile '%c' at row %d col %d", lines[r][c], r + 1, c + 1);
                set_error(error, error_size, message);
                return false;
            }

            world->tiles[r][c] = tile;

            if (tile == TILE_GOLD) {
                world->gold_total++;
                world->gold_remaining++;
            } else if (tile == TILE_EXIT_LADDER && r <= 2) {
                exit_top_count++;
            }

            if (is_player) {
                player_count++;
                world->player_spawn_r = r;
                world->player_spawn_c = c;
            } else if (is_guard) {
                if (world->guard_spawn_count >= MAX_GUARD_SPAWNS) {
                    set_error(error, error_size, "too many guard spawns");
                    return false;
                }
                world->guard_spawn_r[world->guard_spawn_count] = r;
                world->guard_spawn_c[world->guard_spawn_count] = c;
                world->guard_spawn_count++;
            }
        }
    }

    if (player_count != 1) {
        set_error(error, error_size, "level must contain exactly one P spawn");
        return false;
    }
    if (world->guard_spawn_count > MAX_GUARDS) {
        set_error(error, error_size, "guard spawn count exceeds MAX_GUARDS");
        return false;
    }
    if (world->gold_total <= 0 && level_index != 0) {
        set_error(error, error_size, "level must contain at least one gold tile");
        return false;
    }
    if (exit_top_count <= 0) {
        set_error(error, error_size, "level needs an E tile in rows 1 through 3");
        return false;
    }

    world->guard_count = world->guard_spawn_count;
    return true;
}

static bool read_level_file(const char *path, const char *lines[GRID_ROWS], char storage[GRID_ROWS][GRID_COLS + 1], char *error, int error_size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        set_error(error, error_size, "level file not found");
        return false;
    }

    char buffer[128];
    for (int r = 0; r < GRID_ROWS; r++) {
        if (fgets(buffer, sizeof(buffer), file) == NULL) {
            fclose(file);
            set_error(error, error_size, "level ended before row 22");
            return false;
        }

        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
            buffer[--len] = '\0';
        }

        if (len != GRID_COLS) {
            fclose(file);
            char message[96];
            snprintf(message, sizeof(message), "row %d must be exactly %d chars", r + 1, GRID_COLS);
            set_error(error, error_size, message);
            return false;
        }

        memcpy(storage[r], buffer, GRID_COLS + 1);
        lines[r] = storage[r];
    }

    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        fclose(file);
        set_error(error, error_size, "level has more than 22 rows");
        return false;
    }

    fclose(file);
    return true;
}

void world_clear(World *world) {
    memset(world, 0, sizeof(*world));
    world->player_spawn_r = 0;
    world->player_spawn_c = 0;

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            world->tiles[r][c] = TILE_EMPTY;
        }
    }
}

bool world_load_level(World *world, int level_index, char *error, int error_size) {
    char path[64];
    char storage[GRID_ROWS][GRID_COLS + 1];
    const char *lines[GRID_ROWS];
    char file_error[128] = { 0 };

    if (level_index < 0 || level_index >= MAX_LEVELS) {
        set_error(error, error_size, "level index out of range");
        world_load_builtin(world);
        return false;
    }

    snprintf(path, sizeof(path), "assets/levels/level%02d.txt", level_index + 1);
    if (!read_level_file(path, lines, storage, file_error, sizeof(file_error))) {
        snprintf(path, sizeof(path), "src/assets/levels/level%02d.txt", level_index + 1);
        if (!read_level_file(path, lines, storage, file_error, sizeof(file_error))) {
            char message[160];
            snprintf(message, sizeof(message), "level%02d.txt: %s; using built-in fallback", level_index + 1, file_error);
            TraceLog(LOG_WARNING, "%s", message);
            set_error(error, error_size, message);
            world_load_builtin(world);
            return false;
        }
    }

    if (!parse_lines(world, lines, level_index, file_error, sizeof(file_error))) {
        char message[160];
        snprintf(message, sizeof(message), "level%02d.txt: %s; using built-in fallback", level_index + 1, file_error);
        TraceLog(LOG_WARNING, "%s", message);
        set_error(error, error_size, message);
        world_load_builtin(world);
        return false;
    }

    set_error(error, error_size, "");
    return true;
}

void world_load_builtin(World *world) {
    char error[128];
    if (!parse_lines(world, builtin_level, 0, error, sizeof(error))) {
        TraceLog(LOG_ERROR, "Built-in level is invalid: %s", error);
        world_clear(world);
    }
}

WorldTickResult world_update_holes(World *world, float dt, int trap_r, int trap_c) {
    WorldTickResult result = { 0 };

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (world->tiles[r][c] != TILE_HOLE) {
                continue;
            }

            world->hole_timer[r][c] -= dt;
            if (world->hole_timer[r][c] <= 0.0f) {
                world->tiles[r][c] = TILE_BRICK;
                world->hole_timer[r][c] = 0.0f;
                result.refilled_hole = true;
                result.refilled_tiles[r][c] = true;

                if (r == trap_r && c == trap_c) {
                    result.trapped_target = true;
                }
            }
        }
    }

    return result;
}

void world_rebuild_pursuit(const World *world, int player_r, int player_c, PursuitDir pursuit[GRID_ROWS][GRID_COLS]) {
    int queue_r[GRID_ROWS * GRID_COLS];
    int queue_c[GRID_ROWS * GRID_COLS];
    int head = 0;
    int tail = 0;
    bool visited[GRID_ROWS][GRID_COLS] = { 0 };

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            pursuit[r][c] = PURSUE_NONE;
        }
    }

    if (!world_in_bounds(player_r, player_c)) {
        return;
    }

    visited[player_r][player_c] = true;
    queue_r[tail] = player_r;
    queue_c[tail] = player_c;
    tail++;

    while (head < tail) {
        int r = queue_r[head];
        int c = queue_c[head];
        head++;

        const int drs[4] = { 0, 0, -1, 1 };
        const int dcs[4] = { -1, 1, 0, 0 };

        for (int i = 0; i < 4; i++) {
            int nr = r + drs[i];
            int nc = c + dcs[i];
            if (!world_in_bounds(nr, nc) || visited[nr][nc]) {
                continue;
            }

            if (!actor_can_move_between(world, nr, nc, r, c)) {
                continue;
            }

            visited[nr][nc] = true;
            pursuit[nr][nc] = dir_from_delta(r - nr, c - nc);
            queue_r[tail] = nr;
            queue_c[tail] = nc;
            tail++;
        }
    }
}

bool world_in_bounds(int r, int c) {
    return r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS;
}

TileID world_tile_at(const World *world, int r, int c) {
    if (!world_in_bounds(r, c)) {
        return TILE_SOLID;
    }
    return world->tiles[r][c];
}

bool world_tile_is_support(TileID tile, bool exit_revealed) {
    switch (tile) {
        case TILE_BRICK:
        case TILE_SOLID:
        case TILE_LADDER:
            return true;
        case TILE_EXIT_LADDER:
            return exit_revealed;
        default:
            return false;
    }
}

bool world_tile_is_passable(TileID tile, bool exit_revealed) {
    switch (tile) {
        case TILE_EMPTY:
        case TILE_HOLE:
        case TILE_LADDER:
        case TILE_ROPE:
        case TILE_GOLD:
        case TILE_TRAPDOOR:
            return true;
        case TILE_EXIT_LADDER:
            (void)exit_revealed;
            return true;
        default:
            return false;
    }
}

const char *world_tile_name(TileID tile) {
    switch (tile) {
        case TILE_EMPTY: return "EMPTY";
        case TILE_BRICK: return "BRICK";
        case TILE_HOLE: return "HOLE";
        case TILE_SOLID: return "SOLID";
        case TILE_LADDER: return "LADDER";
        case TILE_ROPE: return "ROPE";
        case TILE_GOLD: return "GOLD";
        case TILE_EXIT_LADDER: return "EXIT";
        case TILE_TRAPDOOR: return "TRAPDOOR";
        default: return "UNKNOWN";
    }
}

const char *world_pursuit_dir_name(PursuitDir dir) {
    switch (dir) {
        case PURSUE_LEFT: return "LEFT";
        case PURSUE_RIGHT: return "RIGHT";
        case PURSUE_UP: return "UP";
        case PURSUE_DOWN: return "DOWN";
        case PURSUE_NONE: return "NONE";
        default: return "UNKNOWN";
    }
}
