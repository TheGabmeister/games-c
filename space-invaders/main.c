#include <stdbool.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <flecs.h>

typedef struct { float x, y; } Position;
typedef struct { float w, h; } Size;
typedef struct { float x, y; } Velocity;

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Size);
ECS_COMPONENT_DECLARE(Velocity);
ECS_TAG_DECLARE(Projectile);

#define PLAYER_SPEED    300.0f
#define BULLET_SPEED    600.0f
#define BULLET_SIZE     10.0f
#define WINDOW_W        600
#define WINDOW_H        800

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Space Invaders", WINDOW_W, WINDOW_H, 0);
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
    ECS_COMPONENT_DEFINE(world, Velocity);
    ECS_TAG_DEFINE(world, Projectile);

    ecs_entity_t square = ecs_new(world);
    ecs_set(world, square, Position, {.x = 350, .y = 250});
    ecs_set(world, square, Size, {.w = 100, .h = 100});
    ecs_set(world, square, Velocity, {.x = 0, .y = 0});

    // Query for all renderable/moveable entities (player + projectiles)
    ecs_query_t *q = ecs_query(world, {
        .terms = {
            { ecs_id(Position) },
            { ecs_id(Size) },
            { ecs_id(Velocity) }
        }
    });

    ecs_entity_t bullet = 0; // 0 means no bullet in flight

    bool running = true;
    SDL_Event event;
    Uint64 last_ticks = SDL_GetTicks();

    while (running) {
        Uint64 now = SDL_GetTicks();
        float dt = (now - last_ticks) / 1000.0f;
        last_ticks = now;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_SPACE) {
                if (bullet == 0) {
                    const Position *pp = ecs_get(world, square, Position);
                    const Size     *ps = ecs_get(world, square, Size);
                    float bx = pp->x + ps->w / 2.0f - BULLET_SIZE / 2.0f;
                    float by = pp->y - BULLET_SIZE;
                    bullet = ecs_new(world);
                    ecs_add(world, bullet, Projectile);
                    ecs_set(world, bullet, Position, {.x = bx, .y = by});
                    ecs_set(world, bullet, Size,     {.w = BULLET_SIZE, .h = BULLET_SIZE});
                    ecs_set(world, bullet, Velocity, {.x = 0, .y = -BULLET_SPEED});
                }
            }
        }

        // Player movement
        const bool *keys = SDL_GetKeyboardState(NULL);
        float vx = 0, vy = 0;
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])  vx -= PLAYER_SPEED;
        if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) vx += PLAYER_SPEED;
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])    vy -= PLAYER_SPEED;
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])  vy += PLAYER_SPEED;
        ecs_set(world, square, Velocity, {.x = vx, .y = vy});

        // Destroy bullet if it has left the top of the window
        if (bullet != 0) {
            const Position *bp = ecs_get(world, bullet, Position);
            const Size     *bs = ecs_get(world, bullet, Size);
            if (bp->y + bs->h < 0) {
                ecs_delete(world, bullet);
                bullet = 0;
            }
        }

        // Move and render all entities
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        ecs_iter_t it = ecs_query_iter(world, q);
        while (ecs_query_next(&it)) {
            Position *p = ecs_field(&it, Position, 0);
            Size     *s = ecs_field(&it, Size, 1);
            Velocity *v = ecs_field(&it, Velocity, 2);
            for (int i = 0; i < it.count; i++) {
                p[i].x += v[i].x * dt;
                p[i].y += v[i].y * dt;
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
