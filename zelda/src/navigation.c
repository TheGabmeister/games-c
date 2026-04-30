#include "navigation.h"
#include "game.h"
#include "sounds.h"
#include "vfx.h"
#include "world_interact.h"
#include <stdio.h>
#include <string.h>

static void apply_overworld_flags_to_screen(const Game *game, Screen *screen, int sx, int sy) {
    int index = sy * OVERWORLD_COLS + sx;
    if (world_flag_is_set(game->world.bombed_walls, index)) {
        for (int r = 0; r < SCREEN_TILES_Y; r++) {
            for (int c = 0; c < SCREEN_TILES_X; c++) {
                if ((TileType)screen->tiles[r][c] == TILE_BOMBABLE_WALL)
                    screen->tiles[r][c] = TILE_FLOOR;
            }
        }
    }
    if (world_flag_is_set(game->world.burned_bushes, index)) {
        for (int r = 0; r < SCREEN_TILES_Y; r++) {
            for (int c = 0; c < SCREEN_TILES_X; c++) {
                if ((TileType)screen->tiles[r][c] == TILE_BUSH)
                    screen->tiles[r][c] = TILE_FLOOR;
            }
        }
    }
}

void nav_load_screen(Game *game, int sx, int sy) {
    char path[SCREEN_PATH_MAX];
    snprintf(path, sizeof(path), "assets/screens/%02d_%02d.txt", sx, sy);
    if (!screen_load(&game->current_screen, path)) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
    apply_overworld_flags_to_screen(game, &game->current_screen, sx, sy);
    game->screen_x = sx;
    game->screen_y = sy;
    game->candle_used_this_screen = false;
    enemies_spawn(game->enemies, &game->enemy_count, &game->current_screen);
    projectiles_clear(game->projectiles, &game->projectile_count);
    pickups_clear(game->pickups, &game->pickup_count);
    vfx_clear();
}

void nav_load_dungeon_room(Game *game, int rx, int ry) {
    char path[64];
    snprintf(path, sizeof(path), "assets/dungeons/%d/%02d_%02d.txt",
             game->dungeon.id, rx, ry);
    if (!screen_load(&game->current_screen, path)) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
    game->dungeon.room_x = rx;
    game->dungeon.room_y = ry;
    game->candle_used_this_screen = false;
    game->dungeon.rooms_visited |= dungeon_room_bit(rx, ry);

    uint64_t rbit = dungeon_room_bit(rx, ry);

    for (int i = 0; i < game->current_screen.door_count; i++) {
        DoorMeta *d = &game->current_screen.doors[i];
        if (!d->active) continue;

        bool should_block = false;
        if (d->type == DOOR_LOCKED) {
            if (!dungeon_door_is_unlocked(&game->dungeon, rx, ry, d->side))
                should_block = true;
        } else if (d->type == DOOR_SHUTTER) {
            if (!(game->dungeon.shutter_opened & rbit))
                should_block = true;
        }

        if (should_block) {
            if (d->side == DIR_N) {
                game->current_screen.tiles[0][d->position] = TILE_DOOR_CLOSED;
                game->current_screen.tiles[0][d->position + 1] = TILE_DOOR_CLOSED;
            } else if (d->side == DIR_S) {
                game->current_screen.tiles[SCREEN_TILES_Y - 1][d->position] = TILE_DOOR_CLOSED;
                game->current_screen.tiles[SCREEN_TILES_Y - 1][d->position + 1] = TILE_DOOR_CLOSED;
            } else if (d->side == DIR_W) {
                game->current_screen.tiles[d->position][0] = TILE_DOOR_CLOSED;
                game->current_screen.tiles[d->position + 1][0] = TILE_DOOR_CLOSED;
            } else if (d->side == DIR_E) {
                game->current_screen.tiles[d->position][SCREEN_TILES_X - 1] = TILE_DOOR_CLOSED;
                game->current_screen.tiles[d->position + 1][SCREEN_TILES_X - 1] = TILE_DOOR_CLOSED;
            }
        }
    }

    bool skip_enemies = false;
    if (game->current_screen.is_boss_room && game->dungeon.boss_defeated)
        skip_enemies = true;
    if ((game->dungeon.rooms_cleared & rbit) && !game->current_screen.is_shutter)
        skip_enemies = true;
    if ((game->dungeon.shutter_opened & rbit) && game->current_screen.is_shutter)
        skip_enemies = true;

    if (skip_enemies) {
        game->enemy_count = 0;
    } else {
        enemies_spawn(game->enemies, &game->enemy_count, &game->current_screen);
    }

    projectiles_clear(game->projectiles, &game->projectile_count);
    pickups_clear(game->pickups, &game->pickup_count);
    vfx_clear();

    for (int i = 0; i < game->current_screen.item_count; i++) {
        ItemPlacement *ip = &game->current_screen.items[i];
        if (!ip->active) continue;
        int item_bit = i;
        int room_idx = ry * DUNGEON_MAX_COLS + rx;
        uint64_t collected_bit = 1ULL << (room_idx % 16 * 4 + item_bit);
        if (game->dungeon.items_collected & collected_bit) {
            ip->active = false;
        }
    }
}

void nav_load_cave(Game *game, const char *cave_name) {
    char path[SCREEN_PATH_MAX];
    snprintf(path, sizeof(path), "assets/caves/%s.txt", cave_name);
    if (!screen_load(&game->current_screen, path)) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
    game->candle_used_this_screen = false;
    enemies_spawn(game->enemies, &game->enemy_count, &game->current_screen);
    projectiles_clear(game->projectiles, &game->projectile_count);
    pickups_clear(game->pickups, &game->pickup_count);
    vfx_clear();
}

static bool can_transition_overworld(int screen_x, int screen_y, Direction dir) {
    int nx = screen_x, ny = screen_y;
    switch (dir) {
        case DIR_N: ny--; break;
        case DIR_S: ny++; break;
        case DIR_W: nx--; break;
        case DIR_E: nx++; break;
        default: return false;
    }
    if (nx < 0 || nx >= OVERWORLD_COLS || ny < 0 || ny >= OVERWORLD_ROWS)
        return false;
    return screen_file_exists(nx, ny);
}

static bool can_transition_dungeon(const Game *game, Direction dir) {
    int nx = game->dungeon.room_x, ny = game->dungeon.room_y;
    switch (dir) {
        case DIR_N: ny++; break;
        case DIR_S: ny--; break;
        case DIR_W: nx--; break;
        case DIR_E: nx++; break;
        default: return false;
    }
    if (nx < 0 || nx >= DUNGEON_MAX_COLS || ny < 0 || ny >= DUNGEON_MAX_ROWS)
        return false;
    return dungeon_room_file_exists(game->dungeon.id, nx, ny);
}

static void start_scroll_transition(Game *game, Direction dir) {
    int nx, ny;
    if (game->in_dungeon) {
        nx = game->dungeon.room_x;
        ny = game->dungeon.room_y;
    } else {
        nx = game->screen_x;
        ny = game->screen_y;
    }
    if (game->in_dungeon) {
        switch (dir) {
            case DIR_N: ny++; break;
            case DIR_S: ny--; break;
            case DIR_W: nx--; break;
            case DIR_E: nx++; break;
            default: return;
        }
    } else {
        switch (dir) {
            case DIR_N: ny--; break;
            case DIR_S: ny++; break;
            case DIR_W: nx--; break;
            case DIR_E: nx++; break;
            default: return;
        }
    }

    char path[SCREEN_PATH_MAX];
    if (game->in_dungeon)
        snprintf(path, sizeof(path), "assets/dungeons/%d/%02d_%02d.txt", game->dungeon.id, nx, ny);
    else
        snprintf(path, sizeof(path), "assets/screens/%02d_%02d.txt", nx, ny);
    if (!screen_load(&game->next_screen, path)) return;
    if (!game->in_dungeon) {
        apply_overworld_flags_to_screen(game, &game->next_screen, nx, ny);
    }

    game->trans_player_start = game->player.pos;

    Vector2 end = game->player.pos;
    switch (dir) {
        case DIR_N: end.y = PLAY_AREA_Y + (SCREEN_TILES_Y - 1) * TILE_SIZE; break;
        case DIR_S: end.y = PLAY_AREA_Y; break;
        case DIR_W: end.x = (SCREEN_TILES_X - 1) * TILE_SIZE; break;
        case DIR_E: end.x = 0; break;
        default: break;
    }
    game->trans_player_end = end;
    game->warp_dest_x = nx;
    game->warp_dest_y = ny;

    game->trans_type = TRANS_SCROLL;
    camera_start_scroll(&game->cam, dir);
    game->state = STATE_TRANSITION;
}

static bool has_open_door_at_edge(const Screen *screen, Direction dir, float player_pos) {
    int tile_pos;
    if (dir == DIR_N || dir == DIR_S) {
        tile_pos = (int)(player_pos / TILE_SIZE);
        int row = (dir == DIR_N) ? 0 : SCREEN_TILES_Y - 1;
        return (TileType)screen->tiles[row][tile_pos] == TILE_DOOR;
    } else {
        tile_pos = (int)((player_pos - PLAY_AREA_Y) / TILE_SIZE);
        int col = (dir == DIR_W) ? 0 : SCREEN_TILES_X - 1;
        return (TileType)screen->tiles[tile_pos][col] == TILE_DOOR;
    }
}

void nav_check_edge_transition(Game *game) {
    if (game->in_cave) return;

    Player *p = &game->player;
    Direction dir = (Direction)-1;

    if (p->pos.y <= PLAY_AREA_Y && p->facing == DIR_N)
        dir = DIR_N;
    else if (p->pos.y >= PLAY_AREA_Y + (SCREEN_TILES_Y - 1) * TILE_SIZE && p->facing == DIR_S)
        dir = DIR_S;
    else if (p->pos.x <= 0 && p->facing == DIR_W)
        dir = DIR_W;
    else if (p->pos.x >= (SCREEN_TILES_X - 1) * TILE_SIZE && p->facing == DIR_E)
        dir = DIR_E;

    if ((int)dir == -1) return;

    if (game->in_dungeon) {
        float edge_pos = (dir == DIR_N || dir == DIR_S) ? p->pos.x : p->pos.y;
        if (!has_open_door_at_edge(&game->current_screen, dir, edge_pos)) return;
        if (!can_transition_dungeon(game, dir)) return;
    } else {
        if (!can_transition_overworld(game->screen_x, game->screen_y, dir)) return;
    }

    start_scroll_transition(game, dir);
}

void nav_check_warp(Game *game) {
    int col = (int)(game->player.pos.x / TILE_SIZE);
    int row = (int)((game->player.pos.y - PLAY_AREA_Y) / TILE_SIZE);

    if (col < 0 || col >= SCREEN_TILES_X || row < 0 || row >= SCREEN_TILES_Y) return;

    TileType t = (TileType)game->current_screen.tiles[row][col];
    if (t != TILE_STAIRS) return;

    const Warp *w = screen_warp_at(&game->current_screen, col, row);
    if (!w) return;

    if (strcmp(w->dest, "return") == 0) {
        if (game->in_dungeon) {
            game->warp_dest_x = game->dungeon_return_screen_x;
            game->warp_dest_y = game->dungeon_return_screen_y;
        } else {
            game->warp_dest_x = game->return_screen_x;
            game->warp_dest_y = game->return_screen_y;
        }
        game->warp_dest_name[0] = '\0';
    } else if (strncmp(w->dest, "droom_", 6) == 0 && game->in_dungeon) {
        int drx, dry;
        if (sscanf(w->dest + 6, "%d_%d", &drx, &dry) != 2) return;
        game->warp_dest_x = drx;
        game->warp_dest_y = dry;
        strncpy(game->warp_dest_name, w->dest, WARP_DEST_MAX - 1);
        game->warp_dest_name[WARP_DEST_MAX - 1] = '\0';
    } else if (strncmp(w->dest, "dungeon_", 8) == 0) {
        game->dungeon_return_screen_x = game->screen_x;
        game->dungeon_return_screen_y = game->screen_y;
        game->dungeon_return_tile_col = col;
        game->dungeon_return_tile_row = row;
        strncpy(game->warp_dest_name, w->dest, WARP_DEST_MAX - 1);
        game->warp_dest_name[WARP_DEST_MAX - 1] = '\0';
    } else if (strncmp(w->dest, "cave_", 5) == 0) {
        game->return_screen_x = game->screen_x;
        game->return_screen_y = game->screen_y;
        game->return_tile_col = col;
        game->return_tile_row = row;
        strncpy(game->warp_dest_name, w->dest, WARP_DEST_MAX - 1);
        game->warp_dest_name[WARP_DEST_MAX - 1] = '\0';
    } else {
        int dx, dy;
        if (sscanf(w->dest, "%d_%d", &dx, &dy) != 2) return;
        game->warp_dest_x = dx;
        game->warp_dest_y = dy;
        game->warp_dest_name[0] = '\0';
    }

    game->trans_type = TRANS_FADE;
    camera_start_fade(&game->cam);
    game->state = STATE_TRANSITION;
}

void nav_position_at_return_warp(Game *game) {
    const Warp *rw = NULL;
    for (int i = 0; i < game->current_screen.warp_count; i++) {
        if (game->current_screen.warps[i].active &&
            strcmp(game->current_screen.warps[i].dest, "return") == 0) {
            rw = &game->current_screen.warps[i];
            break;
        }
    }
    if (rw) {
        game->player.pos.x = (float)(rw->tile_col * TILE_SIZE);
        game->player.pos.y = (float)(PLAY_AREA_Y + (rw->tile_row - 1) * TILE_SIZE);
    } else {
        game->player.pos.x = 7.0f * TILE_SIZE;
        game->player.pos.y = PLAY_AREA_Y + 5.0f * TILE_SIZE;
    }
}
