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
#include "components.h"
#include "entity_inspector.h"

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Size);
ECS_COMPONENT_DECLARE(Velocity);
ECS_COMPONENT_DECLARE(BoxCollider);
ECS_TAG_DECLARE(Projectile);
ECS_TAG_DECLARE(Enemy);
ECS_TAG_DECLARE(EnemyProjectile);
ECS_TAG_DECLARE(Player);

#define RESOURCES       "resources/"
#define PLAYER_SPEED    300.0f
#define BULLET_SPEED    600.0f
#define BULLET_SIZE     10.0f
#define WINDOW_W        600
#define WINDOW_H        800
#define ENEMY_SHOOT_INTERVAL 5.0f

typedef struct {
    ecs_entity_t e;
    float x, y, w, h;
} ColliderBounds;

static ecs_entity_t spawn_ship(ecs_world_t *world, const char *name, ecs_entity_t tag, float x, float y) {
    ecs_entity_t e = ecs_new(world);
    ecs_set_name(world, e, name);
    ecs_add_id(world, e, tag);
    ecs_set(world, e, Position,    {.x = x, .y = y});
    ecs_set(world, e, Size,        {.w = 100, .h = 100});
    ecs_set(world, e, Velocity,    {.x = 0, .y = 0});
    ecs_set(world, e, BoxCollider, {.w = 100, .h = 100});
    return e;
}

static ecs_entity_t spawn_bullet(ecs_world_t *world, const char *name, ecs_entity_t tag, float x, float y, float vy) {
    ecs_entity_t b = ecs_new(world);
    ecs_set_name(world, b, name);
    ecs_add_id(world, b, tag);
    ecs_set(world, b, Position,    {.x = x, .y = y});
    ecs_set(world, b, Size,        {.w = BULLET_SIZE, .h = BULLET_SIZE});
    ecs_set(world, b, Velocity,    {.x = 0, .y = vy});
    ecs_set(world, b, BoxCollider, {.w = BULLET_SIZE, .h = BULLET_SIZE});
    return b;
}

static int collect_collider_bounds(ecs_world_t *world, ecs_query_t *q, ColliderBounds *out, int max) {
    int n = 0;
    ecs_iter_t it = ecs_query_iter(world, q);
    while (ecs_query_next(&it)) {
        Position    *p = ecs_field(&it, Position, 0);
        BoxCollider *c = ecs_field(&it, BoxCollider, 1);
        for (int i = 0; i < it.count && n < max; i++) {
            out[n] = (ColliderBounds){ it.entities[i], p[i].x, p[i].y, c[i].w, c[i].h };
            n++;
        }
    }
    return n;
}

static bool aabb_overlap(const ColliderBounds *a, float bx, float by, float bw, float bh) {
    return a->x < bx + bw && a->x + a->w > bx &&
           a->y < by + bh && a->y + a->h > by;
}

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
    ECS_TAG_DEFINE(world, EnemyProjectile);
    ECS_TAG_DEFINE(world, Player);

    ecs_entity_t player = spawn_ship(world, "PlayerShip", Player, 350, 650);
    ecs_entity_t enemy  = spawn_ship(world, "EnemyShip",  Enemy,  250, 100);

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
    ecs_query_t *enemy_proj_coll_q = ecs_query(world, {
        .terms = {
            { ecs_id(Position) },
            { ecs_id(BoxCollider) },
            { ecs_id(EnemyProjectile) }
        }
    });
    ecs_query_t *player_coll_q = ecs_query(world, {
        .terms = {
            { ecs_id(Position) },
            { ecs_id(BoxCollider) },
            { ecs_id(Player) }
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

    ecs_entity_t bullet = 0;       // 0 means no player bullet in flight
    ecs_entity_t enemy_bullet = 0; // 0 means no enemy bullet in flight
    float enemy_shoot_timer = ENEMY_SHOOT_INTERVAL;
    float respawn_timer = 0.0f;

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
                if (bullet == 0 && player != 0) {
                    const Position *pp = ecs_get(world, player, Position);
                    const Size     *ps = ecs_get(world, player, Size);
                    float bx = pp->x + ps->w / 2.0f - BULLET_SIZE / 2.0f;
                    float by = pp->y - BULLET_SIZE;
                    bullet = spawn_bullet(world, "Bullet", Projectile, bx, by, -BULLET_SPEED);
                }
            }
        }
        nk_input_end(ctx);

        // Player movement
        if (player != 0) {
            const bool *keys = SDL_GetKeyboardState(NULL);
            float vx = 0, vy = 0;
            if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])  vx -= PLAYER_SPEED;
            if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) vx += PLAYER_SPEED;
            ecs_set(world, player, Velocity, {.x = vx, .y = vy});
        }

        // Player respawn after 3 seconds
        if (player == 0) {
            respawn_timer -= dt;
            if (respawn_timer <= 0.0f) {
                player = spawn_ship(world, "PlayerShip", Player, 350, 650);
            }
        }

        // Enemy shoots downward every 5 seconds
        if (enemy != 0) {
            enemy_shoot_timer -= dt;
            if (enemy_shoot_timer <= 0.0f) {
                enemy_shoot_timer = ENEMY_SHOOT_INTERVAL;
                if (enemy_bullet == 0) {
                    const Position *ep = ecs_get(world, enemy, Position);
                    const Size     *es = ecs_get(world, enemy, Size);
                    float bx = ep->x + es->w / 2.0f - BULLET_SIZE / 2.0f;
                    float by = ep->y + es->h;
                    enemy_bullet = spawn_bullet(world, "EnemyBullet", EnemyProjectile, bx, by, BULLET_SPEED);
                }
            }
        }

        // Destroy player bullet if it has left the top of the window
        if (bullet != 0) {
            const Position *bp = ecs_get(world, bullet, Position);
            const Size     *bs = ecs_get(world, bullet, Size);
            if (bp->y + bs->h < 0) {
                ecs_delete(world, bullet);
                bullet = 0;
            }
        }

        // Destroy enemy bullet if it has left the bottom of the window
        if (enemy_bullet != 0) {
            const Position *bp = ecs_get(world, enemy_bullet, Position);
            if (bp->y > WINDOW_H) {
                ecs_delete(world, enemy_bullet);
                enemy_bullet = 0;
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
                if (it.entities[i] == player) {
                    SDL_RenderTexture(renderer, player_tex, NULL, &rect);
                } else if (it.entities[i] == enemy) {
                    SDL_RenderTexture(renderer, enemy_tex, NULL, &rect);
                } else {
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        // Collision detection: collect live projectile bounds
        ColliderBounds projs[64];
        int nprojs = collect_collider_bounds(world, proj_coll_q, projs, 64);

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
                        if (aabb_overlap(&projs[j], ep[i].x, ep[i].y, ec[i].w, ec[i].h)) {
                            to_delete[ndel++] = it.entities[i];
                            to_delete[ndel++] = projs[j].e;
                            break;
                        }
                    }
                }
            }
        }

        // Collect enemy projectile bounds
        ColliderBounds eprojs[64];
        int neprojs = collect_collider_bounds(world, enemy_proj_coll_q, eprojs, 64);

        // Test player against enemy projectiles; collect hits for deferred deletion
        {
            ecs_iter_t it = ecs_query_iter(world, player_coll_q);
            while (ecs_query_next(&it)) {
                Position    *pp = ecs_field(&it, Position, 0);
                BoxCollider *pc = ecs_field(&it, BoxCollider, 1);
                for (int i = 0; i < it.count; i++) {
                    for (int j = 0; j < neprojs; j++) {
                        if (aabb_overlap(&eprojs[j], pp[i].x, pp[i].y, pc[i].w, pc[i].h)) {
                            to_delete[ndel++] = it.entities[i];
                            to_delete[ndel++] = eprojs[j].e;
                            break;
                        }
                    }
                }
            }
        }

        // Deferred deletion — guard with ecs_is_alive to handle duplicates
        for (int i = 0; i < ndel; i++) {
            if (!ecs_is_alive(world, to_delete[i])) continue;
            if (to_delete[i] == bullet)       bullet = 0;
            if (to_delete[i] == enemy)        enemy  = 0;
            if (to_delete[i] == enemy_bullet) enemy_bullet = 0;
            if (to_delete[i] == player) { player = 0; respawn_timer = 3.0f; }
            ecs_delete(world, to_delete[i]);
        }

        draw_entity_inspector(ctx, world, debug_q);

        nk_sdl_render(ctx, NK_ANTI_ALIASING_ON);
        nk_sdl_update_TextInput(ctx);

        SDL_RenderPresent(renderer);

        nk_input_begin(ctx);
    }

    ecs_query_fini(debug_q);
    ecs_query_fini(q);
    ecs_query_fini(proj_coll_q);
    ecs_query_fini(enemy_coll_q);
    ecs_query_fini(enemy_proj_coll_q);
    ecs_query_fini(player_coll_q);
    ecs_fini(world);
    SDL_DestroyTexture(player_tex);
    SDL_DestroyTexture(enemy_tex);
    nk_sdl_shutdown(ctx);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
