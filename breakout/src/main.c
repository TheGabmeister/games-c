#include <SDL3/SDL.h>
#include <flecs.h>

typedef struct {
    double x;
    double y;
} Position, Velocity;

bool is_running = true;
SDL_Window* window;
SDL_Renderer* renderer; 
int ms_previous_frame = 0;

ecs_world_t *world;

static void InitWindow();
static void InitGame();

static void HandleInput();
static void Update();
static void Render();
static void Shutdown();

static void Move();

int main(int argc, char* argv[]) 
{
    InitWindow();
    InitGame();

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

void InitGame()
{
    /* Create the world */
    world = ecs_init();

    /* Register components */
    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);

    /* Register system */
    ECS_SYSTEM(world, Move, EcsOnUpdate, Position, Velocity);

    /* Register tags (components without a size) */
    ECS_TAG(world, Eats);
    ECS_TAG(world, Apples);
    ECS_TAG(world, Pears);

    /* Create an entity with name Bob, add Position and food preference */
    ecs_entity_t Bob = ecs_entity(world, { .name = "Bob" });
    ecs_set(world, Bob, Position, {0, 0});
    ecs_set(world, Bob, Velocity, {1, 2});
    ecs_add_pair(world, Bob, Eats, Apples);

    /* Run systems twice. Usually this function is called once per frame */
    ecs_progress(world, 0);
    ecs_progress(world, 0);

    /* See if Bob has moved (he has) */
    const Position *p = ecs_get(world, Bob, Position);
    printf("Bob's position is {%f, %f}\n", p->x, p->y);
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
    // delta time, the difference in ticks since the last frame, converted to seconds 
    double dt = (SDL_GetTicks() - ms_previous_frame) / 1000.0;

    // Store the "previous" frame time
    ms_previous_frame = SDL_GetTicks();
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
    ecs_fini(world);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

void Move(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 0);
    Velocity *v = ecs_field(it, Velocity, 1);

    /* Print the set of components for the iterated over entities */
    char *type_str = ecs_table_str(it->world, it->table);
    printf("Move entities with [%s]\n", type_str);
    ecs_os_free(type_str);

    /* Iterate entities for the current table */
    for (int i = 0; i < it->count; i ++) {
        p[i].x += v[i].x;
        p[i].y += v[i].y;
    }
}