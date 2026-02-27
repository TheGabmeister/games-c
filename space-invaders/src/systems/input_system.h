#pragma once

#include <flecs.h>
#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct {
    bool shoot_pressed;
} InputActions;

void input_system_init(void);

/* Reset per-frame action flags — call at the start of each frame */
void input_system_reset_actions(InputActions *actions);

/* Feed each SDL event in; sets shoot_pressed on SPACE key-down */
void input_system_process_event(SDL_Event *event, InputActions *actions);

/* Read current keyboard state and apply horizontal velocity to the player */
void input_system_update_player(ecs_world_t *world, ecs_entity_t player, float speed);
