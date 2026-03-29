#include "../platform.h"
#include "../components/input.h"
#include "../defines.h"
#include "../game.h"
#include "input.h"

/*=============================================================================
 * process_input — read keyboard state into Input singleton
 *===========================================================================*/
void process_input(ecs_iter_t *it)
{
    Input *input = ecs_singleton_get_mut(it->world, Input);

    /* Clear edge-triggered fields */
    input->pause_pressed = false;
    input->interact_pressed = false;
    input->debug_toggle_collision_pressed = false;
    input->debug_toggle_room_info_pressed = false;
    input->debug_toggle_entity_list_pressed = false;
    input->move_x = 0.0f;
    input->move_y = 0.0f;

    const bool *keys = SDL_GetKeyboardState(NULL);

    /* Movement — held keys */
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
        input->move_y = -1.0f;
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
        input->move_y =  1.0f;
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
        input->move_x = -1.0f;
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
        input->move_x =  1.0f;

    /* Edge-triggered keys */
    static bool prev_space = false, prev_enter = false;
    static bool prev_esc = false, prev_p = false;
    static bool prev_f1 = false, prev_f2 = false, prev_f3 = false;

    bool space = keys[SDL_SCANCODE_SPACE];
    bool enter = keys[SDL_SCANCODE_RETURN];
    bool esc   = keys[SDL_SCANCODE_ESCAPE];
    bool p     = keys[SDL_SCANCODE_P];
    bool f1    = keys[SDL_SCANCODE_F1];
    bool f2    = keys[SDL_SCANCODE_F2];
    bool f3    = keys[SDL_SCANCODE_F3];

    if ((space && !prev_space) || (enter && !prev_enter))
        input->interact_pressed = true;
    if ((esc && !prev_esc) || (p && !prev_p))
        input->pause_pressed = true;
    if (f1 && !prev_f1)
        input->debug_toggle_collision_pressed = true;
    if (f2 && !prev_f2)
        input->debug_toggle_room_info_pressed = true;
    if (f3 && !prev_f3)
        input->debug_toggle_entity_list_pressed = true;

    prev_space = space;
    prev_enter = enter;
    prev_esc   = esc;
    prev_p     = p;
    prev_f1    = f1;
    prev_f2    = f2;
    prev_f3    = f3;
}
