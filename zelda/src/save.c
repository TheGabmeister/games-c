#include "save.h"
#include "game.h"
#include "navigation.h"
#include "music.h"
#include "raylib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#define SAVE_MAGIC   0x3144565aU
#define SAVE_VERSION 1U

typedef struct SaveData {
    uint32_t magic;
    uint32_t version;

    Vector2 player_pos;
    Direction player_facing;
    int player_health;
    int player_max_health;
    Inventory inventory;
    WorldState world;
    DungeonState dungeon;

    int screen_x;
    int screen_y;
    bool in_dungeon;
    bool in_cave;
    int active_save_slot;
} SaveData;

static void ensure_save_dir(void) {
#ifdef _WIN32
    _mkdir("saves");
#else
    mkdir("saves", 0777);
#endif
}

static void save_path(int slot, char *path, int path_size) {
    snprintf(path, (size_t)path_size, "saves/slot_%d.sav", slot + 1);
}

static bool slot_valid(int slot) {
    return slot >= 0 && slot < SAVE_SLOT_COUNT;
}

static bool read_save_data(int slot, SaveData *data) {
    if (!slot_valid(slot)) return false;

    char path[64];
    save_path(slot, path, sizeof(path));
    int bytes = 0;
    unsigned char *raw = LoadFileData(path, &bytes);
    if (!raw) return false;
    bool ok = bytes == (int)sizeof(*data);
    if (ok) {
        memcpy(data, raw, sizeof(*data));
        ok = data->magic == SAVE_MAGIC && data->version == SAVE_VERSION;
    }
    UnloadFileData(raw);
    return ok;
}

bool save_read_summary(int slot, SaveSlotSummary *summary) {
    memset(summary, 0, sizeof(*summary));
    SaveData data;
    if (!read_save_data(slot, &data)) return false;

    summary->exists = true;
    summary->health = data.player_health;
    summary->max_health = data.player_max_health;
    summary->rupees = data.inventory.rupees;
    summary->screen_x = data.screen_x;
    summary->screen_y = data.screen_y;
    summary->in_dungeon = data.in_dungeon;
    summary->dungeon_id = data.dungeon.id;
    return true;
}

void save_start_new_game(Game *game, int slot) {
    memset(game, 0, sizeof(*game));
    game->state = STATE_PLAY;
    game->active_save_slot = slot;
    world_init(&game->world);
    player_init(&game->player);
    nav_load_screen(game, START_SCREEN_X, START_SCREEN_Y);
    music_exit_dungeon();
    save_write_game(game, slot);
}

bool save_write_game(const Game *game, int slot) {
    if (!slot_valid(slot)) return false;
    ensure_save_dir();

    SaveData data;
    memset(&data, 0, sizeof(data));
    data.magic = SAVE_MAGIC;
    data.version = SAVE_VERSION;
    data.player_pos = game->player.pos;
    data.player_facing = game->player.facing;
    data.player_health = game->player.health;
    data.player_max_health = game->player.max_health;
    data.inventory = game->player.inventory;
    data.world = game->world;
    data.dungeon = game->dungeon;
    data.in_dungeon = game->in_dungeon;
    data.in_cave = false;
    data.active_save_slot = slot;
    if (game->in_cave) {
        data.screen_x = game->return_screen_x;
        data.screen_y = game->return_screen_y;
    } else {
        data.screen_x = game->screen_x;
        data.screen_y = game->screen_y;
    }

    char path[64], temp_path[64];
    save_path(slot, path, sizeof(path));
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);

    if (!SaveFileData(temp_path, &data, (unsigned int)sizeof(data)))
        return false;
    remove(path);
    return rename(temp_path, path) == 0;
}

bool save_load_game(Game *game, int slot) {
    SaveData data;
    if (!read_save_data(slot, &data)) return false;

    memset(game, 0, sizeof(*game));
    game->state = STATE_PLAY;
    game->active_save_slot = slot;
    game->world = data.world;
    game->dungeon = data.dungeon;
    game->in_dungeon = data.in_dungeon;
    game->in_cave = false;

    if (game->in_dungeon) {
        dungeon_scan_rooms(&game->dungeon);
        music_enter_dungeon(game->dungeon.id);
        nav_load_dungeon_room(game, game->dungeon.room_x, game->dungeon.room_y);
    } else {
        music_exit_dungeon();
        nav_load_screen(game, data.screen_x, data.screen_y);
    }
    player_init(&game->player);
    game->player.pos = data.player_pos;
    game->player.facing = data.player_facing;
    game->player.health = data.player_health;
    game->player.max_health = data.player_max_health;
    game->player.inventory = data.inventory;
    anim_set(&game->player.anim, &player_idle_anims[game->player.facing]);
    world_apply_screen_flags(game);
    return true;
}
