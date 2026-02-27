#include "game.h"


void game_init()
{
    setup_window();
    load_level();
    spawn_entities();

}

void spawn_entites()
{
    world = ecs_init();
    ECS_COMPONENT_DEFINE(world, Transform);
    ECS_COMPONENT_DEFINE(world, Sprite);

    ecs_entity_t player = ecs_new(world);
    ecs_set(world, player, Transform, {
        .position = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 80.0f},
        .rotation = 0.0f,
        .scale    = {1.0f, 1.0f}
    });
    ecs_set(world, player, Sprite, {
        .texture = asset_registry_get("player-ship"),
        .color   = {255, 255, 255, 255}
    });

    render_query = ecs_query(world, {
        .terms = {{ ecs_id(Transform) }, { ecs_id(Sprite) }}
    });
}

void game_run()
{
    SDL_Event event;
    while (true)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN)
                return;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        ecs_iter_t it = ecs_query_iter(world, render_query);
        while (ecs_query_next(&it))
        {
            Transform *transforms = ecs_field(&it, Transform, 0);
            Sprite    *sprites    = ecs_field(&it, Sprite,    1);
            for (int i = 0; i < it.count; i++)
            {
                if (!sprites[i].texture) continue;
                float tw, th;
                SDL_GetTextureSize(sprites[i].texture, &tw, &th);
                SDL_FRect dst = {
                    transforms[i].position[0] - tw * transforms[i].scale[0] * 0.5f,
                    transforms[i].position[1] - th * transforms[i].scale[1] * 0.5f,
                    tw * transforms[i].scale[0],
                    th * transforms[i].scale[1]
                };
                SDL_RenderTexture(renderer, sprites[i].texture, NULL, &dst);
            }
        }

        SDL_RenderPresent(renderer);
    }
}

void game_destroy()
{
    ecs_query_fini(render_query);
    ecs_fini(world);
    asset_registry_destroy();
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
            asset_registry_add(id->valuestring, tex);
    }

    cJSON_Delete(json);
}


