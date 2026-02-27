#include "game.h"

ecs_entity_t Player;


void game_init()
{
    setup_window();
    load_level();
    spawn_entities();

    renderer_system_init(world);
    input_system_init(world);
    movement_system_init(world);
}

void spawn_entities()
{
    world = ecs_init();
    ECS_COMPONENT_DEFINE(world, Transform);
    ECS_COMPONENT_DEFINE(world, Sprite);
    ECS_COMPONENT_DEFINE(world, Velocity);
    Player = ecs_new(world);

    ecs_entity_t player = ecs_new(world);
    ecs_add_id(world, player, Player);
    ecs_set(world, player, Transform, {
        .position = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 80.0f},
        .rotation = 0.0f,
        .scale    = {1.0f, 1.0f}
    });
    ecs_set(world, player, Velocity, { .x = 0.0f, .y = 0.0f });
    ecs_set(world, player, Sprite, {
        .texture = asset_manager_get("player-ship"),
        .color   = {255, 255, 255, 255}
    });

    
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
            if (event.type == SDL_EVENT_QUIT || event.key.scancode == SDL_SCANCODE_ESCAPE)
                isRunning = false;
        }

        input_system_run(world);
        movement_system_run(world, dt);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderer_system_run(world, renderer);

        SDL_RenderPresent(renderer);
    }
}

void game_destroy()
{
    input_system_destroy();
    movement_system_destroy();
    renderer_system_destroy();
    ecs_fini(world);
    asset_manager_destroy();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void setup_window()
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

void load_level()
{
    FILE *f = fopen(ASSETS "level_01.json", "rb");
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

    cJSON *json = cJSON_Parse(buf);
    free(buf);

    if (!json) {
        SDL_Log("load_level: cJSON_Parse failed: %s", cJSON_GetErrorPtr());
        return;
    }

    cJSON *assets = cJSON_GetObjectItemCaseSensitive(json, "assets");
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

    cJSON_Delete(json);
}


