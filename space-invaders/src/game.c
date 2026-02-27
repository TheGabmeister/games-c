#include <stdbool.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <flecs.h>

#include "game.h"
#include "texture.h"
#include "entity_inspector.h"
#include "entities/player.h"
#include "entities/enemy.h"
#include "entities/bullet.h"
#include "systems/physics_system.h"
#include "systems/render_system.h"
#include "systems/collision_system.h"
#include "systems/input_system.h"
#include "components/box_collider_comp.h"
#include "components/sprite_comp.h"
#include "components/transform_comp.h"
#include "components/velocity_comp.h"
#include "components/health_comp.h"

/* ---- ECS component/tag definitions (one translation unit only) ---- */

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Size);
ECS_COMPONENT_DECLARE(Velocity);
ECS_COMPONENT_DECLARE(BoxCollider);
ECS_COMPONENT_DECLARE(Sprite);
ECS_COMPONENT_DECLARE(Health);
ECS_TAG_DECLARE(Projectile);
ECS_TAG_DECLARE(Enemy);
ECS_TAG_DECLARE(EnemyProjectile);
ECS_TAG_DECLARE(Player);

#define RESOURCES    "resources/"
#define PLAYER_SPEED 300.0f
#define BULLET_SPEED 600.0f
#define BULLET_SIZE  10.0f

static bool sdl_init(SDL_Window **window, SDL_Renderer **renderer) {
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

int game_run(void) {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    if (!sdl_init(&window, &renderer)) return 1;

    /* Load textures */
    const char *base = SDL_GetBasePath();
    char tex_path[512];
    snprintf(tex_path, sizeof(tex_path), "%s" RESOURCES "player-ship.png", base);
    SDL_Texture *player_tex = load_texture(renderer, tex_path);
    snprintf(tex_path, sizeof(tex_path), "%s" RESOURCES "enemy-ufo.png", base);
    SDL_Texture *enemy_tex  = load_texture(renderer, tex_path);

    /* ECS world */
    ecs_world_t *world = ecs_init();
    ECS_COMPONENT_DEFINE(world, Position);
    ECS_COMPONENT_DEFINE(world, Size);
    ECS_COMPONENT_DEFINE(world, Velocity);
    ECS_COMPONENT_DEFINE(world, BoxCollider);
    ECS_COMPONENT_DEFINE(world, Sprite);
    ECS_COMPONENT_DEFINE(world, Health);
    ECS_TAG_DEFINE(world, Projectile);
    ECS_TAG_DEFINE(world, Enemy);
    ECS_TAG_DEFINE(world, EnemyProjectile);
    ECS_TAG_DEFINE(world, Player);

    /* Systems */
    render_system_init(window, renderer, world);
    physics_system_init(world);
    collision_system_init(world);
    input_system_init();

    /* Debug query (entity inspector) */
    ecs_query_t *debug_q = ecs_query(world, {
        .terms = { { ecs_id(Position) } }
    });

    /* Initial entities */
    ecs_entity_t player       = player_spawn(world, 350.0f, 650.0f, player_tex);
    ecs_entity_t enemy        = enemy_spawn(world, 250.0f, 100.0f, enemy_tex);
    ecs_entity_t bullet       = 0;
    ecs_entity_t enemy_bullet = 0;
    float enemy_shoot_timer   = ENEMY_SHOOT_INTERVAL;
    float respawn_timer       = 0.0f;

    InputActions input   = { 0 };
    bool         running = true;
    SDL_Event    event;
    Uint64       last_ticks = SDL_GetTicks();

    while (running) {
        Uint64 now = SDL_GetTicks();
        float  dt  = (now - last_ticks) / 1000.0f;
        last_ticks = now;

        /* --- Phase 1: Input collection --- */
        input_system_reset_actions(&input);
        while (SDL_PollEvent(&event)) {
            SDL_ConvertEventToRenderCoordinates(renderer, &event);
            render_system_handle_event(&event);
            input_system_process_event(&event, &input);
            if (event.type == SDL_EVENT_QUIT) running = false;
        }
        render_system_end_input();

        /* --- Phase 2: Game logic --- */

        /* Player movement */
        input_system_update_player(world, player, PLAYER_SPEED);

        /* Player shoot */
        if (input.shoot_pressed && bullet == 0 && player != 0) {
            const Position *pp = ecs_get(world, player, Position);
            const Size     *ps = ecs_get(world, player, Size);
            float bx = pp->x + ps->w / 2.0f - BULLET_SIZE / 2.0f;
            float by = pp->y - BULLET_SIZE;
            bullet = bullet_spawn(world, "Bullet", bx, by, -BULLET_SPEED, Projectile);
        }

        /* Player respawn */
        if (player == 0) {
            respawn_timer -= dt;
            if (respawn_timer <= 0.0f)
                player = player_spawn(world, 350.0f, 650.0f, player_tex);
        }


        /* --- Phase 3: Systems --- */
        physics_system_update(world, dt);

        ecs_entity_t killed[64];
        int nkilled = collision_system_update(world, killed, 64);
        for (int i = 0; i < nkilled; i++) {
            if (killed[i] == bullet)       bullet = 0;
            if (killed[i] == enemy)        enemy  = 0;
            if (killed[i] == enemy_bullet) enemy_bullet = 0;
            if (killed[i] == player)     { player = 0; respawn_timer = 3.0f; }
        }

        /* --- Phase 4: Render --- */
        render_system_clear();
        render_system_update(world);
        draw_entity_inspector(render_system_get_ctx(), world, debug_q);
        render_system_present();
    }

    /* Shutdown — order matters: systems before world, textures before renderer */
    ecs_query_fini(debug_q);
    collision_system_shutdown();
    physics_system_shutdown();
    render_system_shutdown();
    ecs_fini(world);
    SDL_DestroyTexture(player_tex);
    SDL_DestroyTexture(enemy_tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
