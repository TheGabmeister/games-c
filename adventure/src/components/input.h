#ifndef INPUT_COMPONENT_H
#define INPUT_COMPONENT_H

#include <flecs.h>
#include "../platform.h"

typedef struct Input
{
    bool quit;
    bool toggle_pause;
    int move_p1;
    int move_p2;
} Input;

ECS_COMPONENT_DECLARE(Input);

#endif
