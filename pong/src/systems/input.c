#include "../platform.h"

#include "../components/input.h"

#include "input.h"

#include "../defines.h"

//==============================================================================

void process_input(ecs_iter_t *it)
{
    Input *input = ecs_singleton_get_mut(it->world, Input);
    *input = (Input){0};

    const bool *keys = SDL_GetKeyboardState(NULL);

    // Player 1: W and S
    if (keys[SDL_SCANCODE_W])
        input->move_p1 = -1;
    else if (keys[SDL_SCANCODE_S])
        input->move_p1 = 1;

    // Player 2: Up and Down arrows
    if (keys[SDL_SCANCODE_UP])
        input->move_p2 = -1;
    else if (keys[SDL_SCANCODE_DOWN])
        input->move_p2 = 1;
}
