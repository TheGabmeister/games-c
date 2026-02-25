#include <stdbool.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <flecs.h>

typedef struct { float x, y; } Position;
typedef struct { float w, h; } Size;

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Size);

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Space Invaders", 800, 600, 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    ecs_world_t *world = ecs_init();

    ECS_COMPONENT_DEFINE(world, Position);
    ECS_COMPONENT_DEFINE(world, Size);

    ecs_entity_t square = ecs_new(world);
    ecs_set(world, square, Position, {.x = 350, .y = 250});
    ecs_set(world, square, Size, {.w = 100, .h = 100});

    ecs_query_t *q = ecs_query(world, {
        .terms = {
            { ecs_id(Position) },
            { ecs_id(Size) }
        }
    });

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        ecs_iter_t it = ecs_query_iter(world, q);
        while (ecs_query_next(&it)) {
            Position *p = ecs_field(&it, Position, 0);
            Size *s = ecs_field(&it, Size, 1);
            for (int i = 0; i < it.count; i++) {
                SDL_FRect rect = { p[i].x, p[i].y, s[i].w, s[i].h };
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);
    }

    ecs_query_fini(q);
    ecs_fini(world);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
