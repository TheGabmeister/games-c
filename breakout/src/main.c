//#include "./game.h"
#include <SDL3/SDL.h>

bool is_running = true;
SDL_Window* window;
static void Destroy();

int main(int argc, char* argv[]) 
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0) 
    {
    //Logger::Err("Error initializing SDL.");

        return;
    }
    window = SDL_CreateWindow("breakout", 1280, 720, SDL_WINDOW_RESIZABLE);

    while (is_running) 
    {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            

            // Handle core SDL events (close window, key pressed, etc.)
            switch (e.type) {
                case SDL_EVENT_QUIT:
                    is_running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (e.key.key == SDLK_ESCAPE) {

                    }
                    if (e.key.key == SDLK_F1) {

                    }
                    //eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.key);
                    break;
            }
        }
    }

    Destroy();
    return 0;
}

void Destroy()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}