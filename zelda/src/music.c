#include "music.h"
#include "raylib.h"
#include <stdio.h>

static Music overworld_music;
static bool overworld_loaded;
static int current_biome;

static Music dungeon_music;
static bool dungeon_loaded;

static Music boss_music;
static bool boss_loaded;

static bool boss_active;

static void unload_dungeon_streams(void) {
    if (dungeon_loaded) {
        StopMusicStream(dungeon_music);
        UnloadMusicStream(dungeon_music);
        dungeon_loaded = false;
    }
    if (boss_loaded) {
        StopMusicStream(boss_music);
        UnloadMusicStream(boss_music);
        boss_loaded = false;
    }
    boss_active = false;
}

static void load_overworld_track(const char *path) {
    if (overworld_loaded) {
        StopMusicStream(overworld_music);
        UnloadMusicStream(overworld_music);
        overworld_loaded = false;
    }
    if (FileExists(path)) {
        overworld_music = LoadMusicStream(path);
        overworld_loaded = IsMusicValid(overworld_music);
        if (overworld_loaded) PlayMusicStream(overworld_music);
    }
}

void music_init(void) {
    current_biome = 0;
    load_overworld_track("assets/music/overworld.ogg");
}

void music_update(void) {
    if (boss_active && boss_loaded) {
        UpdateMusicStream(boss_music);
    } else if (dungeon_loaded) {
        UpdateMusicStream(dungeon_music);
    } else if (overworld_loaded) {
        UpdateMusicStream(overworld_music);
    }
}

void music_cleanup(void) {
    if (overworld_loaded) UnloadMusicStream(overworld_music);
    if (dungeon_loaded) UnloadMusicStream(dungeon_music);
    if (boss_loaded) UnloadMusicStream(boss_music);
}

void music_enter_dungeon(int dungeon_id) {
    if (overworld_loaded) StopMusicStream(overworld_music);
    unload_dungeon_streams();

    char path[64];
    snprintf(path, sizeof(path), "assets/music/dungeon%d.ogg", dungeon_id);
    if (FileExists(path)) {
        dungeon_music = LoadMusicStream(path);
        dungeon_loaded = IsMusicValid(dungeon_music);
        if (dungeon_loaded) PlayMusicStream(dungeon_music);
    }
    snprintf(path, sizeof(path), "assets/music/boss%d.ogg", dungeon_id);
    if (FileExists(path)) {
        boss_music = LoadMusicStream(path);
        boss_loaded = IsMusicValid(boss_music);
    }
    boss_active = false;
}

void music_exit_dungeon(void) {
    unload_dungeon_streams();
    if (overworld_loaded) PlayMusicStream(overworld_music);
}

void music_set_boss(bool in_boss_room) {
    if (in_boss_room == boss_active) return;
    boss_active = in_boss_room;
    if (in_boss_room) {
        if (dungeon_loaded) StopMusicStream(dungeon_music);
        if (boss_loaded) PlayMusicStream(boss_music);
    } else {
        if (boss_loaded) StopMusicStream(boss_music);
        if (dungeon_loaded) PlayMusicStream(dungeon_music);
    }
}

void music_set_biome(int biome_id) {
    if (biome_id == current_biome && overworld_loaded) return;
    current_biome = biome_id;
    char path[64];
    if (biome_id > 0)
        snprintf(path, sizeof(path), "assets/music/overworld_%d.ogg", biome_id);
    else
        snprintf(path, sizeof(path), "assets/music/overworld.ogg");
    load_overworld_track(path);
}
