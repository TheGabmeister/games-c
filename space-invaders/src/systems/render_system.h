#pragma once

#include <flecs.h>
#include <SDL3/SDL.h>

struct nk_context;

void render_system_init(SDL_Window *window, SDL_Renderer *renderer, ecs_world_t *world);
struct nk_context *render_system_get_ctx(void);

/* Call for each SDL event (forwards to Nuklear) */
void render_system_handle_event(SDL_Event *event);

/* Call after the SDL_PollEvent loop — ends Nuklear input collection */
void render_system_end_input(void);

/* Clears the screen for a new frame */
void render_system_clear(void);

/* Draws all entities that have Position + Size + Sprite */
void render_system_update(ecs_world_t *world);

/* Renders Nuklear UI, presents the frame, and begins the next input collection */
void render_system_present(void);

/* Shuts down Nuklear; does NOT destroy the SDL renderer or window */
void render_system_shutdown(void);
