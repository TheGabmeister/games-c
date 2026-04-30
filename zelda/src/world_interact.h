#ifndef WORLD_INTERACT_H
#define WORLD_INTERACT_H

#include "game_config.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct WorldState {
    uint64_t bombed_walls[WORLD_FLAG_WORDS];
    uint64_t burned_bushes[WORLD_FLAG_WORDS];
    uint64_t gifts_taken;
    uint64_t shops_spent;
    uint64_t npcs_triggered;
    uint64_t upgrades_taken;
} WorldState;

struct Game;

void world_init(WorldState *world);
void world_apply_screen_flags(struct Game *game);
void world_check_cave_interaction(struct Game *game);
void world_check_push_rock(struct Game *game);
void world_use_candle(struct Game *game);
void world_mark_bombable_revealed(struct Game *game);
void world_mark_bushes_burned(struct Game *game);
void world_draw_cave_npc(const struct Game *game);

bool world_flag_is_set(const uint64_t *words, int index);
void world_flag_set(uint64_t *words, int index);

#endif
