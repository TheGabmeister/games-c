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
