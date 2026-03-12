#include "../platform.h"

#include "../components/input.h"

#include "input.h"

#include "../defines.h"

//==============================================================================

void process_input(ecs_iter_t *it)
{
  
    Input *input = ecs_singleton_get_mut(it->world, Input);
    *input = (Input){0};

#ifdef DEBUG
  input->toggle_debug = is_key_pressed(KEY_TAB);
#endif

}
