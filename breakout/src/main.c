#include <SDL3/SDL.h>

bool is_running = true;
SDL_Window* window;
SDL_Renderer* renderer; 

static void InitWindow();
static void HandleInput();
static void Update();
static void Render();
static void Shutdown();

int main(int argc, char* argv[]) 
{
    InitWindow();

    while (is_running) 
    {
        HandleInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}

void InitWindow()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0) 
    {
    //Logger::Err("Error initializing SDL.");

        return;
    }
    window = SDL_CreateWindow("breakout", 1280, 720, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL); // Create renderer
}

void HandleInput()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) 
    {
        // Handle core SDL events (close window, key pressed, etc.)
        switch (e.type) 
        {
            case SDL_EVENT_QUIT:
                is_running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (e.key.key == SDLK_ESCAPE) { }
                if (e.key.key == SDLK_F1) { }
                //eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.key);
                break;
        }
    }
}

void Update()
{

}

void Render()
{
    // Drawing code
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    SDL_FRect rect = { 100, 100, 200, 150 }; // x, y, width, height
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red rectangle
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);
}

void Shutdown()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}