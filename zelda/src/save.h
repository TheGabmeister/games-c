#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>

#define SAVE_SLOT_COUNT 3

typedef struct SaveSlotSummary {
    bool exists;
    int health;
    int max_health;
    int rupees;
    int screen_x;
    int screen_y;
    bool in_dungeon;
    int dungeon_id;
} SaveSlotSummary;

struct Game;

bool save_read_summary(int slot, SaveSlotSummary *summary);
void save_start_new_game(struct Game *game, int slot);
bool save_write_game(const struct Game *game, int slot);
bool save_load_game(struct Game *game, int slot);

#endif
