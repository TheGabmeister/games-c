#include "game.h"
#include "config.h"
#include "draw.h"
#include "input.h"
#include "window.h"
#include <SDL3/SDL.h>

static bool g_running;

static const Scene *g_current_scene;
static const Scene *g_next_scene;

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

        if (!window_poll_events()) g_running = false;
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

void game_set_scene(const Scene *scene)
{
    g_next_scene = scene;
}

void game_quit(void)
{
    g_running = false;
}
