#include "world_state.h"
#include "game.h"
#include "dialogue.h"
#include "shop.h"
#include "sounds.h"
#include "input.h"
#include "textures.h"
#include "raylib.h"
#include <string.h>

void world_init(WorldState *world) {
    memset(world, 0, sizeof(*world));
}

bool world_flag_is_set(const uint64_t *words, int index) {
    if (index < 0) return false;
    return (words[index / 64] & (1ULL << (index % 64))) != 0;
}

void world_flag_set(uint64_t *words, int index) {
    if (index < 0) return;
    words[index / 64] |= 1ULL << (index % 64);
}

static int current_screen_index(const Game *game) {
    if (game->in_dungeon || game->in_cave) return -1;
    if (game->screen_x < 0 || game->screen_x >= OVERWORLD_COLS ||
        game->screen_y < 0 || game->screen_y >= OVERWORLD_ROWS) {
        return -1;
    }
    return game->screen_y * OVERWORLD_COLS + game->screen_x;
}

static bool service_near_player(const Game *game) {
    float cx = game->player.pos.x + TILE_SIZE / 2.0f;
    float cy = game->player.pos.y + TILE_SIZE / 2.0f;
    float npc_x = (float)(game->current_screen.npc_col * TILE_SIZE);
    float npc_y = PLAY_AREA_Y + (float)(game->current_screen.npc_row * TILE_SIZE);
    return CheckCollisionRecs((Rectangle){ cx - 96, cy - 96, 192, 192 },
                              (Rectangle){ npc_x - 32, npc_y - 32, 64, 64 });
}

static bool cave_has_service(const Screen *screen) {
    return screen->dialogue.active || screen->gift.active ||
           screen->upgrade.active || screen->shop.active;
}

static bool service_flag(const uint64_t *words, int id) {
    if (id < 0 || id >= WORLD_SERVICE_MAX) return false;
    return (words[id / 64] & (1ULL << (id % 64))) != 0;
}

static void set_service_flag(uint64_t *words, int id) {
    if (id < 0 || id >= WORLD_SERVICE_MAX) return;
    words[id / 64] |= 1ULL << (id % 64);
}

static void grant_gift(Game *game, const CaveGiftMeta *gift) {
    inventory_grant(&game->player.inventory,
                    &game->player.health, &game->player.max_health,
                    gift->reward, gift->amount);
}

void world_apply_screen_flags(const Game *game, Screen *screen, int sx, int sy) {
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

void world_mark_bombable_revealed(Game *game) {
    int index = current_screen_index(game);
    if (index < 0) return;
    for (int r = 0; r < SCREEN_TILES_Y; r++) {
        for (int c = 0; c < SCREEN_TILES_X; c++) {
            if ((TileType)game->current_screen.tiles[r][c] == TILE_BOMBABLE_WALL) {
                world_flag_set(game->world.bombed_walls, index);
                return;
            }
        }
    }
}

void world_mark_bushes_burned(Game *game) {
    int index = current_screen_index(game);
    if (index < 0) return;
    bool burned = false;
    for (int r = 0; r < SCREEN_TILES_Y; r++) {
        for (int c = 0; c < SCREEN_TILES_X; c++) {
            if ((TileType)game->current_screen.tiles[r][c] == TILE_BUSH) {
                game->current_screen.tiles[r][c] = TILE_FLOOR;
                burned = true;
            }
        }
    }
    if (burned) world_flag_set(game->world.burned_bushes, index);
}

void world_use_candle(Game *game) {
    if (game->candle_used_this_screen) return;
    game->candle_used_this_screen = true;
    world_mark_bushes_burned(game);
}

void world_check_push_rock(Game *game) {
    if ((game->player.inventory.items & item_bit(ITEM_BRACELET)) == 0) {
        game->push_timer = 0;
        return;
    }

    int col = (int)((game->player.pos.x + TILE_SIZE / 2.0f) / TILE_SIZE);
    int row = (int)((game->player.pos.y + TILE_SIZE / 2.0f - PLAY_AREA_Y) / TILE_SIZE);
    int tr = row, tc = col;
    int dr = 0, dc = 0;
    switch (game->player.facing) {
        case DIR_N: tr--; dr = -1; break;
        case DIR_S: tr++; dr = 1; break;
        case DIR_W: tc--; dc = -1; break;
        case DIR_E: tc++; dc = 1; break;
        default: return;
    }
    if (tr < 0 || tr >= SCREEN_TILES_Y || tc < 0 || tc >= SCREEN_TILES_X ||
        (TileType)game->current_screen.tiles[tr][tc] != TILE_HEAVY_ROCK) {
        game->push_timer = 0;
        return;
    }
    if (game->heavy_rock_pushed[tr][tc]) {
        game->push_timer = 0;
        return;
    }

    game->push_timer++;
    if (game->push_timer < 12) return;

    int nr = tr + dr;
    int nc = tc + dc;
    if (nr < 0 || nr >= SCREEN_TILES_Y || nc < 0 || nc >= SCREEN_TILES_X ||
        !tile_defs[game->current_screen.tiles[nr][nc]].passable) {
        game->push_timer = 0;
        return;
    }

    game->current_screen.tiles[tr][tc] = TILE_FLOOR;
    game->current_screen.tiles[nr][nc] = TILE_HEAVY_ROCK;
    game->heavy_rock_pushed[nr][nc] = true;
    game->push_timer = 0;
    sound_play(SOUND_SECRET);
}

void world_check_cave_interaction(Game *game) {
    if (!game->in_cave || !cave_has_service(&game->current_screen) ||
        !input_confirm() || !service_near_player(game)) {
        return;
    }

    const Screen *screen = &game->current_screen;
    if (screen->shop.active) {
        shop_open(game, &screen->shop);
        return;
    }

    if (screen->gift.active) {
        if (!service_flag(game->world.gifts_taken, screen->gift.id)) {
            grant_gift(game, &screen->gift);
            set_service_flag(game->world.gifts_taken, screen->gift.id);
            dialogue_start(game, screen->gift.text[0] ? screen->gift.text : "TAKE THIS.");
        } else {
            dialogue_start(game, "IT IS ALREADY YOURS.");
        }
        return;
    }

    if (screen->upgrade.active) {
        if (service_flag(game->world.upgrades_taken, screen->upgrade.id)) {
            dialogue_start(game, "YOU ALREADY HAVE THIS.");
        } else if (game->player.max_health < screen->upgrade.required_max_health) {
            dialogue_start(game, "RETURN WITH STRONGER HEARTS.");
        } else {
            Inventory *inv = &game->player.inventory;
            switch (screen->upgrade.upgrade_type) {
                case UPGRADE_SWORD: inv->sword_tier = screen->upgrade.tier; break;
                case UPGRADE_SHIELD: inv->shield_tier = screen->upgrade.tier; break;
                case UPGRADE_ARMOR: inv->armor_tier = screen->upgrade.tier; break;
                case UPGRADE_BOMB_CAPACITY: inv->bomb_capacity = screen->upgrade.tier; break;
                case UPGRADE_ARROW_CAPACITY: inv->arrow_capacity = screen->upgrade.tier; break;
                default: break;
            }
            set_service_flag(game->world.upgrades_taken, screen->upgrade.id);
            dialogue_start(game, screen->upgrade.text[0] ? screen->upgrade.text : "TAKE THIS.");
            sound_play(SOUND_ITEM_GET);
        }
        return;
    }

    if (screen->dialogue.active) {
        set_service_flag(game->world.npcs_triggered, screen->dialogue.id);
        dialogue_start(game, screen->dialogue.text);
    }
}

void world_draw_cave_npc(const Game *game) {
    if (!game->in_cave || !cave_has_service(&game->current_screen)) return;

    Vector2 pos = { (float)(game->current_screen.npc_col * TILE_SIZE),
                     PLAY_AREA_Y + (float)(game->current_screen.npc_row * TILE_SIZE) };
    if (IsTextureValid(textures[TEX_NPC_OLD_MAN])) {
        DrawTexture(textures[TEX_NPC_OLD_MAN], (int)pos.x, (int)pos.y, WHITE);
    } else {
        DrawRectangle((int)pos.x + 12, (int)pos.y + 8, 40, 52,
                      (Color){ 118, 87, 181, 255 });
        DrawCircle((int)pos.x + 32, (int)pos.y + 20, 14,
                   (Color){ 232, 199, 154, 255 });
    }
}
