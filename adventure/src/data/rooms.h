#ifndef ROOMS_H
#define ROOMS_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/*=============================================================================
 * Direction enum — canonical order used everywhere
 *===========================================================================*/
typedef enum Direction {
    DIR_NORTH = 0,
    DIR_EAST  = 1,
    DIR_SOUTH = 2,
    DIR_WEST  = 3,
    DIR_COUNT = 4
} Direction;

/*=============================================================================
 * Room data structures
 *===========================================================================*/
#define ROOM_NONE (-1)
#define MAX_ROOM_WALLS 16
#define MAX_ROOM_BRIDGE_SOCKETS 4
#define TOTAL_ROOMS 30

typedef struct WallRect {
    float x, y, w, h;
} WallRect;

typedef struct RoomExit {
    int   destination_room;
    float span_min;
    float span_max;
} RoomExit;

typedef struct BridgeSocket {
    Direction wall;
    float     span_min;
    float     span_max;
} BridgeSocket;

typedef enum RoomFlags {
    ROOM_FLAG_NONE   = 0,
    ROOM_FLAG_DARK   = 1 << 0,
    ROOM_FLAG_CASTLE = 1 << 1,
    ROOM_FLAG_SECRET = 1 << 2
} RoomFlags;

typedef struct RoomDef {
    int           id;
    const char   *name;
    RoomExit      exits[DIR_COUNT];
    WallRect      interior_walls[MAX_ROOM_WALLS];
    int           interior_wall_count;
    BridgeSocket  bridge_sockets[MAX_ROOM_BRIDGE_SOCKETS];
    int           bridge_socket_count;
    RoomFlags     flags;
    SDL_Color     ambient_tint;
} RoomDef;

/*=============================================================================
 * Room ID constants — stable across all game modes
 *===========================================================================*/
enum {
    ROOM_GOLD_ENTRANCE     =  0,
    ROOM_GOLD_THRONE       =  1,
    ROOM_GOLD_COURTYARD    =  2,
    ROOM_GOLD_ARMORY       =  3,
    ROOM_OVERWORLD_NORTH   =  4,
    ROOM_OVERWORLD_SOUTH   =  5,
    ROOM_CROSSROADS        =  6,
    ROOM_EASTERN_PATH      =  7,
    ROOM_WESTERN_PATH      =  8,
    ROOM_RIVER_CROSSING    =  9,
    ROOM_FOREST_EDGE       = 10,
    ROOM_MOUNTAIN_PASS     = 11,
    ROOM_VALLEY            = 12,
    ROOM_ANCIENT_RUINS     = 13,
    ROOM_SWAMP_EDGE        = 14,
    ROOM_BLACK_GATE        = 15,
    ROOM_BLACK_HALL        = 16,
    ROOM_BLACK_DUNGEON     = 17,
    ROOM_BLACK_TREASURY    = 18,
    ROOM_WHITE_GATE        = 19,
    ROOM_WHITE_HALL        = 20,
    ROOM_WHITE_TOWER       = 21,
    ROOM_WHITE_LIBRARY     = 22,
    ROOM_CATACOMBS_ENTRY   = 23,
    ROOM_CATACOMBS_1       = 24,
    ROOM_CATACOMBS_2       = 25,
    ROOM_CATACOMBS_3       = 26,
    ROOM_CATACOMBS_DEAD    = 27,
    ROOM_CATACOMBS_CHAMBER = 28,
    ROOM_SECRET            = 29
};

/*=============================================================================
 * BFS room distance table
 *===========================================================================*/
extern int room_distances[TOTAL_ROOMS][TOTAL_ROOMS];

void     rooms_compute_distances(void);
int      rooms_bfs_next_room(int from, int to);

/*=============================================================================
 * Room data access
 *===========================================================================*/
extern const RoomDef g_rooms[TOTAL_ROOMS];

const RoomDef *room_get(int id);

#endif
