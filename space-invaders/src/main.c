#include <stdbool.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <flecs.h>

/* mandatory: sdl3_renderer depends on those defines */
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT

#define NK_IMPLEMENTATION
#include "nuklear.h"
#define NK_SDL3_RENDERER_IMPLEMENTATION
#include "nuklear_sdl3_renderer.h"

#include "texture.h"

typedef struct { float x, y; } Position;
typedef struct { float w, h; } Size;
typedef struct { float x, y; } Velocity;
typedef struct { float w, h; } BoxCollider;

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Size);
ECS_COMPONENT_DECLARE(Velocity);
ECS_COMPONENT_DECLARE(BoxCollider);
ECS_TAG_DECLARE(Projectile);
ECS_TAG_DECLARE(Enemy);

#define RESOURCES       "resources/"
#define PLAYER_SPEED    300.0f
#define BULLET_SPEED    600.0f
#define BULLET_SIZE     10.0f
#define WINDOW_W        600
#define WINDOW_H        800

static bool initialize(SDL_Window **window, SDL_Renderer **renderer) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    *window = SDL_CreateWindow("Space Invaders", WINDOW_W, WINDOW_H, 0);
    if (!*window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, NULL);
    if (!*renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return false;
    }

    return true;
}

int main(void) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    if (!initialize(&window, &renderer)) {
        return 1;
    }

    struct nk_context *ctx = nk_sdl_init(window, renderer, nk_sdl_allocator());
    nk_sdl_style_set_debug_font(ctx);

    const char *base = SDL_GetBasePath();
    char tex_path[512];

    snprintf(tex_path, sizeof(tex_path), "%s" RESOURCES "player-ship.png", base);
    SDL_Texture *player_tex = load_texture(renderer, tex_path);

    snprintf(tex_path, sizeof(tex_path), "%s" RESOURCES "enemy-ufo.png", base);
    SDL_Texture *enemy_tex = load_texture(renderer, tex_path);

    ecs_world_t *world = ecs_init();

    ECS_COMPONENT_DEFINE(world, Position);
    ECS_COMPONENT_DEFINE(world, Size);
    ECS_COMPONENT_DEFINE(world, Velocity);
    ECS_COMPONENT_DEFINE(world, BoxCollider);
    ECS_TAG_DEFINE(world, Projectile);
    ECS_TAG_DEFINE(world, Enemy);

    ecs_entity_t square = ecs_new(world);
    ecs_set_name(world, square, "Player");
    ecs_set(world, square, Position, {.x = 350, .y = 650});
    ecs_set(world, square, Size, {.w = 100, .h = 100});
    ecs_set(world, square, Velocity, {.x = 0, .y = 0});

    ecs_entity_t enemy = ecs_new(world);
    ecs_set_name(world, enemy, "EnemyShip");
    ecs_add(world, enemy, Enemy);
    ecs_set(world, enemy, Position,    {.x = 250, .y = 100});
    ecs_set(world, enemy, Size,        {.w = 100, .h = 100});
    ecs_set(world, enemy, Velocity,    {.x = 0, .y = 0});
    ecs_set(world, enemy, BoxCollider, {.w = 100, .h = 100});

    ecs_query_t *proj_coll_q = ecs_query(world, {
        .terms = {
            { ecs_id(Position) },
            { ecs_id(BoxCollider) },
            { ecs_id(Projectile) }
        }
    });
    ecs_query_t *enemy_coll_q = ecs_query(world, {
        .terms = {
            { ecs_id(Position) },
            { ecs_id(BoxCollider) },
            { ecs_id(Enemy) }
        }
    });

    // Query for all renderable/moveable entities (player + projectiles)
    ecs_query_t *q = ecs_query(world, {
        .terms = {
            { ecs_id(Position) },
            { ecs_id(Size) },
            { ecs_id(Velocity) }
        }
    });

    // Query for the entity inspector: all entities with a Position
    ecs_query_t *debug_q = ecs_query(world, {
        .terms = {
            { ecs_id(Position) }
        }
    });

    ecs_entity_t bullet = 0; // 0 means no bullet in flight

    bool running = true;
    SDL_Event event;
    Uint64 last_ticks = SDL_GetTicks();

    nk_input_begin(ctx);

    while (running) {
        Uint64 now = SDL_GetTicks();
        float dt = (now - last_ticks) / 1000.0f;
        last_ticks = now;

        while (SDL_PollEvent(&event)) {
            SDL_ConvertEventToRenderCoordinates(renderer, &event);
            nk_sdl_handle_event(ctx, &event);
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
                    ecs_set_name(world, bullet, "Bullet");
                    ecs_add(world, bullet, Projectile);
                    ecs_set(world, bullet, Position,    {.x = bx, .y = by});
                    ecs_set(world, bullet, Size,        {.w = BULLET_SIZE, .h = BULLET_SIZE});
                    ecs_set(world, bullet, Velocity,    {.x = 0, .y = -BULLET_SPEED});
                    ecs_set(world, bullet, BoxCollider, {.w = BULLET_SIZE, .h = BULLET_SIZE});
                }
            }
        }
        nk_input_end(ctx);

        // Player movement
        const bool *keys = SDL_GetKeyboardState(NULL);
        float vx = 0, vy = 0;
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])  vx -= PLAYER_SPEED;
        if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) vx += PLAYER_SPEED;
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
                if (it.entities[i] == square) {
                    SDL_RenderTexture(renderer, player_tex, NULL, &rect);
                } else if (it.entities[i] == enemy) {
                    SDL_RenderTexture(renderer, enemy_tex, NULL, &rect);
                } else {
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        // Collision detection: collect live projectile bounds
        struct { ecs_entity_t e; float x, y, w, h; } projs[64];
        int nprojs = 0;
        {
            ecs_iter_t it = ecs_query_iter(world, proj_coll_q);
            while (ecs_query_next(&it)) {
                Position    *p = ecs_field(&it, Position, 0);
                BoxCollider *c = ecs_field(&it, BoxCollider, 1);
                for (int i = 0; i < it.count && nprojs < 64; i++) {
                    projs[nprojs].e = it.entities[i];
                    projs[nprojs].x = p[i].x; projs[nprojs].y = p[i].y;
                    projs[nprojs].w = c[i].w; projs[nprojs].h = c[i].h;
                    nprojs++;
                }
            }
        }

        // Test each enemy against every projectile; collect hits for deferred deletion
        ecs_entity_t to_delete[128];
        int ndel = 0;
        {
            ecs_iter_t it = ecs_query_iter(world, enemy_coll_q);
            while (ecs_query_next(&it)) {
                Position    *ep = ecs_field(&it, Position, 0);
                BoxCollider *ec = ecs_field(&it, BoxCollider, 1);
                for (int i = 0; i < it.count; i++) {
                    for (int j = 0; j < nprojs; j++) {
                        bool hit = projs[j].x < ep[i].x + ec[i].w && projs[j].x + projs[j].w > ep[i].x &&
                                   projs[j].y < ep[i].y + ec[i].h && projs[j].y + projs[j].h > ep[i].y;
                        if (hit) {
                            to_delete[ndel++] = it.entities[i];
                            to_delete[ndel++] = projs[j].e;
                            break;
                        }
                    }
                }
            }
        }

        // Deferred deletion — guard with ecs_is_alive to handle duplicates
        for (int i = 0; i < ndel; i++) {
            if (!ecs_is_alive(world, to_delete[i])) continue;
            if (to_delete[i] == bullet) bullet = 0;
            if (to_delete[i] == enemy)  enemy  = 0;
            ecs_delete(world, to_delete[i]);
        }

        // Entity inspector window
        {
            int entity_count = 0;
            ecs_iter_t dit = ecs_query_iter(world, debug_q);
            while (ecs_query_next(&dit)) entity_count += dit.count;

            if (nk_begin(ctx, "Entity Inspector",
                         nk_rect(10, 10, 220, 200),
                         NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                         NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
            {
                char count_buf[32];
                snprintf(count_buf, sizeof(count_buf), "Entities: %d", entity_count);
                nk_layout_row_dynamic(ctx, 18, 1);
                nk_label(ctx, count_buf, NK_TEXT_LEFT);

                nk_layout_row_template_begin(ctx, 18);
                nk_layout_row_template_push_static(ctx, 70);
                nk_layout_row_template_push_static(ctx, 65);
                nk_layout_row_template_push_static(ctx, 65);
                nk_layout_row_template_end(ctx);

                nk_label(ctx, "Name", NK_TEXT_LEFT);
                nk_label(ctx, "X",    NK_TEXT_LEFT);
                nk_label(ctx, "Y",    NK_TEXT_LEFT);

                dit = ecs_query_iter(world, debug_q);
                while (ecs_query_next(&dit)) {
                    Position *p = ecs_field(&dit, Position, 0);
                    for (int i = 0; i < dit.count; i++) {
                        const char *name = ecs_get_name(world, dit.entities[i]);
                        char xbuf[24], ybuf[24];
                        snprintf(xbuf, sizeof(xbuf), "%.1f", p[i].x);
                        snprintf(ybuf, sizeof(ybuf), "%.1f", p[i].y);
                        nk_label(ctx, name ? name : "unnamed", NK_TEXT_LEFT);
                        nk_label(ctx, xbuf, NK_TEXT_LEFT);
                        nk_label(ctx, ybuf, NK_TEXT_LEFT);
                    }
                }
            }
            nk_end(ctx);
        }

        nk_sdl_render(ctx, NK_ANTI_ALIASING_ON);
        nk_sdl_update_TextInput(ctx);

        SDL_RenderPresent(renderer);

        nk_input_begin(ctx);
    }

    ecs_query_fini(debug_q);
    ecs_query_fini(q);
    ecs_query_fini(proj_coll_q);
    ecs_query_fini(enemy_coll_q);
    ecs_fini(world);
    SDL_DestroyTexture(player_tex);
    SDL_DestroyTexture(enemy_tex);
    nk_sdl_shutdown(ctx);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
