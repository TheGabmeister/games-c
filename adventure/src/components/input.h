#ifndef INPUT_COMPONENT_H
#define INPUT_COMPONENT_H

#include <flecs.h>
#include <stdbool.h>

typedef struct Input
{
    bool quit;
    bool pause_pressed;
    bool interact_pressed;
    bool debug_toggle_collision_pressed;
    bool debug_toggle_room_info_pressed;
    bool debug_toggle_entity_list_pressed;
    float move_x;
    float move_y;
} Input;

ECS_COMPONENT_DECLARE(Input);

#endif
