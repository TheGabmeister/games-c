#include "systems/input_system.h"
#include "components/velocity_comp.h"

void input_system_init(void) { /* nothing to initialise */ }

void input_system_reset_actions(InputActions *actions) {
    actions->shoot_pressed = false;
}

void input_system_process_event(SDL_Event *event, InputActions *actions) {
    if (event->type == SDL_EVENT_KEY_DOWN &&
        event->key.scancode == SDL_SCANCODE_SPACE) {
        actions->shoot_pressed = true;
    }
}

void input_system_update_player(ecs_world_t *world, ecs_entity_t player, float speed) {
    if (player == 0 || !ecs_is_alive(world, player)) return;
    const bool *keys = SDL_GetKeyboardState(NULL);
    float vx = 0.0f;
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])  vx -= speed;
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) vx += speed;
    ecs_set(world, player, Velocity, { .x = vx, .y = 0.0f });
}
