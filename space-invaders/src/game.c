#include "game.h"


void game_init()
{
    setup_window();
    load_level();
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
    }

}

void game_destroy()
{
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