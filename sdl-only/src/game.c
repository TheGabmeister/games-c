#include "game.h"
#include "config.h"
#include "draw.h"
#include "input.h"
#include <SDL3/SDL.h>

static SDL_Window *g_window;
static SDL_Renderer *g_renderer;
static bool g_running;

static const Scene *g_current_scene;
static const Scene *g_next_scene;

bool game_init(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    g_window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!g_window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    g_renderer = SDL_CreateRenderer(g_window, NULL);
    if (!g_renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return false;
    }

    SDL_SetRenderVSync(g_renderer, 1);
    draw_init(g_renderer);

    return true;
}

static void transition_scene(void)
{
    if (!g_next_scene) return;

    if (g_current_scene && g_current_scene->shutdown) {
        g_current_scene->shutdown();
    }

    g_current_scene = g_next_scene;
    g_next_scene = NULL;

    if (g_current_scene->init) {
        if (!g_current_scene->init()) {
            SDL_Log("Scene init failed");
            g_running = false;
        }
    }
}

void game_run(const Scene *initial_scene)
{
    g_running = true;
    g_next_scene = initial_scene;
    transition_scene();

    Uint64 prev_ticks = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();
    float accumulator = 0.0f;

    while (g_running) {
        /* Delta time */
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)(now - prev_ticks) / (float)freq;
        prev_ticks = now;
        if (dt > MAX_FRAME_DT) dt = MAX_FRAME_DT;
        accumulator += dt;

        /* Events */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                g_running = false;
            }
        }

        /* Input snapshot */
        input_update();

        /* Fixed timestep updates */
        while (accumulator >= FIXED_DT) {
            if (g_current_scene && g_current_scene->update) {
                g_current_scene->update(FIXED_DT);
            }
            accumulator -= FIXED_DT;
        }

        /* Render */
        if (g_current_scene && g_current_scene->render) {
            g_current_scene->render();
        }
        draw_present();

        /* Deferred scene transition */
        if (g_next_scene) {
            transition_scene();
        }
    }

    /* Shutdown current scene */
    if (g_current_scene && g_current_scene->shutdown) {
        g_current_scene->shutdown();
    }
    g_current_scene = NULL;
}

void game_shutdown(void)
{
    if (g_renderer) SDL_DestroyRenderer(g_renderer);
    if (g_window) SDL_DestroyWindow(g_window);
    SDL_Quit();
    g_renderer = NULL;
    g_window = NULL;
}

void game_set_scene(const Scene *scene)
{
    g_next_scene = scene;
}

void game_quit(void)
{
    g_running = false;
}
