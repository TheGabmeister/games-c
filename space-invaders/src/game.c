#include "game.h"
#include "game_state.h"
#include "tags.h"
#include "defines.h"
#include "texture.h"
#include "settings.h"
#include "score.h"
#include "managers/asset.h"
#include "managers/audio.h"
#include "managers/prefab.h"
#include "managers/component.h"
#include "components/health.h"
#include "systems/renderer.h"
#include "systems/input.h"
#include "systems/movement.h"
#include "systems/combat.h"
#include "systems/enemy_movement.h"
#include "systems/boundary.h"
#include "systems/collision.h"
#include "systems/gui.h"
#include <string.h>

/* SDL and ECS handles — private to game.c */
static SDL_Window   *window   = NULL;
static SDL_Renderer *renderer = NULL;
static ecs_world_t  *world    = NULL;

/* Tags — non-static: systems access them via tags.h externs */
ecs_entity_t Player     = 0;
ecs_entity_t Enemy      = 0;
ecs_entity_t Projectile = 0;
ecs_entity_t Shooting   = 0;

static bool      isRunning  = false;
static GameState game_state = GAME_STATE_MENU;

/* Query used only for win-condition counting; excludes prefabs automatically. */
static ecs_query_t *enemy_count_query  = NULL;
/* Query to read player lives (Health component). */
static ecs_query_t *player_health_query = NULL;

/* ------------------------------------------------------------------ */
/* Text rendering helpers                                               */
/* ------------------------------------------------------------------ */

/* Render text centered horizontally at the given screen-space y.
 * SDL_RenderDebugText uses an 8x8 pixel font; scale multiplies that. */
static void render_text_centered(float scale, float screen_y, const char *text)
{
    float text_w_screen = (float)strlen(text) * 8.0f * scale;
    float screen_x = ((float)WINDOW_WIDTH - text_w_screen) * 0.5f;
    SDL_SetRenderScale(renderer, scale, scale);
    SDL_RenderDebugText(renderer, screen_x / scale, screen_y / scale, text);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}

/* ------------------------------------------------------------------ */
/* Per-state render functions                                           */
/* ------------------------------------------------------------------ */

static void render_menu()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    render_text_centered(4.0f, 160.0f, "SPACE INVADERS");

    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    render_text_centered(2.0f, 290.0f, "PRESS ENTER TO PLAY");
    render_text_centered(2.0f, 325.0f, "ESC TO QUIT");
}

static void render_game_over()
{
    /* Frozen world is already drawn — overlay the result text. */
    SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
    render_text_centered(4.0f, 180.0f, "GAME OVER");

    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    render_text_centered(2.0f, 310.0f, "ENTER: RESTART   ESC: QUIT");
}

static void render_win()
{
    SDL_SetRenderDrawColor(renderer, 50, 220, 50, 255);
    render_text_centered(4.0f, 180.0f, "YOU WIN!");

    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    render_text_centered(2.0f, 310.0f, "ENTER: PLAY AGAIN   ESC: QUIT");
}

static void render_hud()
{
    /* Read player lives from the ECS. */
    int lives = 0;
    ecs_iter_t it = ecs_query_iter(world, player_health_query);
    while (ecs_query_next(&it)) {
        Health *h = ecs_field(&it, Health, 1);
        if (it.count > 0) lives = h[0].current;
    }

    char buf[32];
    const float scale = 1.5f;
    const float char_w = 8.0f * scale;
    const float y_screen = 8.0f;

    SDL_SetRenderScale(renderer, scale, scale);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    /* Left: score */
    SDL_snprintf(buf, sizeof(buf), "SCORE: %05d", score_get());
    SDL_RenderDebugText(renderer, 8.0f / scale, y_screen / scale, buf);

    /* Center: hi-score */
    SDL_snprintf(buf, sizeof(buf), "HI: %05d", score_get_high());
    float hi_w = (float)strlen(buf) * char_w;
    SDL_RenderDebugText(renderer,
        ((float)WINDOW_WIDTH - hi_w) * 0.5f / scale,
        y_screen / scale, buf);

    /* Right: lives */
    SDL_snprintf(buf, sizeof(buf), "LIVES: %d", lives);
    float lives_w = (float)strlen(buf) * char_w;
    SDL_RenderDebugText(renderer,
        ((float)WINDOW_WIDTH - lives_w - 8.0f) / scale,
        y_screen / scale, buf);

    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}

/* ------------------------------------------------------------------ */
/* Forward declarations                                                 */
/* ------------------------------------------------------------------ */

static void setup_window();
static void load_level();
static void render_hud();

/* ------------------------------------------------------------------ */
/* World lifecycle (safe to call multiple times for restart)           */
/* ------------------------------------------------------------------ */

static void world_init()
{
    load_level();
    enemy_count_query   = ecs_query(world, { .terms = { { .id = Enemy } } });
    /* Terms: [Player(tag), Health] — indices 0 (tag), 1 */
    player_health_query = ecs_query(world, {
        .terms = { { .id = Player }, { .id = ecs_id(Health) } }
    });
    renderer_system_init(world);
    input_system_init(world);
    movement_system_init(world);
    combat_system_init(world);
    enemy_movement_system_init(world);
    boundary_system_init(world);
    collision_system_init(world);
}

static void world_fini()
{
    input_system_destroy();
    combat_system_destroy();
    movement_system_destroy();
    enemy_movement_system_destroy();
    boundary_system_destroy();
    collision_system_destroy();
    renderer_system_destroy();
    ecs_query_fini(enemy_count_query);
    enemy_count_query = NULL;
    ecs_query_fini(player_health_query);
    player_health_query = NULL;
    ecs_fini(world);
    world = NULL;
    /* Free textures after the world (and all Sprite components) are gone. */
    asset_manager_destroy();
}

/* ------------------------------------------------------------------ */
/* State transitions                                                    */
/* ------------------------------------------------------------------ */

static void start_game()
{
    score_reset();
    if (world) world_fini();
    world_init();
    game_state = GAME_STATE_PLAYING;
}

/* Check win/lose conditions; only call during GAME_STATE_PLAYING. */
static void check_game_conditions()
{
    /* Count live enemy instances via a query — ecs_count_id is not used here
     * because it includes the EnemyPrefab entity itself, which also carries
     * the Enemy tag, so it would never reach zero. Queries exclude prefabs. */
    int enemy_count = 0;
    ecs_iter_t it = ecs_query_iter(world, enemy_count_query);
    while (ecs_query_next(&it)) enemy_count += it.count;

    if (enemy_count == 0) {
        score_update_high();
        game_state = GAME_STATE_WIN;
        return;
    }
    if (enemy_movement_system_reached_bottom()) {
        score_update_high();
        game_state = GAME_STATE_GAME_OVER;
        return;
    }
    /* Player out of lives */
    ecs_iter_t pit = ecs_query_iter(world, player_health_query);
    while (ecs_query_next(&pit)) {
        Health *h = ecs_field(&pit, Health, 1);
        for (int i = 0; i < pit.count; i++) {
            if (h[i].current <= 0) {
                score_update_high();
                game_state = GAME_STATE_GAME_OVER;
                ecs_iter_fini(&pit);
                return;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

void game_init()
{
    setup_window();
    score_load_high();
    audio_manager_init();
    gui_system_init(window, renderer);
    /* Level loads when the player presses ENTER from the menu. */
}

void game_run()
{
    SDL_Event event;
    Uint64 last_ticks = SDL_GetTicks();
    isRunning = true;

    while (isRunning)
    {
        Uint64 now = SDL_GetTicks();
        float dt = (now - last_ticks) / 1000.0f;
        if (dt > 0.05f) dt = 0.05f;
        last_ticks = now;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
                break;
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                SDL_Scancode sc = event.key.scancode;
                switch (game_state) {
                    case GAME_STATE_MENU:
                        if (sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_KP_ENTER)
                            start_game();
                        else if (sc == SDL_SCANCODE_ESCAPE)
                            isRunning = false;
                        break;
                    case GAME_STATE_PLAYING:
                        if (sc == SDL_SCANCODE_ESCAPE)
                            isRunning = false;
                        break;
                    case GAME_STATE_GAME_OVER:
                    case GAME_STATE_WIN:
                        if (sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_KP_ENTER)
                            start_game();
                        else if (sc == SDL_SCANCODE_ESCAPE)
                            isRunning = false;
                        break;
                }
            }

            if (game_state == GAME_STATE_PLAYING)
                gui_system_handle_event(&event);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        switch (game_state) {
            case GAME_STATE_MENU:
                render_menu();
                break;

            case GAME_STATE_PLAYING:
                input_system_run(world);
                combat_system_run(world);
                movement_system_run(world, dt);
                enemy_movement_system_run(world);
                boundary_system_run(world);
                collision_system_run(world);
                check_game_conditions();

                renderer_system_run(world, renderer);
                gui_system_run(world);
                render_hud();
                break;

            case GAME_STATE_GAME_OVER:
                renderer_system_run(world, renderer);
                render_game_over();
                render_hud();
                break;

            case GAME_STATE_WIN:
                renderer_system_run(world, renderer);
                render_win();
                render_hud();
                break;
        }

        SDL_RenderPresent(renderer);
    }
}

void game_destroy()
{
    if (world) world_fini();
    score_save_high();
    gui_system_destroy();
    audio_manager_destroy();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void setup_window()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
}

static void load_level()
{
    char path[512];
    snprintf(path, sizeof(path), "%s%s", ASSETS_DIR, "level_01.json");
    FILE *f = fopen(path, "rb");
    if (!f) {
        SDL_Log("load_level: failed to open level_01.json");
        return;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    fclose(f);
    buf[len] = '\0';

    cJSON *level_json = cJSON_Parse(buf);
    free(buf);

    if (!level_json) {
        SDL_Log("load_level: cJSON_Parse failed: %s", cJSON_GetErrorPtr());
        return;
    }

    /* Phase 1: load textures into asset_manager */
    cJSON *assets = cJSON_GetObjectItemCaseSensitive(level_json, "assets");
    cJSON *asset;
    cJSON_ArrayForEach(asset, assets) {
        cJSON *type = cJSON_GetObjectItemCaseSensitive(asset, "type");
        if (!cJSON_IsString(type) || strcmp(type->valuestring, "texture") != 0)
            continue;

        cJSON *file = cJSON_GetObjectItemCaseSensitive(asset, "file");
        if (!cJSON_IsString(file))
            continue;

        cJSON *id = cJSON_GetObjectItemCaseSensitive(asset, "id");
        if (!cJSON_IsString(id))
            continue;

        SDL_Texture *tex = load_texture(renderer, file->valuestring);
        if (tex)
            asset_manager_add(id->valuestring, tex);
    }

    /* Phase 2: init ECS world, components, tags, and prefabs */
    world      = ecs_init();
    Player     = ecs_new(world);
    Enemy      = ecs_new(world);
    Projectile = ecs_new(world);
    Shooting   = ecs_new(world);
    component_manager_init(world);
    prefab_manager_init(world);

    /* Phase 3: instantiate entities from JSON */
    cJSON *entities = cJSON_GetObjectItemCaseSensitive(level_json, "entities");
    cJSON *entry;
    cJSON_ArrayForEach(entry, entities) {
        cJSON *prefab_name = cJSON_GetObjectItemCaseSensitive(entry, "prefab");
        if (!cJSON_IsString(prefab_name)) {
            SDL_Log("load_level: entry missing 'prefab' key, skipping");
            continue;
        }

        cJSON *overrides = cJSON_GetObjectItemCaseSensitive(entry, "overrides");
        if (overrides) {
            prefab_instantiate(world, prefab_name->valuestring, overrides);
        } else {
            cJSON *empty = cJSON_CreateObject();
            prefab_instantiate(world, prefab_name->valuestring, empty);
            cJSON_Delete(empty);
        }
    }

    cJSON_Delete(level_json);
}
