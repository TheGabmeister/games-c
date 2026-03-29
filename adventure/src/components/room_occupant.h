#ifndef ROOM_OCCUPANT_COMPONENT_H
#define ROOM_OCCUPANT_COMPONENT_H

#include <flecs.h>

typedef struct RoomOccupant
{
    int room_id;
} RoomOccupant;

ECS_COMPONENT_DECLARE(RoomOccupant);

#endif
