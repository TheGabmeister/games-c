#pragma once
#include "game.h"

/* Set player to spawn position and initial state */
void player_init(GameState *gs);
void player_reset_position(GameState *gs);

/* Read keyboard input into queued_dir */
void player_input(GameState *gs);

/* Move, eat tiles, check ghost collision */
void player_update(GameState *gs, float dt);

/* Advance death animation; returns true when animation is complete */
bool player_update_death(GameState *gs, float dt);
