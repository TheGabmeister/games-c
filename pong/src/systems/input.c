#include "../platform.h"

#include "../components/input.h"
#include "../components/debug.h"

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

    // F1 toggle debug colliders (edge-triggered)
    static bool prev_f1 = false;
    bool f1 = keys[SDL_SCANCODE_F1];
    if (f1 && !prev_f1)
    {
        Debug *debug = ecs_singleton_ensure(it->world, Debug);
        debug->show_colliders = !debug->show_colliders;
    }
    prev_f1 = f1;
}
