#include "dungeon_interact.h"
#include "game.h"
#include "sounds.h"
#include "textures.h"
#include <assert.h>
#include <math.h>

static bool all_enemies_dead(const Game *game) {
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].active) return false;
    }
    return true;
}

static void mark_item_collected(Game *game, int item_index) {
    dungeon_item_set_collected(&game->dungeon,
        game->dungeon.room_x, game->dungeon.room_y, item_index);
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

static bool boss_room_items_hidden(const Game *game) {
    return game->current_screen.is_boss_room && !game->dungeon.boss_defeated;
}

void dungeon_check_locked_door(Game *game) {
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
    if (t != TILE_WALL && t != TILE_DOOR_CLOSED) return;

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
            sound_play(SOUND_KEY_USE);
            sound_play(SOUND_DOOR_OPEN);
            break;
        }
    }
}

void dungeon_check_shutter_room(Game *game) {
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
        sound_play(SOUND_SHUTTER_OPEN);
    }
}

void dungeon_check_push_block(Game *game) {
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
        sound_play(SOUND_SECRET);
    }
}

void dungeon_check_items(Game *game) {
    if (!game->in_dungeon) return;
    if (boss_room_items_hidden(game)) return;

    int col = (int)(game->player.pos.x / TILE_SIZE);
    int row = (int)((game->player.pos.y - PLAY_AREA_Y) / TILE_SIZE);

    for (int i = 0; i < game->current_screen.item_count; i++) {
        ItemPlacement *ip = &game->current_screen.items[i];
        if (!ip->active) continue;
        if (ip->tile_col != col || ip->tile_row != row) continue;

        switch (ip->type) {
            case ITEM_KEY:
                game->player.inventory.keys++;
                break;
            case ITEM_MAP:
                game->dungeon.has_map = true;
                game->item_get_type = ITEM_MAP;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            case ITEM_COMPASS:
                game->dungeon.has_compass = true;
                game->item_get_type = ITEM_COMPASS;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            case ITEM_HEART_CONTAINER:
                game->player.max_health += 2;
                game->player.health = game->player.max_health;
                game->item_get_type = ITEM_HEART_CONTAINER;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            case ITEM_FRAGMENT:
                game->player.inventory.relic_fragments++;
                game->dungeon.fragment_collected = true;
                game->item_get_type = ITEM_FRAGMENT;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            case ITEM_BOOMERANG:
                game->player.inventory.items |= item_bit(ITEM_BOOMERANG);
                game->item_get_type = ITEM_BOOMERANG;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            case ITEM_BOW:
                game->player.inventory.items |= item_bit(ITEM_BOW);
                game->item_get_type = ITEM_BOW;
                game->item_get_timer = 120;
                game->state = STATE_ITEM_GET;
                break;
            default:
                break;
        }
        ip->active = false;
        mark_item_collected(game, i);
        sound_play(SOUND_ITEM_GET);
    }
}

TextureID dungeon_item_texture(ItemType type) {
    switch (type) {
        case ITEM_KEY:             return TEX_ITEM_KEY;
        case ITEM_MAP:             return TEX_ITEM_MAP;
        case ITEM_COMPASS:         return TEX_ITEM_COMPASS;
        case ITEM_HEART_CONTAINER: return TEX_ITEM_HEART_CONTAINER;
        case ITEM_FRAGMENT:        return TEX_ITEM_FRAGMENT;
        case ITEM_BOOMERANG:       return TEX_BOOMERANG;
        case ITEM_BOW:             return TEX_ITEM_KEY;
        default:                    return TEX_COUNT;
    }
}

void dungeon_draw_items(const Game *game) {
    if (!game->in_dungeon) return;
    if (boss_room_items_hidden(game)) return;
    for (int i = 0; i < game->current_screen.item_count; i++) {
        const ItemPlacement *ip = &game->current_screen.items[i];
        if (!ip->active) continue;
        int px = ip->tile_col * TILE_SIZE;
        int py = PLAY_AREA_Y + ip->tile_row * TILE_SIZE;
        TextureID tex_id = dungeon_item_texture(ip->type);
        assert(tex_id < TEX_COUNT && IsTextureValid(textures[tex_id]));
        DrawTexture(textures[tex_id], px, py, WHITE);
    }
}
