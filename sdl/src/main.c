#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "platform.h"

int main(int argc, char *argv[])
{
    init_window(600, 800, "template");

    SDL_Log("Hello, SDL3 World!");
    SDL_Log("SDL3_image version: %d.%d.%d", SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_MICRO_VERSION);
    SDL_Log("SDL3_mixer version: %d.%d.%d", SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_MICRO_VERSION);
    SDL_Log("SDL3_ttf version: %d.%d.%d", SDL_TTF_MAJOR_VERSION, SDL_TTF_MINOR_VERSION, SDL_TTF_MICRO_VERSION);

    SDL_Event event;

    if (!is_window_ready())
        return 1;

    SDL_Renderer *renderer = get_renderer();

    while (!window_should_close()) {
        engine_begin_frame();
        while (SDL_PollEvent(&event))
        {
            engine_process_event(&event);
        }

        float delta = get_deltatime();

        SDL_SetRenderDrawColor(renderer, 25, 25, 50, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_FRect rect = {300.0f, 225.0f, 200.0f, 150.0f};
        SDL_RenderFillRect(renderer, &rect);

        SDL_RenderPresent(renderer);
    }

    close_window();
    return 0;
}
