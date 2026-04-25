#ifndef WORLD_H
#define WORLD_H

#include "game.h"

void world_reset_player(Game *game);
void world_spawn_wave(Game *game);
void world_update_playing(Game *game, float dt);
void world_draw_playfield(Game *game);

int world_count_active_grunts(Game *game);
int world_count_active_humans(Game *game);
int world_count_active_hulks(Game *game);
int world_count_active_electrodes(Game *game);
int world_next_human_rescue_score(Game *game);

#endif
