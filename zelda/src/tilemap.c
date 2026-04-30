#include "tilemap.h"
#include "textures.h"
#include "enemy/enemy.h"
#include <stdio.h>
#include <string.h>

const TileDef tile_defs[TILE_TYPE_COUNT] = {
    [TILE_FLOOR]     = { TILE_FLOOR,     0, true,  ITEM_NONE },
    [TILE_WALL]      = { TILE_WALL,      1, false, ITEM_NONE },
    [TILE_WATER]     = { TILE_WATER,     2, false, ITEM_RAFT },
    [TILE_DOOR]      = { TILE_DOOR,      4, true,  ITEM_NONE },
    [TILE_PUSHBLOCK] = { TILE_PUSHBLOCK, 5, false, ITEM_NONE },
    [TILE_STAIRS]        = { TILE_STAIRS,        6, true,  ITEM_NONE },
    [TILE_BOMBABLE_WALL] = { TILE_BOMBABLE_WALL, 7, false, ITEM_NONE },
    [TILE_DOOR_CLOSED]   = { TILE_DOOR_CLOSED,   8, false, ITEM_NONE },
    [TILE_DOCK]          = { TILE_DOCK,          9, true,  ITEM_NONE },
    [TILE_GAP]           = { TILE_GAP,           2, false, ITEM_LADDER },
    [TILE_HEAVY_ROCK]    = { TILE_HEAVY_ROCK,    5, false, ITEM_BRACELET },
    [TILE_BUSH]          = { TILE_BUSH,          1, false, ITEM_CANDLE },
};

static ItemType item_from_name(const char *name) {
    if (strcmp(name, "rupees") == 0 || strcmp(name, "rupee") == 0)
        return ITEM_NONE;
    if (strcmp(name, "bombs") == 0)
        return ITEM_BOMB;
    if (strcmp(name, "keys") == 0)
        return ITEM_KEY;
    for (int t = 1; t < ITEM_TYPE_COUNT; t++) {
        if (item_type_names[t] && strcmp(name, item_type_names[t]) == 0)
            return (ItemType)t;
    }
    return ITEM_NONE;
}

static TileType char_to_tile(char c) {
    switch (c) {
        case 'W': return TILE_WALL;
        case '.': return TILE_FLOOR;
        case '~': return TILE_WATER;
        case 'D': return TILE_DOOR;
        case 'P': return TILE_PUSHBLOCK;
        case 'S': return TILE_STAIRS;
        case 'B': return TILE_BOMBABLE_WALL;
        case '=': return TILE_DOCK;
        case 'G': return TILE_GAP;
        case 'R': return TILE_HEAVY_ROCK;
        case 'T': return TILE_BUSH;
        default:  return TILE_FLOOR;
    }
}

static void copy_text_after_pipe(char *dst, int dst_size, const char *line) {
    const char *pipe = strchr(line, '|');
    if (!pipe) {
        dst[0] = '\0';
        return;
    }
    pipe++;
    while (*pipe == ' ') pipe++;
    strncpy(dst, pipe, (size_t)dst_size - 1);
    dst[dst_size - 1] = '\0';
}

bool screen_load(Screen *screen, const char *path) {
    char *text = LoadFileText(path);
    if (!text) return false;

    memset(screen, 0, sizeof(*screen));

    int row = 0;
    const char *p = text;

    while (*p && row < SCREEN_TILES_Y) {
        const char *line_start = p;

        while (*p && *p != '\n') p++;
        int line_len = (int)(p - line_start);
        if (line_len > 0 && line_start[line_len - 1] == '\r') line_len--;

        if (*p == '\n') p++;

        if (line_len == 0) continue;
        if (line_start[0] == '#') continue;

        bool has_colon = false;
        for (int i = 0; i < line_len; i++) {
            if (line_start[i] == ':') { has_colon = true; break; }
        }
        if (has_colon) {
            char line_copy[256];
            int copy_len = line_len < 255 ? line_len : 255;
            memcpy(line_copy, line_start, copy_len);
            line_copy[copy_len] = '\0';

            if (line_len > 5 && strncmp(line_start, "warp:", 5) == 0
                && screen->warp_count < MAX_WARPS_PER_SCREEN) {
                char type_buf[16];
                int wc, wr;
                char dest_buf[WARP_DEST_MAX];
                if (sscanf(line_copy, "warp: %15s %d %d -> %31s",
                           type_buf, &wc, &wr, dest_buf) == 4) {
                    Warp *w = &screen->warps[screen->warp_count++];
                    w->tile_col = wc;
                    w->tile_row = wr;
                    strncpy(w->dest, dest_buf, WARP_DEST_MAX - 1);
                    w->dest[WARP_DEST_MAX - 1] = '\0';
                    w->active = true;
                }
            } else if (line_len > 6 && strncmp(line_start, "enemy:", 6) == 0
                       && screen->enemy_spawn_count < MAX_ENEMIES_PER_SCREEN) {
                char type_buf[16];
                int ec, er;
                if (sscanf(line_copy, "enemy: %15s %d %d", type_buf, &ec, &er) == 3) {
                    int etype = -1;
                    for (int t = 0; t < ENEMY_TYPE_COUNT; t++) {
                        if (enemy_defs[t].name && strcmp(type_buf, enemy_defs[t].name) == 0) {
                            etype = t;
                            break;
                        }
                    }
                    if (etype >= 0) {
                        EnemySpawn *es = &screen->enemy_spawns[screen->enemy_spawn_count++];
                        es->type = etype;
                        es->tile_col = ec;
                        es->tile_row = er;
                    }
                }
            } else if (line_len > 5 && strncmp(line_start, "door:", 5) == 0
                       && screen->door_count < MAX_DOORS_PER_ROOM) {
                char dir_buf[16], type_buf[16];
                int pos;
                if (sscanf(line_copy, "door: %15s %d %15s", dir_buf, &pos, type_buf) == 3) {
                    DoorMeta *d = &screen->doors[screen->door_count++];
                    d->active = true;
                    d->position = pos;
                    if (strcmp(dir_buf, "north") == 0)      d->side = DIR_N;
                    else if (strcmp(dir_buf, "south") == 0) d->side = DIR_S;
                    else if (strcmp(dir_buf, "east") == 0)  d->side = DIR_E;
                    else if (strcmp(dir_buf, "west") == 0)  d->side = DIR_W;
                    else { screen->door_count--; }
                    if (d->active) {
                        if (strcmp(type_buf, "locked") == 0)       d->type = DOOR_LOCKED;
                        else if (strcmp(type_buf, "shutter") == 0) d->type = DOOR_SHUTTER;
                        else                                      d->type = DOOR_OPEN;
                    }
                }
            } else if (line_len > 5 && strncmp(line_start, "item:", 5) == 0
                       && screen->item_count < MAX_ITEMS_PER_ROOM) {
                char type_buf[24];
                int ic, ir;
                if (sscanf(line_copy, "item: %23s %d %d", type_buf, &ic, &ir) == 3) {
                    ItemType itype = ITEM_NONE;
                    for (int t = 1; t < ITEM_TYPE_COUNT; t++) {
                        if (item_type_names[t] && strcmp(type_buf, item_type_names[t]) == 0) {
                            itype = (ItemType)t;
                            break;
                        }
                    }
                    if (itype != ITEM_NONE) {
                        ItemPlacement *ip = &screen->items[screen->item_count++];
                        ip->type = itype;
                        ip->tile_col = ic;
                        ip->tile_row = ir;
                        ip->active = true;
                    }
                }
            } else if (strncmp(line_copy, "shutter:", 8) == 0) {
                char val[8];
                if (sscanf(line_copy, "shutter: %7s", val) == 1 && strcmp(val, "true") == 0) {
                    screen->is_shutter = true;
                }
            } else if (strncmp(line_copy, "dark:", 5) == 0) {
                char val[8];
                if (sscanf(line_copy, "dark: %7s", val) == 1 && strcmp(val, "true") == 0) {
                    screen->is_dark = true;
                }
            } else if (strncmp(line_copy, "boss:", 5) == 0) {
                char val[8];
                if (sscanf(line_copy, "boss: %7s", val) == 1 && strcmp(val, "true") == 0) {
                    screen->is_boss_room = true;
                }
            } else if (strncmp(line_copy, "dialogue:", 9) == 0) {
                int id;
                if (sscanf(line_copy, "dialogue: %d", &id) == 1) {
                    screen->dialogue.id = id;
                    screen->dialogue.active = true;
                    copy_text_after_pipe(screen->dialogue.text, CAVE_TEXT_MAX, line_copy);
                }
            } else if (strncmp(line_copy, "gift:", 5) == 0) {
                int id, amount;
                char reward_buf[24];
                if (sscanf(line_copy, "gift: %d %23s %d", &id, reward_buf, &amount) == 3) {
                    screen->gift.id = id;
                    screen->gift.reward = item_from_name(reward_buf);
                    screen->gift.amount = amount;
                    screen->gift.active = true;
                    copy_text_after_pipe(screen->gift.text, CAVE_TEXT_MAX, line_copy);
                }
            } else if (strncmp(line_copy, "upgrade:", 8) == 0) {
                int id, tier, required_health;
                char type_buf[16];
                if (sscanf(line_copy, "upgrade: %d %15s %d %d",
                           &id, type_buf, &tier, &required_health) == 4 &&
                    strcmp(type_buf, "sword") == 0) {
                    screen->upgrade.id = id;
                    screen->upgrade.sword_tier = tier;
                    screen->upgrade.required_max_health = required_health;
                    screen->upgrade.active = true;
                    copy_text_after_pipe(screen->upgrade.text, CAVE_TEXT_MAX, line_copy);
                }
            } else if (strncmp(line_copy, "shop:", 5) == 0) {
                int id;
                char item_buf[SHOP_ITEM_MAX][24];
                int price[SHOP_ITEM_MAX];
                int parsed = sscanf(line_copy, "shop: %d %23s %d %23s %d %23s %d",
                                    &id,
                                    item_buf[0], &price[0],
                                    item_buf[1], &price[1],
                                    item_buf[2], &price[2]);
                if (parsed >= 3) {
                    screen->shop.id = id;
                    screen->shop.active = true;
                    screen->shop.item_count = (parsed - 1) / 2;
                    if (screen->shop.item_count > SHOP_ITEM_MAX)
                        screen->shop.item_count = SHOP_ITEM_MAX;
                    for (int si = 0; si < screen->shop.item_count; si++) {
                        screen->shop.items[si].item = item_from_name(item_buf[si]);
                        screen->shop.items[si].price = price[si];
                    }
                }
            }
            continue;
        }

        if (line_len < SCREEN_TILES_X) continue;

        for (int col = 0; col < SCREEN_TILES_X; col++) {
            screen->tiles[row][col] = (uint8_t)char_to_tile(line_start[col]);
        }
        row++;
    }

    UnloadFileText(text);
    return row == SCREEN_TILES_Y;
}

void screen_draw(const Screen *screen) {
    screen_draw_offset(screen, 0, 0);
}

void screen_draw_offset(const Screen *screen, int offset_x, int offset_y) {
    for (int row = 0; row < SCREEN_TILES_Y; row++) {
        for (int col = 0; col < SCREEN_TILES_X; col++) {
            TileType t = (TileType)screen->tiles[row][col];
            int sprite = tile_defs[t].sprite_index;
            Rectangle src = texture_frame_rect(4, sprite);
            Vector2 pos = {
                (float)(col * TILE_SIZE + offset_x),
                (float)(PLAY_AREA_Y + row * TILE_SIZE + offset_y)
            };
            DrawTextureRec(textures[TEX_TILES], src, pos, WHITE);
        }
    }
}

bool screen_tile_blocked(const Screen *screen, Rectangle hitbox) {
    float rel_y = hitbox.y - PLAY_AREA_Y;

    int col_min = (int)(hitbox.x / TILE_SIZE);
    int col_max = (int)((hitbox.x + hitbox.width - 1) / TILE_SIZE);
    int row_min = (int)(rel_y / TILE_SIZE);
    int row_max = (int)((rel_y + hitbox.height - 1) / TILE_SIZE);

    if (col_min < 0) col_min = 0;
    if (col_max >= SCREEN_TILES_X) col_max = SCREEN_TILES_X - 1;
    if (row_min < 0) row_min = 0;
    if (row_max >= SCREEN_TILES_Y) row_max = SCREEN_TILES_Y - 1;

    for (int r = row_min; r <= row_max; r++) {
        for (int c = col_min; c <= col_max; c++) {
            if (!tile_defs[screen->tiles[r][c]].passable) return true;
        }
    }
    return false;
}

bool screen_tile_blocked_for_items(const Screen *screen, Rectangle hitbox,
                                   Rectangle current_hitbox, uint32_t item_flags) {
    float rel_y = hitbox.y - PLAY_AREA_Y;
    int current_col = (int)((current_hitbox.x + current_hitbox.width / 2.0f) / TILE_SIZE);
    int current_row = (int)((current_hitbox.y + current_hitbox.height / 2.0f - PLAY_AREA_Y) / TILE_SIZE);
    TileType current_tile = TILE_WALL;
    if (current_col >= 0 && current_col < SCREEN_TILES_X &&
        current_row >= 0 && current_row < SCREEN_TILES_Y) {
        current_tile = (TileType)screen->tiles[current_row][current_col];
    }

    int col_min = (int)(hitbox.x / TILE_SIZE);
    int col_max = (int)((hitbox.x + hitbox.width - 1) / TILE_SIZE);
    int row_min = (int)(rel_y / TILE_SIZE);
    int row_max = (int)((rel_y + hitbox.height - 1) / TILE_SIZE);

    if (col_min < 0) col_min = 0;
    if (col_max >= SCREEN_TILES_X) col_max = SCREEN_TILES_X - 1;
    if (row_min < 0) row_min = 0;
    if (row_max >= SCREEN_TILES_Y) row_max = SCREEN_TILES_Y - 1;

    for (int r = row_min; r <= row_max; r++) {
        for (int c = col_min; c <= col_max; c++) {
            const TileDef *def = &tile_defs[screen->tiles[r][c]];
            if (def->passable) continue;
            if (def->pass_requires == ITEM_NONE) return true;

            bool has_item = (item_flags & item_bit(def->pass_requires)) != 0;
            if (!has_item) return true;

            if (def->type == TILE_WATER &&
                current_tile != TILE_DOCK && current_tile != TILE_WATER) {
                return true;
            }
            if (def->type == TILE_BUSH || def->type == TILE_HEAVY_ROCK)
                return true;
        }
    }
    return false;
}

const TileDef *screen_tile_at_pixel(const Screen *screen, int px, int py) {
    int col = px / TILE_SIZE;
    int row = (py - PLAY_AREA_Y) / TILE_SIZE;
    if (col < 0 || col >= SCREEN_TILES_X || row < 0 || row >= SCREEN_TILES_Y) {
        return &tile_defs[TILE_WALL];
    }
    return &tile_defs[screen->tiles[row][col]];
}

bool screen_file_exists(int sx, int sy) {
    char path[SCREEN_PATH_MAX];
    snprintf(path, sizeof(path), "assets/screens/%02d_%02d.txt", sx, sy);
    return FileExists(path);
}

const Warp *screen_warp_at(const Screen *screen, int col, int row) {
    for (int i = 0; i < screen->warp_count; i++) {
        if (screen->warps[i].active &&
            screen->warps[i].tile_col == col &&
            screen->warps[i].tile_row == row) {
            return &screen->warps[i];
        }
    }
    return NULL;
}
