#include "game.h"
#include "sounds.h"
#include "hud.h"
#include "textures.h"
#include "debug.h"
#include "input.h"
#include "vfx.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static void load_screen_at(Game *game, int sx, int sy) {
    char path[SCREEN_PATH_MAX];
    snprintf(path, sizeof(path), "assets/screens/%02d_%02d.txt", sx, sy);
    if (!screen_load(&game->current_screen, path)) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
    game->screen_x = sx;
    game->screen_y = sy;
    enemies_spawn(game->enemies, &game->enemy_count, &game->current_screen);
    projectiles_clear(game->projectiles, &game->projectile_count);
    pickups_clear(game->pickups, &game->pickup_count);
    vfx_clear();
}

static void load_dungeon_room(Game *game, int rx, int ry) {
    char path[64];
    snprintf(path, sizeof(path), "assets/dungeons/%d/%02d_%02d.txt",
             game->dungeon.id, rx, ry);
    if (!screen_load(&game->current_screen, path)) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
    game->dungeon.room_x = rx;
    game->dungeon.room_y = ry;
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
                game->current_screen.tiles[0][d->position] = TILE_WALL;
                game->current_screen.tiles[0][d->position + 1] = TILE_WALL;
            } else if (d->side == DIR_S) {
                game->current_screen.tiles[SCREEN_TILES_Y - 1][d->position] = TILE_WALL;
                game->current_screen.tiles[SCREEN_TILES_Y - 1][d->position + 1] = TILE_WALL;
            } else if (d->side == DIR_W) {
                game->current_screen.tiles[d->position][0] = TILE_WALL;
                game->current_screen.tiles[d->position + 1][0] = TILE_WALL;
            } else if (d->side == DIR_E) {
                game->current_screen.tiles[d->position][SCREEN_TILES_X - 1] = TILE_WALL;
                game->current_screen.tiles[d->position + 1][SCREEN_TILES_X - 1] = TILE_WALL;
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

static void load_cave_screen(Game *game, const char *cave_name) {
    char path[SCREEN_PATH_MAX];
    snprintf(path, sizeof(path), "assets/caves/%s.txt", cave_name);
    if (!screen_load(&game->current_screen, path)) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
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

static void check_edge_transition(Game *game) {
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

static void check_warp(Game *game) {
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

static bool all_enemies_dead(const Game *game) {
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].active) return false;
    }
    return true;
}

static void mark_item_collected(Game *game, int item_index) {
    int room_idx = game->dungeon.room_y * DUNGEON_MAX_COLS + game->dungeon.room_x;
    uint64_t collected_bit = 1ULL << (room_idx % 16 * 4 + item_index);
    game->dungeon.items_collected |= collected_bit;
}

static void open_door_tiles(Screen *screen, const DoorMeta *d) {
    if (d->side == DIR_N) {
        screen->tiles[0][d->position] = TILE_DOOR;
        screen->tiles[0][d->position + 1] = TILE_DOOR;
    } else if (d->side == DIR_S) {
        screen->tiles[SCREEN_TILES_Y - 1][d->position] = TILE_DOOR;
        screen->tiles[SCREEN_TILES_Y - 1][d->position + 1] = TILE_DOOR;
    } else if (d->side == DIR_W) {
        screen->tiles[d->position][0] = TILE_DOOR;
        screen->tiles[d->position + 1][0] = TILE_DOOR;
    } else if (d->side == DIR_E) {
        screen->tiles[d->position][SCREEN_TILES_X - 1] = TILE_DOOR;
        screen->tiles[d->position + 1][SCREEN_TILES_X - 1] = TILE_DOOR;
    }
}

static void check_locked_door(Game *game) {
    if (!game->in_dungeon) return;

    Player *p = &game->player;
    if (p->state == PSTATE_KNOCKBACK || p->state == PSTATE_ATTACKING) return;

    int facing_col = (int)(p->pos.x / TILE_SIZE);
    int facing_row = (int)((p->pos.y - PLAY_AREA_Y) / TILE_SIZE);
    switch (p->facing) {
        case DIR_N: facing_row--; break;
        case DIR_S: facing_row++; break;
        case DIR_W: facing_col--; break;
        case DIR_E: facing_col++; break;
        default: break;
    }
    if (facing_col < 0 || facing_col >= SCREEN_TILES_X ||
        facing_row < 0 || facing_row >= SCREEN_TILES_Y) return;

    TileType t = (TileType)game->current_screen.tiles[facing_row][facing_col];
    if (t != TILE_WALL) return;

    for (int i = 0; i < game->current_screen.door_count; i++) {
        DoorMeta *d = &game->current_screen.doors[i];
        if (!d->active || d->type != DOOR_LOCKED) continue;

        bool matches = false;
        if (d->side == DIR_N && facing_row == 0 &&
            (facing_col == d->position || facing_col == d->position + 1))
            matches = true;
        else if (d->side == DIR_S && facing_row == SCREEN_TILES_Y - 1 &&
                 (facing_col == d->position || facing_col == d->position + 1))
            matches = true;
        else if (d->side == DIR_W && facing_col == 0 &&
                 (facing_row == d->position || facing_row == d->position + 1))
            matches = true;
        else if (d->side == DIR_E && facing_col == SCREEN_TILES_X - 1 &&
                 (facing_row == d->position || facing_row == d->position + 1))
            matches = true;

        if (matches && p->inventory.keys > 0) {
            p->inventory.keys--;
            open_door_tiles(&game->current_screen, d);
            dungeon_door_set_unlocked(&game->dungeon,
                game->dungeon.room_x, game->dungeon.room_y, d->side);
            int adj_rx = game->dungeon.room_x, adj_ry = game->dungeon.room_y;
            switch (d->side) {
                case DIR_N: adj_ry++; break;
                case DIR_S: adj_ry--; break;
                case DIR_W: adj_rx--; break;
                case DIR_E: adj_rx++; break;
                default: break;
            }
            dungeon_door_set_unlocked(&game->dungeon, adj_rx, adj_ry, opposite_dir(d->side));
            d->type = DOOR_OPEN;
            sound_play(game, SOUND_KEY_USE);
            sound_play(game, SOUND_DOOR_OPEN);
            break;
        }
    }
}

static void check_shutter_room(Game *game) {
    if (!game->in_dungeon) return;
    if (!game->current_screen.is_shutter) return;

    uint64_t rbit = dungeon_room_bit(game->dungeon.room_x, game->dungeon.room_y);
    if (game->dungeon.shutter_opened & rbit) return;

    if (game->enemy_count > 0 && all_enemies_dead(game)) {
        game->dungeon.shutter_opened |= rbit;
        game->dungeon.rooms_cleared |= rbit;
        for (int i = 0; i < game->current_screen.door_count; i++) {
            DoorMeta *d = &game->current_screen.doors[i];
            if (d->active) {
                open_door_tiles(&game->current_screen, d);
            }
        }
        sound_play(game, SOUND_SHUTTER_OPEN);
    }
}

static void check_dungeon_items(Game *game) {
    if (!game->in_dungeon) return;

    int col = (int)(game->player.pos.x / TILE_SIZE);
    int row = (int)((game->player.pos.y - PLAY_AREA_Y) / TILE_SIZE);

    for (int i = 0; i < game->current_screen.item_count; i++) {
        ItemPlacement *ip = &game->current_screen.items[i];
        if (!ip->active) continue;
        if (ip->tile_col != col || ip->tile_row != row) continue;

        switch (ip->type) {
            case DITEM_KEY:
                game->player.inventory.keys++;
                break;
            case DITEM_MAP:
                game->dungeon.has_map = true;
                game->item_get_type = DITEM_MAP;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            case DITEM_COMPASS:
                game->dungeon.has_compass = true;
                game->item_get_type = DITEM_COMPASS;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            case DITEM_HEART_CONTAINER:
                game->player.max_health += 2;
                game->player.health = game->player.max_health;
                game->item_get_type = DITEM_HEART_CONTAINER;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            case DITEM_FRAGMENT:
                game->player.inventory.relic_fragments++;
                game->dungeon.fragment_collected = true;
                game->item_get_type = DITEM_FRAGMENT;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            case DITEM_BOOMERANG:
                game->player.inventory.items |= (1 << ITEM_BOOMERANG);
                game->item_get_type = DITEM_BOOMERANG;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            case DITEM_BOW:
                game->player.inventory.items |= (1 << ITEM_BOW);
                game->item_get_type = DITEM_BOW;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            default:
                break;
        }
        ip->active = false;
        mark_item_collected(game, i);
        sound_play(game, SOUND_ITEM_GET);
    }
}

static void check_push_block(Game *game) {
    if (!game->in_dungeon) return;
    if (!all_enemies_dead(game)) {
        game->push_timer = 0;
        return;
    }

    Player *p = &game->player;
    if (p->state != PSTATE_MOVING) {
        game->push_timer = 0;
        return;
    }

    int col = (int)(p->pos.x / TILE_SIZE);
    int row = (int)((p->pos.y - PLAY_AREA_Y) / TILE_SIZE);
    int target_col = col, target_row = row;
    switch (p->facing) {
        case DIR_N: target_row--; break;
        case DIR_S: target_row++; break;
        case DIR_W: target_col--; break;
        case DIR_E: target_col++; break;
        default: break;
    }

    if (target_col < 0 || target_col >= SCREEN_TILES_X ||
        target_row < 0 || target_row >= SCREEN_TILES_Y) {
        game->push_timer = 0;
        return;
    }

    if ((TileType)game->current_screen.tiles[target_row][target_col] != TILE_PUSHBLOCK) {
        game->push_timer = 0;
        return;
    }

    float dist_x = fabsf(p->pos.x - target_col * TILE_SIZE);
    float dist_y = fabsf(p->pos.y - (PLAY_AREA_Y + target_row * TILE_SIZE));
    if (dist_x > TILE_SIZE * 1.2f || dist_y > TILE_SIZE * 1.2f) {
        game->push_timer = 0;
        return;
    }

    game->push_timer++;
    if (game->push_timer >= 12) {
        int dest_col = target_col, dest_row = target_row;
        switch (p->facing) {
            case DIR_N: dest_row--; break;
            case DIR_S: dest_row++; break;
            case DIR_W: dest_col--; break;
            case DIR_E: dest_col++; break;
            default: break;
        }

        game->current_screen.tiles[target_row][target_col] = TILE_STAIRS;

        if (dest_col >= 0 && dest_col < SCREEN_TILES_X &&
            dest_row >= 0 && dest_row < SCREEN_TILES_Y &&
            tile_defs[game->current_screen.tiles[dest_row][dest_col]].passable) {
            game->current_screen.tiles[dest_row][dest_col] = TILE_PUSHBLOCK;
        }

        game->push_timer = 0;
        sound_play(game, SOUND_SECRET);
    }
}

static void spawn_small_slimes(Game *game, Vector2 pos) {
    for (int s = 0; s < 2 && game->enemy_count < MAX_ENEMIES_PER_SCREEN; s++) {
        Enemy *e = &game->enemies[game->enemy_count++];
        *e = (Enemy){0};
        e->type = ENEMY_SLIME;
        e->subtype = 1;
        e->pos.x = pos.x + (s == 0 ? -16.0f : 16.0f);
        e->pos.y = pos.y;
        e->pos.x = Clamp(e->pos.x, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
        e->facing = DIR_S;
        e->state = ESTATE_IDLE;
        e->state_timer = 20 + rand() % 40;
        e->health = 1;
        e->active = true;
    }
}

static void check_boss_death(Game *game) {
    if (!game->in_dungeon || !game->current_screen.is_boss_room) return;
    if (game->dungeon.boss_defeated) return;

    game->dungeon.boss_defeated = true;
    sound_play(game, SOUND_BOSS_DEFEAT);
}

static void on_enemy_death(Game *game, Enemy *e) {
    if (e->type == ENEMY_DRAGON && game->in_dungeon && game->current_screen.is_boss_room) {
        check_boss_death(game);
        return;
    }
    int roll = rand() % 100;
    if (roll < 35) return;
    PickupType type;
    if (roll < 55)      type = PICKUP_RUPEE;
    else if (roll < 70) type = PICKUP_HEART;
    else if (roll < 85) type = PICKUP_ARROW;
    else                type = PICKUP_BOMB;
    pickup_spawn(game->pickups, &game->pickup_count, type, e->pos);
}

static void check_pickups(Game *game) {
    Player *p = &game->player;
    Rectangle player_rect = player_hitbox(p);
    for (int i = 0; i < game->pickup_count; i++) {
        Pickup *pk = &game->pickups[i];
        if (!pk->active) continue;
        if (!CheckCollisionRecs(player_rect, pickup_hitbox(pk))) continue;

        switch (pk->type) {
            case PICKUP_RUPEE:
                p->inventory.rupees += pk->value;
                if (p->inventory.rupees > 255) p->inventory.rupees = 255;
                sound_play(game, SOUND_PICKUP_RUPEE);
                break;
            case PICKUP_HEART:
                p->health += pk->value;
                if (p->health > p->max_health) p->health = p->max_health;
                sound_play(game, SOUND_PICKUP_HEART);
                break;
            case PICKUP_BOMB:
                p->inventory.bombs += pk->value;
                if (p->inventory.bombs > p->inventory.bomb_capacity)
                    p->inventory.bombs = p->inventory.bomb_capacity;
                sound_play(game, SOUND_PICKUP_BOMB);
                break;
            case PICKUP_ARROW:
                p->inventory.arrows += pk->value;
                if (p->inventory.arrows > p->inventory.arrow_capacity)
                    p->inventory.arrows = p->inventory.arrow_capacity;
                sound_play(game, SOUND_PICKUP_BOMB);
                break;
            default: break;
        }
        pk->active = false;
    }
}

static void check_bomb_explosions(Game *game) {
    for (int pi = 0; pi < game->projectile_count; pi++) {
        Projectile *proj = &game->projectiles[pi];
        if (!proj->active || proj->type != PROJ_BOMB || proj->timer > 0) continue;

        sound_play(game, SOUND_BOMB_EXPLODE);
        float cx = proj->pos.x + TILE_SIZE / 2.0f;
        float cy = proj->pos.y + TILE_SIZE / 2.0f;
        float r2 = BOMB_BLAST_RADIUS * BOMB_BLAST_RADIUS;

        for (int i = 0; i < game->enemy_count; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->active) continue;
            float dx = (e->pos.x + TILE_SIZE / 2.0f) - cx;
            float dy = (e->pos.y + TILE_SIZE / 2.0f) - cy;
            if (dx * dx + dy * dy <= r2) {
                enemy_take_damage(e, BOMB_DAMAGE);
                if (!e->active) {
                    sound_play(game, SOUND_ENEMY_DEATH);
                    on_enemy_death(game, e);
                }
            }
        }

        Player *p = &game->player;
        float pdx = (p->pos.x + TILE_SIZE / 2.0f) - cx;
        float pdy = (p->pos.y + TILE_SIZE / 2.0f) - cy;
        if (pdx * pdx + pdy * pdy <= r2) {
            Direction kb_dir;
            if (fabsf(pdx) > fabsf(pdy))
                kb_dir = (pdx > 0) ? DIR_E : DIR_W;
            else
                kb_dir = (pdy > 0) ? DIR_S : DIR_N;
            player_take_damage(p, BOMB_DAMAGE, kb_dir);
            sound_play(game, SOUND_PLAYER_DAMAGE);
        }

        int tc = (int)(cx / TILE_SIZE);
        int tr = (int)((cy - PLAY_AREA_Y) / TILE_SIZE);
        for (int dr = -2; dr <= 2; dr++) {
            for (int dc = -2; dc <= 2; dc++) {
                int r = tr + dr, c = tc + dc;
                if (r < 0 || r >= SCREEN_TILES_Y || c < 0 || c >= SCREEN_TILES_X) continue;
                if (game->current_screen.tiles[r][c] == TILE_BOMBABLE_WALL) {
                    game->current_screen.tiles[r][c] = TILE_FLOOR;
                }
            }
        }

        vfx_spawn(VFX_EXPLOSION, (Vector2){ cx, cy }, BOMB_BLAST_RADIUS);
        proj->active = false;
    }
}

static void check_combat(Game *game) {
    Player *p = &game->player;

    if (p->state == PSTATE_ATTACKING) {
        Rectangle sword = player_sword_hitbox(p);
        if (sword.width > 0) {
            for (int i = 0; i < game->enemy_count; i++) {
                Enemy *e = &game->enemies[i];
                if (!e->active || e->invuln_timer > 0) continue;
                if (CheckCollisionRecs(sword, enemy_hitbox(e))) {
                    e->invuln_timer = SWORD_ACTIVE_FRAMES;
                    sound_play(game, SOUND_SWORD_HIT);
                    if (e->type == ENEMY_SLIME && e->subtype == 0 &&
                        p->inventory.sword_tier <= 1) {
                        Vector2 split_pos = e->pos;
                        e->active = false;
                        e->state = ESTATE_DEAD;
                        sound_play(game, SOUND_ENEMY_DEATH);
                        spawn_small_slimes(game, split_pos);
                    } else {
                        enemy_take_damage(e, SWORD_DAMAGE);
                        if (!e->active) {
                            sound_play(game, SOUND_ENEMY_DEATH);
                            on_enemy_death(game, e);
                        }
                    }
                }
            }
        }
    }

    // Player projectiles vs enemies
    for (int pi = 0; pi < game->projectile_count; pi++) {
        Projectile *proj = &game->projectiles[pi];
        if (!proj->active || proj->owner != OWNER_PLAYER) continue;
        Rectangle proj_rect = projectile_hitbox(proj);
        for (int i = 0; i < game->enemy_count; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->active || e->invuln_timer > 0) continue;
            if (!CheckCollisionRecs(proj_rect, enemy_hitbox(e))) continue;

            if (e->type == ENEMY_SLIME && e->subtype == 0 &&
                p->inventory.sword_tier <= 1 && proj->type == PROJ_ARROW) {
                Vector2 split_pos = e->pos;
                e->active = false;
                e->state = ESTATE_DEAD;
                sound_play(game, SOUND_ENEMY_DEATH);
                spawn_small_slimes(game, split_pos);
            } else {
                enemy_take_damage(e, proj->damage);
                e->invuln_timer = SWORD_ACTIVE_FRAMES;
                if (!e->active) {
                    sound_play(game, SOUND_ENEMY_DEATH);
                    on_enemy_death(game, e);
                } else {
                    sound_play(game, SOUND_SWORD_HIT);
                }
            }

            if (proj->stun_frames > 0 && e->active) {
                e->state = ESTATE_IDLE;
                e->state_timer = proj->stun_frames;
                e->velocity = (Vector2){0, 0};
            }
            if (proj->type != PROJ_BOOMERANG) proj->active = false;
            break;
        }
    }

    if (p->invuln_timer <= 0 && p->state != PSTATE_KNOCKBACK) {
        Rectangle player_rect = player_hitbox(p);
        for (int i = 0; i < game->enemy_count; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->active) continue;
            if (CheckCollisionRecs(player_rect, enemy_hitbox(e))) {
                float dx = p->pos.x - e->pos.x;
                float dy = p->pos.y - e->pos.y;
                Direction kb_dir;
                if (fabsf(dx) > fabsf(dy))
                    kb_dir = (dx > 0) ? DIR_E : DIR_W;
                else
                    kb_dir = (dy > 0) ? DIR_S : DIR_N;
                player_take_damage(p, enemy_defs[e->type].contact_damage, kb_dir);
                sound_play(game, SOUND_PLAYER_DAMAGE);
                break;
            }
        }
    }

    // Enemy projectiles vs player
    if (p->invuln_timer <= 0 && p->state != PSTATE_KNOCKBACK) {
        Rectangle player_rect = player_hitbox(p);
        for (int pi = 0; pi < game->projectile_count; pi++) {
            Projectile *proj = &game->projectiles[pi];
            if (!proj->active || proj->owner != OWNER_ENEMY) continue;
            if (!CheckCollisionRecs(player_rect, projectile_hitbox(proj))) continue;

            bool blocked = false;
            if (p->state != PSTATE_ATTACKING && p->inventory.shield_tier >= 1) {
                Direction proj_from = opposite_dir(proj->facing);
                if (proj_from == p->facing) {
                    const ProjectileDef *pdef = &projectile_defs[proj->type];
                    if (pdef->blocked_by_shield_small ||
                        (p->inventory.shield_tier >= 2 && pdef->blocked_by_shield_large)) {
                        blocked = true;
                    }
                }
            }

            if (blocked) {
                proj->active = false;
                sound_play(game, SOUND_SHIELD_BLOCK);
            } else {
                player_take_damage(p, proj->damage, proj->facing);
                sound_play(game, SOUND_PLAYER_DAMAGE);
                proj->active = false;
            }
            break;
        }
    }
}

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    game->state = STATE_PLAY;

    player_init(&game->player);
    load_screen_at(game, START_SCREEN_X, START_SCREEN_Y);
}

void game_update(Game *game) {
    if (IsKeyPressed(KEY_F3)) debug_toggle();

    float dt = GetFrameTime();

    if (game->in_dungeon) {
        if (game->boss_music_loaded && game->current_screen.is_boss_room &&
            !game->dungeon.boss_defeated) {
            UpdateMusicStream(game->boss_music);
        } else if (game->dungeon_music_loaded) {
            UpdateMusicStream(game->dungeon_music);
        }
    } else if (game->music_loaded) {
        UpdateMusicStream(game->overworld_music);
    }

    switch (game->state) {
        case STATE_PLAY: {
            if (input_pause()) {
                game->state = STATE_PAUSE;
                break;
            }
            PlayerState prev_state = game->player.state;
            player_update(&game->player, &game->current_screen,
                          game->projectiles, &game->projectile_count, dt);
            if (game->player.state == PSTATE_ATTACKING && prev_state != PSTATE_ATTACKING) {
                sound_play(game, SOUND_SWORD_SWING);
            }
            if (game->player.state == PSTATE_USING_ITEM && prev_state != PSTATE_USING_ITEM &&
                game->player.inventory.equipped == ITEM_CANDLE &&
                game->in_dungeon && game->current_screen.is_dark) {
                uint64_t rbit = dungeon_room_bit(game->dungeon.room_x, game->dungeon.room_y);
                game->dungeon.rooms_lit |= rbit;
                sound_play(game, SOUND_SECRET);
            }
            enemies_update(game->enemies, game->enemy_count,
                           game->player.pos, &game->current_screen,
                           game->projectiles, &game->projectile_count, dt);
            projectiles_update(game->projectiles, &game->projectile_count,
                               &game->current_screen, game->player.pos, dt);
            check_bomb_explosions(game);
            vfx_update();
            check_combat(game);
            pickups_update(game->pickups, game->pickup_count);
            check_pickups(game);
            check_locked_door(game);
            check_shutter_room(game);
            check_push_block(game);
            check_dungeon_items(game);
            if (game->state != STATE_PLAY) break;
            if (game->player.health <= 0) {
                game->state = STATE_DEATH;
                game->death_timer = 90;
                break;
            }
            if (game->player.health > 0 &&
                game->player.health <= LOW_HEALTH_THRESHOLD) {
                game->low_health_counter++;
                if (game->low_health_counter >= LOW_HEALTH_BEEP_FRAMES) {
                    sound_play(game, SOUND_LOW_HEALTH);
                    game->low_health_counter = 0;
                }
            } else {
                game->low_health_counter = 0;
            }
            check_warp(game);
            if (game->state == STATE_PLAY) {
                check_edge_transition(game);
            }
            break;
        }

        case STATE_TRANSITION:
            camera_update(&game->cam);

            if (camera_at_midpoint(&game->cam)) {
                if (game->warp_dest_name[0] != '\0' &&
                    strncmp(game->warp_dest_name, "droom_", 6) == 0 && game->in_dungeon) {
                    int drx = game->warp_dest_x, dry = game->warp_dest_y;
                    bool was_boss = game->current_screen.is_boss_room;
                    load_dungeon_room(game, drx, dry);
                    bool is_boss = game->current_screen.is_boss_room && !game->dungeon.boss_defeated;
                    if (is_boss && !was_boss) {
                        if (game->dungeon_music_loaded) StopMusicStream(game->dungeon_music);
                        if (game->boss_music_loaded) PlayMusicStream(game->boss_music);
                        sound_play(game, SOUND_BOSS_ROAR);
                    } else if (!is_boss && was_boss) {
                        if (game->boss_music_loaded) StopMusicStream(game->boss_music);
                        if (game->dungeon_music_loaded) PlayMusicStream(game->dungeon_music);
                    }
                    game->player.pos.x = 7.0f * TILE_SIZE;
                    game->player.pos.y = PLAY_AREA_Y + 5.0f * TILE_SIZE;
                    game->warp_dest_name[0] = '\0';
                } else if (game->warp_dest_name[0] != '\0' &&
                    strncmp(game->warp_dest_name, "dungeon_", 8) == 0) {
                    int dungeon_id = 0;
                    sscanf(game->warp_dest_name + 8, "%d", &dungeon_id);
                    memset(&game->dungeon, 0, sizeof(game->dungeon));
                    game->dungeon.id = dungeon_id;
                    game->dungeon.entrance_room_x = 0;
                    game->dungeon.entrance_room_y = 0;
                    dungeon_scan_rooms(&game->dungeon);
                    game->in_dungeon = true;

                    if (game->music_loaded) {
                        StopMusicStream(game->overworld_music);
                    }
                    char dmus_path[64];
                    snprintf(dmus_path, sizeof(dmus_path), "assets/music/dungeon%d.ogg", dungeon_id);
                    if (FileExists(dmus_path)) {
                        game->dungeon_music = LoadMusicStream(dmus_path);
                        game->dungeon_music_loaded = IsMusicValid(game->dungeon_music);
                        if (game->dungeon_music_loaded) PlayMusicStream(game->dungeon_music);
                    }
                    snprintf(dmus_path, sizeof(dmus_path), "assets/music/boss%d.ogg", dungeon_id);
                    if (FileExists(dmus_path)) {
                        game->boss_music = LoadMusicStream(dmus_path);
                        game->boss_music_loaded = IsMusicValid(game->boss_music);
                    }

                    load_dungeon_room(game, 0, 0);
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
                    game->warp_dest_name[0] = '\0';
                } else if (game->warp_dest_name[0] != '\0') {
                    load_cave_screen(game, game->warp_dest_name);
                    game->in_cave = true;
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
                } else if (game->in_dungeon) {
                    load_screen_at(game, game->warp_dest_x, game->warp_dest_y);
                    game->in_dungeon = false;
                    if (game->dungeon_music_loaded) {
                        StopMusicStream(game->dungeon_music);
                        UnloadMusicStream(game->dungeon_music);
                        game->dungeon_music_loaded = false;
                    }
                    if (game->boss_music_loaded) {
                        StopMusicStream(game->boss_music);
                        UnloadMusicStream(game->boss_music);
                        game->boss_music_loaded = false;
                    }
                    if (game->music_loaded) {
                        PlayMusicStream(game->overworld_music);
                    }
                    game->player.pos.x = (float)(game->dungeon_return_tile_col * TILE_SIZE);
                    game->player.pos.y = (float)(PLAY_AREA_Y + (game->dungeon_return_tile_row - 1) * TILE_SIZE);
                } else if (game->in_cave) {
                    load_screen_at(game, game->warp_dest_x, game->warp_dest_y);
                    game->in_cave = false;
                    game->player.pos.x = (float)(game->return_tile_col * TILE_SIZE);
                    game->player.pos.y = (float)(PLAY_AREA_Y + (game->return_tile_row - 1) * TILE_SIZE);
                } else {
                    load_screen_at(game, game->warp_dest_x, game->warp_dest_y);
                    game->player.pos.x = 7.0f * TILE_SIZE;
                    game->player.pos.y = PLAY_AREA_Y + 5.0f * TILE_SIZE;
                }
            }

            if (!camera_is_active(&game->cam)) {
                if (game->trans_type == TRANS_SCROLL) {
                    game->player.pos = game->trans_player_end;
                    if (game->in_dungeon) {
                        bool was_boss = game->current_screen.is_boss_room;
                        load_dungeon_room(game, game->warp_dest_x, game->warp_dest_y);
                        bool is_boss = game->current_screen.is_boss_room && !game->dungeon.boss_defeated;
                        if (is_boss && !was_boss) {
                            if (game->dungeon_music_loaded) StopMusicStream(game->dungeon_music);
                            if (game->boss_music_loaded) PlayMusicStream(game->boss_music);
                            sound_play(game, SOUND_BOSS_ROAR);
                        } else if (!is_boss && was_boss) {
                            if (game->boss_music_loaded) StopMusicStream(game->boss_music);
                            if (game->dungeon_music_loaded) PlayMusicStream(game->dungeon_music);
                        }
                    } else {
                        game->current_screen = game->next_screen;
                        game->screen_x = game->warp_dest_x;
                        game->screen_y = game->warp_dest_y;
                        enemies_spawn(game->enemies, &game->enemy_count,
                                      &game->current_screen);
                        projectiles_clear(game->projectiles, &game->projectile_count);
                        pickups_clear(game->pickups, &game->pickup_count);
                        vfx_clear();
                    }
                }
                game->state = STATE_PLAY;
            }
            break;

        case STATE_DEATH:
            game->death_timer--;
            if (game->death_timer <= 0) {
                if (game->in_dungeon) {
                    game->player.health = 6;
                    game->player.state = PSTATE_IDLE;
                    game->player.invuln_timer = 0;
                    game->player.knockback_timer = 0;
                    game->dungeon.rooms_cleared = 0;
                    game->dungeon.shutter_opened = 0;
                    game->dungeon.rooms_lit = 0;
                    load_dungeon_room(game, game->dungeon.entrance_room_x,
                                      game->dungeon.entrance_room_y);
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
                } else {
                    player_init(&game->player);
                    game->in_cave = false;
                    load_screen_at(game, START_SCREEN_X, START_SCREEN_Y);
                }
                game->state = STATE_PLAY;
            }
            break;

        case STATE_ITEM_GET:
            game->item_get_timer--;
            if (game->item_get_timer <= 0) {
                game->state = STATE_PLAY;
            }
            break;

        case STATE_PAUSE:
            if (input_pause() || input_back()) {
                game->state = STATE_PLAY;
            } else {
                pause_screen_update(&game->pause_state, &game->player.inventory);
            }
            break;

        default:
            break;
    }
}

static TextureID dungeon_item_texture(DungeonItemType type) {
    switch (type) {
        case DITEM_KEY:             return TEX_ITEM_KEY;
        case DITEM_MAP:             return TEX_ITEM_MAP;
        case DITEM_COMPASS:         return TEX_ITEM_COMPASS;
        case DITEM_HEART_CONTAINER: return TEX_ITEM_HEART_CONTAINER;
        case DITEM_FRAGMENT:        return TEX_ITEM_FRAGMENT;
        case DITEM_BOOMERANG:       return TEX_BOOMERANG;
        case DITEM_BOW:             return TEX_ITEM_KEY;
        default:                    return TEX_COUNT;
    }
}

static Color dungeon_item_fallback_color(DungeonItemType type) {
    switch (type) {
        case DITEM_KEY:             return YELLOW;
        case DITEM_MAP:             return BLUE;
        case DITEM_COMPASS:         return RED;
        case DITEM_HEART_CONTAINER: return RED;
        case DITEM_FRAGMENT:        return GOLD;
        case DITEM_BOOMERANG:       return SKYBLUE;
        case DITEM_BOW:             return BROWN;
        default:                    return WHITE;
    }
}

static void draw_dungeon_items(const Game *game) {
    if (!game->in_dungeon) return;
    for (int i = 0; i < game->current_screen.item_count; i++) {
        const ItemPlacement *ip = &game->current_screen.items[i];
        if (!ip->active) continue;
        int px = ip->tile_col * TILE_SIZE;
        int py = PLAY_AREA_Y + ip->tile_row * TILE_SIZE;
        TextureID tex_id = dungeon_item_texture(ip->type);
        if (tex_id < TEX_COUNT && IsTextureValid(textures[tex_id])) {
            DrawTexture(textures[tex_id], px, py, WHITE);
        } else {
            DrawRectangle(px + TILE_SIZE / 4, py + TILE_SIZE / 4,
                          TILE_SIZE / 2, TILE_SIZE / 2, dungeon_item_fallback_color(ip->type));
        }
    }
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 20, 24, 32, 255 });

    if (game->state == STATE_TRANSITION && game->cam.type == TRANS_SCROLL) {
        int ox_old, oy_old, ox_new, oy_new;
        camera_scroll_offset_old(&game->cam, &ox_old, &oy_old);
        camera_scroll_offset_new(&game->cam, &ox_new, &oy_new);

        BeginScissorMode(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT);
        screen_draw_offset(&game->current_screen, ox_old, oy_old);
        screen_draw_offset(&game->next_screen, ox_new, oy_new);

        float t = (float)game->cam.timer / game->cam.total_frames;
        Vector2 player_pos = {
            game->trans_player_start.x + (game->trans_player_end.x - game->trans_player_start.x) * t,
            game->trans_player_start.y + (game->trans_player_end.y - game->trans_player_start.y) * t,
        };
        Vector2 old_pos = game->player.pos;
        game->player.pos = player_pos;
        player_draw(&game->player);
        game->player.pos = old_pos;

        EndScissorMode();
    } else if (game->state == STATE_TRANSITION && game->cam.type == TRANS_FADE) {
        screen_draw(&game->current_screen);
        enemies_draw(game->enemies, game->enemy_count);
        player_draw(&game->player);
        unsigned char alpha = camera_fade_alpha(&game->cam);
        DrawRectangle(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT,
                      (Color){ 0, 0, 0, alpha });
    } else if (game->state == STATE_DEATH) {
        screen_draw(&game->current_screen);
        enemies_draw(game->enemies, game->enemy_count);
        player_draw(&game->player);
        int half = 45;
        if (game->death_timer < half) {
            unsigned char alpha = (unsigned char)(255 * (half - game->death_timer) / half);
            DrawRectangle(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT,
                          (Color){ 0, 0, 0, alpha });
        }
    } else if (game->state == STATE_PAUSE) {
        screen_draw(&game->current_screen);
        enemies_draw(game->enemies, game->enemy_count);
        pickups_draw(game->pickups, game->pickup_count);
        projectiles_draw(game->projectiles, game->projectile_count);
        player_draw(&game->player);
        pause_screen_draw(&game->pause_state, game);
    } else if (game->state == STATE_ITEM_GET) {
        screen_draw(&game->current_screen);
        enemies_draw(game->enemies, game->enemy_count);
        player_draw(&game->player);
        DrawRectangle(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT,
                      (Color){ 0, 0, 0, 160 });

        const char *item_name = "ITEM";
        Color item_color = GOLD;
        switch (game->item_get_type) {
            case DITEM_KEY:             item_name = "KEY";             item_color = YELLOW; break;
            case DITEM_MAP:             item_name = "DUNGEON MAP";     item_color = BLUE; break;
            case DITEM_COMPASS:         item_name = "COMPASS";         item_color = RED; break;
            case DITEM_HEART_CONTAINER: item_name = "HEART CONTAINER"; item_color = RED; break;
            case DITEM_FRAGMENT:        item_name = "RELIC FRAGMENT";  item_color = GOLD; break;
            case DITEM_BOOMERANG:       item_name = "BOOMERANG";       item_color = SKYBLUE; break;
            case DITEM_BOW:             item_name = "BOW";             item_color = BROWN; break;
            default: break;
        }

        int item_x = (int)game->player.pos.x + TILE_SIZE / 2 - 20;
        int item_y = (int)game->player.pos.y - TILE_SIZE;
        TextureID get_tex = dungeon_item_texture(game->item_get_type);
        if (get_tex < TEX_COUNT && IsTextureValid(textures[get_tex])) {
            DrawTexture(textures[get_tex], item_x - 12, item_y - 12, WHITE);
        } else {
            DrawRectangle(item_x, item_y, 40, 40, item_color);
        }

        int text_w = MeasureText(item_name, 30);
        DrawText(item_name, WINDOW_WIDTH / 2 - text_w / 2,
                 PLAY_AREA_Y + PLAY_AREA_HEIGHT / 2 + 60, 30, WHITE);
    } else {
        screen_draw(&game->current_screen);
        draw_dungeon_items(game);
        enemies_draw(game->enemies, game->enemy_count);
        pickups_draw(game->pickups, game->pickup_count);
        projectiles_draw(game->projectiles, game->projectile_count);
        player_draw(&game->player);
        vfx_draw();

        if (game->in_dungeon && game->current_screen.is_dark) {
            uint64_t rbit = dungeon_room_bit(game->dungeon.room_x, game->dungeon.room_y);
            if (!(game->dungeon.rooms_lit & rbit)) {
                DrawRectangle(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT, BLACK);
            }
        }
    }

    hud_draw(game);
    debug_draw_game(game);
    debug_draw_overlay();

    EndDrawing();
}
