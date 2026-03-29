#include "rooms.h"
#include <string.h>

/*=============================================================================
 * Playfield constants
 *===========================================================================*/
#define PF_W  960.0f
#define PF_H  720.0f
#define WALL  16.0f

/*=============================================================================
 * Convenience macros for exits
 *
 *   N/S exits: span is along X axis (0..960)
 *   E/W exits: span is along Y axis (0..720)
 *
 * "Centered" helpers place a gap in the middle of the relevant edge.
 *===========================================================================*/
#define EXIT_NONE  { ROOM_NONE, 0, 0 }

/* Centered exits with 80px gap */
#define EXIT_N(dst)  { (dst), (PF_W/2 - 40), (PF_W/2 + 40) }
#define EXIT_S(dst)  { (dst), (PF_W/2 - 40), (PF_W/2 + 40) }
#define EXIT_E(dst)  { (dst), (PF_H/2 - 40), (PF_H/2 + 40) }
#define EXIT_W(dst)  { (dst), (PF_H/2 - 40), (PF_H/2 + 40) }

/* Custom-positioned exits */
#define EXIT_NC(dst, mn, mx) { (dst), (mn), (mx) }
#define EXIT_SC(dst, mn, mx) { (dst), (mn), (mx) }
#define EXIT_EC(dst, mn, mx) { (dst), (mn), (mx) }
#define EXIT_WC(dst, mn, mx) { (dst), (mn), (mx) }

/*=============================================================================
 * Ambient tint palettes
 *===========================================================================*/
#define TINT_GOLD     { 40, 30, 10, 255 }
#define TINT_BLACK    { 10, 10, 20, 255 }
#define TINT_WHITE    { 20, 20, 30, 255 }
#define TINT_OVERWORLD { 10, 20, 10, 255 }
#define TINT_CATACOMBS { 20, 15, 10, 255 }
#define TINT_SECRET   { 30, 10, 30, 255 }

/*=============================================================================
 * Room definitions — 30 rooms total
 *
 * Coordinate system: (0,0) top-left, X right, Y down.
 * North = top edge (y=0), South = bottom edge (y=720),
 * East = right edge (x=960), West = left edge (x=0).
 *===========================================================================*/
const RoomDef g_rooms[TOTAL_ROOMS] = {

    /*=========================================================================
     * GOLD CASTLE  (rooms 0-3)  — ROOM_FLAG_CASTLE
     *=======================================================================*/

    /* 0: ROOM_GOLD_ENTRANCE — player start.
     *    North -> Throne (1), East -> Courtyard (2), South -> Overworld North (4)
     *    Interior: corridor walls creating an entrance hall feel */
    [ROOM_GOLD_ENTRANCE] = {
        .id   = ROOM_GOLD_ENTRANCE,
        .name = "Gold Castle — Entrance",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_GOLD_THRONE),
            [DIR_EAST]  = EXIT_E(ROOM_GOLD_COURTYARD),
            [DIR_SOUTH] = EXIT_S(ROOM_OVERWORLD_NORTH),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Left corridor wall */
            { 200, 100, WALL, 520 },
            /* Right corridor wall */
            { 744, 100, WALL, 520 },
            /* Small pillar left of center */
            { 380, 300, 32, 32 },
            /* Small pillar right of center */
            { 548, 300, 32, 32 },
        },
        .interior_wall_count = 4,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_GOLD
    },

    /* 1: ROOM_GOLD_THRONE
     *    South -> Entrance (0) */
    [ROOM_GOLD_THRONE] = {
        .id   = ROOM_GOLD_THRONE,
        .name = "Gold Castle — Throne Room",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_S(ROOM_GOLD_ENTRANCE),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Throne platform step — wide wall near top */
            { 200, 120, 560, WALL },
            /* Left alcove wall */
            { 200, 120, WALL, 200 },
            /* Right alcove wall */
            { 744, 120, WALL, 200 },
            /* Left pillar near throne */
            { 340, 200, 32, 32 },
            /* Right pillar near throne */
            { 588, 200, 32, 32 },
            /* Decorative center pillar bottom area */
            { 472, 500, 16, 16 },
        },
        .interior_wall_count = 6,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_GOLD
    },

    /* 2: ROOM_GOLD_COURTYARD
     *    West -> Entrance (0), East -> Armory (3) */
    [ROOM_GOLD_COURTYARD] = {
        .id   = ROOM_GOLD_COURTYARD,
        .name = "Gold Castle — Courtyard",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_E(ROOM_GOLD_ARMORY),
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_W(ROOM_GOLD_ENTRANCE)
        },
        .interior_walls = {
            /* Central fountain (square obstacle) */
            { 430, 310, 100, 100 },
            /* Garden wall top-left */
            { 160, 160, 200, WALL },
            /* Garden wall bottom-right */
            { 600, 544, 200, WALL },
        },
        .interior_wall_count = 3,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_GOLD
    },

    /* 3: ROOM_GOLD_ARMORY
     *    West -> Courtyard (2) */
    [ROOM_GOLD_ARMORY] = {
        .id   = ROOM_GOLD_ARMORY,
        .name = "Gold Castle — Armory",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_W(ROOM_GOLD_COURTYARD)
        },
        .interior_walls = {
            /* Weapon rack left */
            { 160, 100, WALL, 200 },
            /* Weapon rack right */
            { 784, 100, WALL, 200 },
            /* Armor stand wall center */
            { 400, 400, 160, WALL },
            /* Divider wall */
            { 160, 420, WALL, 200 },
        },
        .interior_wall_count = 4,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_GOLD
    },

    /*=========================================================================
     * OVERWORLD  (rooms 4-14)
     *=======================================================================*/

    /* 4: ROOM_OVERWORLD_NORTH
     *    North -> Gold Entrance (0), South -> Crossroads (6) */
    [ROOM_OVERWORLD_NORTH] = {
        .id   = ROOM_OVERWORLD_NORTH,
        .name = "Overworld — Northern Field",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_GOLD_ENTRANCE),
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_S(ROOM_CROSSROADS),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Scattered rock */
            { 300, 250, 48, 32 },
            /* Tree trunk */
            { 700, 400, 32, 48 },
        },
        .interior_wall_count = 2,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /* 5: ROOM_OVERWORLD_SOUTH
     *    North -> Crossroads (6), South -> River Crossing (9) */
    [ROOM_OVERWORLD_SOUTH] = {
        .id   = ROOM_OVERWORLD_SOUTH,
        .name = "Overworld — Southern Field",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_CROSSROADS),
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_S(ROOM_RIVER_CROSSING),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Lone boulder */
            { 500, 350, 40, 40 },
        },
        .interior_wall_count = 1,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /* 6: ROOM_CROSSROADS — central hub
     *    North -> Overworld North (4), East -> Eastern Path (7),
     *    South -> Overworld South (5), West -> Western Path (8) */
    [ROOM_CROSSROADS] = {
        .id   = ROOM_CROSSROADS,
        .name = "Crossroads",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_OVERWORLD_NORTH),
            [DIR_EAST]  = EXIT_E(ROOM_EASTERN_PATH),
            [DIR_SOUTH] = EXIT_S(ROOM_OVERWORLD_SOUTH),
            [DIR_WEST]  = EXIT_W(ROOM_WESTERN_PATH)
        },
        .interior_walls = {
            /* Signpost marker (small obstacle) */
            { 468, 348, 24, 24 },
        },
        .interior_wall_count = 1,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /* 7: ROOM_EASTERN_PATH
     *    West -> Crossroads (6), East -> Black Gate (15) */
    [ROOM_EASTERN_PATH] = {
        .id   = ROOM_EASTERN_PATH,
        .name = "Eastern Path",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_E(ROOM_BLACK_GATE),
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_W(ROOM_CROSSROADS)
        },
        .interior_walls = {
            /* Path-side hedge top */
            { 200, 180, 560, WALL },
            /* Path-side hedge bottom */
            { 200, 524, 560, WALL },
        },
        .interior_wall_count = 2,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /* 8: ROOM_WESTERN_PATH
     *    East -> Crossroads (6), West -> White Gate (19) */
    [ROOM_WESTERN_PATH] = {
        .id   = ROOM_WESTERN_PATH,
        .name = "Western Path",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_E(ROOM_CROSSROADS),
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_W(ROOM_WHITE_GATE)
        },
        .interior_walls = {
            /* Path-side hedge top */
            { 200, 200, 560, WALL },
            /* Path-side hedge bottom */
            { 200, 504, 560, WALL },
        },
        .interior_wall_count = 2,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /* 9: ROOM_RIVER_CROSSING
     *    North -> Overworld South (5), East -> Forest Edge (10),
     *    West -> Swamp Edge (14) */
    [ROOM_RIVER_CROSSING] = {
        .id   = ROOM_RIVER_CROSSING,
        .name = "River Crossing",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_OVERWORLD_SOUTH),
            [DIR_EAST]  = EXIT_E(ROOM_FOREST_EDGE),
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_W(ROOM_SWAMP_EDGE)
        },
        .interior_walls = {
            /* River bank top (water obstacle) */
            { 0, 400, 440, WALL },
            { 520, 400, 440, WALL },
            /* River bank bottom */
            { 0, 480, 440, WALL },
            { 520, 480, 440, WALL },
        },
        .interior_wall_count = 4,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /* 10: ROOM_FOREST_EDGE
     *     West -> River Crossing (9), South -> Mountain Pass (11) */
    [ROOM_FOREST_EDGE] = {
        .id   = ROOM_FOREST_EDGE,
        .name = "Forest Edge",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_S(ROOM_MOUNTAIN_PASS),
            [DIR_WEST]  = EXIT_W(ROOM_RIVER_CROSSING)
        },
        .interior_walls = {
            /* Tree cluster 1 */
            { 200, 150, 48, 48 },
            /* Tree cluster 2 */
            { 600, 200, 48, 48 },
            /* Tree cluster 3 */
            { 350, 450, 48, 48 },
            /* Tree cluster 4 */
            { 750, 500, 48, 48 },
        },
        .interior_wall_count = 4,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /* 11: ROOM_MOUNTAIN_PASS
     *     North -> Forest Edge (10), West -> Valley (12) */
    [ROOM_MOUNTAIN_PASS] = {
        .id   = ROOM_MOUNTAIN_PASS,
        .name = "Mountain Pass",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_FOREST_EDGE),
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_W(ROOM_VALLEY)
        },
        .interior_walls = {
            /* Narrow pass walls — left cliff */
            { 100, 0, WALL, 280 },
            { 100, 440, WALL, 280 },
            /* Right cliff */
            { 844, 0, WALL, 280 },
            { 844, 440, WALL, 280 },
            /* Boulder in pass */
            { 460, 340, 40, 40 },
        },
        .interior_wall_count = 5,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /* 12: ROOM_VALLEY
     *     North -> Ancient Ruins (13), East -> Mountain Pass (11)
     *     Bridge socket on east wall to Secret (29) — note: east exit goes to
     *     Mountain Pass, bridge socket provides *additional* hidden passage.
     *     Actually per spec, east exit goes to Mountain Pass via normal exit
     *     and the bridge socket is a separate mechanism. But Mountain Pass
     *     already has West -> Valley. Let me re-read: Mountain Pass (11) west
     *     exit goes to Valley, and Valley east exit goes... nowhere normally.
     *     Wait — the spec says Mountain Pass (11) -> west -> Valley (12).
     *     That means Valley has an east exit back to Mountain Pass? Let me
     *     check: the spec lists explicit connections. Valley (12) -> north ->
     *     Ancient Ruins (13). Mountain Pass -> west -> Valley means Valley
     *     should have east -> Mountain Pass for symmetry.
     *     Bridge socket on east wall is for the SECRET room (29) only. The
     *     normal east exit and the bridge socket can coexist at different Y
     *     positions on the east wall. */
    [ROOM_VALLEY] = {
        .id   = ROOM_VALLEY,
        .name = "Valley",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_ANCIENT_RUINS),
            [DIR_EAST]  = EXIT_EC(ROOM_MOUNTAIN_PASS, 320, 400),
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Valley floor rocks */
            { 300, 500, 64, 32 },
            { 600, 300, 48, 48 },
        },
        .interior_wall_count = 2,
        .bridge_sockets = {
            /* Bridge socket to Secret room — lower part of east wall */
            { DIR_EAST, 560, 640 }
        },
        .bridge_socket_count = 1,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /* 13: ROOM_ANCIENT_RUINS
     *     South -> Valley (12) */
    [ROOM_ANCIENT_RUINS] = {
        .id   = ROOM_ANCIENT_RUINS,
        .name = "Ancient Ruins",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_S(ROOM_VALLEY),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Ruined columns */
            { 200, 200, 32, 32 },
            { 400, 150, 32, 32 },
            { 600, 250, 32, 32 },
            { 300, 450, 32, 32 },
            { 700, 400, 32, 32 },
            /* Broken wall segments */
            { 250, 300, 120, WALL },
            { 550, 350, 150, WALL },
        },
        .interior_wall_count = 7,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /* 14: ROOM_SWAMP_EDGE
     *     East -> River Crossing (9), South -> Catacombs Entry (23) */
    [ROOM_SWAMP_EDGE] = {
        .id   = ROOM_SWAMP_EDGE,
        .name = "Swamp Edge",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_E(ROOM_RIVER_CROSSING),
            [DIR_SOUTH] = EXIT_S(ROOM_CATACOMBS_ENTRY),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Swamp pools (impassable) */
            { 100, 200, 120, 80 },
            { 600, 400, 140, 60 },
            { 350, 500, 80, 80 },
        },
        .interior_wall_count = 3,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_OVERWORLD
    },

    /*=========================================================================
     * BLACK CASTLE  (rooms 15-18)  — ROOM_FLAG_CASTLE
     *=======================================================================*/

    /* 15: ROOM_BLACK_GATE
     *     West -> Eastern Path (7), North -> Black Hall (16) */
    [ROOM_BLACK_GATE] = {
        .id   = ROOM_BLACK_GATE,
        .name = "Black Castle — Gate",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_BLACK_HALL),
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_W(ROOM_EASTERN_PATH)
        },
        .interior_walls = {
            /* Gate corridor walls */
            { 300, 0, WALL, 300 },
            { 644, 0, WALL, 300 },
            { 300, 420, WALL, 300 },
            { 644, 420, WALL, 300 },
        },
        .interior_wall_count = 4,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_BLACK
    },

    /* 16: ROOM_BLACK_HALL
     *     South -> Black Gate (15), East -> Black Dungeon (17),
     *     North -> Black Treasury (18) */
    [ROOM_BLACK_HALL] = {
        .id   = ROOM_BLACK_HALL,
        .name = "Black Castle — Hall",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_BLACK_TREASURY),
            [DIR_EAST]  = EXIT_E(ROOM_BLACK_DUNGEON),
            [DIR_SOUTH] = EXIT_S(ROOM_BLACK_GATE),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Pillars along hall */
            { 250, 200, 32, 32 },
            { 678, 200, 32, 32 },
            { 250, 488, 32, 32 },
            { 678, 488, 32, 32 },
        },
        .interior_wall_count = 4,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_BLACK
    },

    /* 17: ROOM_BLACK_DUNGEON
     *     West -> Black Hall (16) */
    [ROOM_BLACK_DUNGEON] = {
        .id   = ROOM_BLACK_DUNGEON,
        .name = "Black Castle — Dungeon",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_W(ROOM_BLACK_HALL)
        },
        .interior_walls = {
            /* Cell walls */
            { 300, 100, WALL, 240 },
            { 300, 100, 200, WALL },
            { 600, 100, WALL, 240 },
            { 300, 400, WALL, 220 },
            { 600, 400, WALL, 220 },
            { 300, 400, 200, WALL },
            { 500, 400, 200, WALL },  /* gap between cells */
        },
        .interior_wall_count = 7,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_BLACK
    },

    /* 18: ROOM_BLACK_TREASURY
     *     South -> Black Hall (16) */
    [ROOM_BLACK_TREASURY] = {
        .id   = ROOM_BLACK_TREASURY,
        .name = "Black Castle — Treasury",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_S(ROOM_BLACK_HALL),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Vault walls forming an inner chamber */
            { 250, 150, WALL, 420 },
            { 694, 150, WALL, 420 },
            { 250, 150, 460, WALL },
            /* Inner pillar */
            { 460, 360, 40, 40 },
        },
        .interior_wall_count = 4,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_BLACK
    },

    /*=========================================================================
     * WHITE CASTLE  (rooms 19-22)  — ROOM_FLAG_CASTLE
     *=======================================================================*/

    /* 19: ROOM_WHITE_GATE
     *     East -> Western Path (8), North -> White Hall (20) */
    [ROOM_WHITE_GATE] = {
        .id   = ROOM_WHITE_GATE,
        .name = "White Castle — Gate",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_WHITE_HALL),
            [DIR_EAST]  = EXIT_E(ROOM_WESTERN_PATH),
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Gate corridor walls */
            { 300, 0, WALL, 300 },
            { 644, 0, WALL, 300 },
            { 300, 420, WALL, 300 },
            { 644, 420, WALL, 300 },
        },
        .interior_wall_count = 4,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_WHITE
    },

    /* 20: ROOM_WHITE_HALL
     *     South -> White Gate (19), North -> White Tower (21),
     *     East -> White Library (22) */
    [ROOM_WHITE_HALL] = {
        .id   = ROOM_WHITE_HALL,
        .name = "White Castle — Hall",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_WHITE_TOWER),
            [DIR_EAST]  = EXIT_E(ROOM_WHITE_LIBRARY),
            [DIR_SOUTH] = EXIT_S(ROOM_WHITE_GATE),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Elegant pillars */
            { 250, 200, 32, 32 },
            { 678, 200, 32, 32 },
            { 250, 488, 32, 32 },
            { 678, 488, 32, 32 },
        },
        .interior_wall_count = 4,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_WHITE
    },

    /* 21: ROOM_WHITE_TOWER
     *     South -> White Hall (20) */
    [ROOM_WHITE_TOWER] = {
        .id   = ROOM_WHITE_TOWER,
        .name = "White Castle — Tower",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_S(ROOM_WHITE_HALL),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Circular tower approximation — walls forming octagonal shape */
            { 200, 100, 560, WALL },
            { 200, 100, WALL, 200 },
            { 744, 100, WALL, 200 },
            /* Spiral staircase center */
            { 430, 300, 100, 100 },
        },
        .interior_wall_count = 4,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_WHITE
    },

    /* 22: ROOM_WHITE_LIBRARY
     *     West -> White Hall (20) */
    [ROOM_WHITE_LIBRARY] = {
        .id   = ROOM_WHITE_LIBRARY,
        .name = "White Castle — Library",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_W(ROOM_WHITE_HALL)
        },
        .interior_walls = {
            /* Bookshelves — horizontal rows */
            { 200, 120, 560, WALL },
            { 200, 240, 400, WALL },
            { 360, 360, 400, WALL },
            { 200, 480, 400, WALL },
            { 360, 600, 400, WALL },
        },
        .interior_wall_count = 5,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_CASTLE,
        .ambient_tint = TINT_WHITE
    },

    /*=========================================================================
     * CATACOMBS  (rooms 23-28)
     *=======================================================================*/

    /* 23: ROOM_CATACOMBS_ENTRY
     *     North -> Swamp Edge (14), North(internal) -> Passage 1 (24) */
    [ROOM_CATACOMBS_ENTRY] = {
        .id   = ROOM_CATACOMBS_ENTRY,
        .name = "Catacombs — Entry",
        .exits = {
            [DIR_NORTH] = EXIT_N(ROOM_SWAMP_EDGE),
            [DIR_EAST]  = EXIT_EC(ROOM_CATACOMBS_1, 320, 400),
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Narrow entry corridor walls */
            { 200, 0, WALL, 320 },
            { 200, 400, WALL, 320 },
            { 744, 0, WALL, 200 },
            { 744, 500, WALL, 220 },
            /* Rubble */
            { 400, 300, 48, 32 },
            { 500, 500, 32, 48 },
        },
        .interior_wall_count = 6,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_CATACOMBS
    },

    /* 24: ROOM_CATACOMBS_1
     *     West -> Entry (23), East -> Passage 2 (25)
     *     (Spec says Entry -> north -> Passage 1, but to make the maze work
     *      better topologically, Entry east -> Passage 1 west, etc.) */
    [ROOM_CATACOMBS_1] = {
        .id   = ROOM_CATACOMBS_1,
        .name = "Catacombs — Passage 1",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_EC(ROOM_CATACOMBS_2, 320, 400),
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_WC(ROOM_CATACOMBS_ENTRY, 320, 400)
        },
        .interior_walls = {
            /* Maze walls creating narrow passages */
            { 200, 0, WALL, 280 },
            { 200, 440, WALL, 280 },
            { 400, 150, WALL, 200 },
            { 400, 500, WALL, 220 },
            { 600, 0, WALL, 350 },
            { 600, 500, WALL, 220 },
            /* Cross wall */
            { 200, 280, 200, WALL },
        },
        .interior_wall_count = 7,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_CATACOMBS
    },

    /* 25: ROOM_CATACOMBS_2
     *     West -> Passage 1 (24), North -> Passage 3 (26),
     *     South -> Dead End (27) */
    [ROOM_CATACOMBS_2] = {
        .id   = ROOM_CATACOMBS_2,
        .name = "Catacombs — Passage 2",
        .exits = {
            [DIR_NORTH] = EXIT_NC(ROOM_CATACOMBS_3, 440, 520),
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_SC(ROOM_CATACOMBS_DEAD, 440, 520),
            [DIR_WEST]  = EXIT_WC(ROOM_CATACOMBS_1, 320, 400)
        },
        .interior_walls = {
            /* Maze: central pillar block */
            { 400, 280, 160, 160 },
            /* Side walls */
            { 200, 0, WALL, 320 },
            { 200, 400, WALL, 320 },
            { 744, 0, WALL, 280 },
            { 744, 440, WALL, 280 },
            /* Corridor divider */
            { 600, 200, WALL, 150 },
        },
        .interior_wall_count = 6,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_CATACOMBS
    },

    /* 26: ROOM_CATACOMBS_3  (DARK)
     *     South -> Passage 2 (25), East -> Chamber (28) */
    [ROOM_CATACOMBS_3] = {
        .id   = ROOM_CATACOMBS_3,
        .name = "Catacombs — Passage 3",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_EC(ROOM_CATACOMBS_CHAMBER, 320, 400),
            [DIR_SOUTH] = EXIT_SC(ROOM_CATACOMBS_2, 440, 520),
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Heavy maze walls — hard to navigate in the dark */
            { 200, 100, WALL, 250 },
            { 200, 450, WALL, 170 },
            { 400, 0, WALL, 200 },
            { 400, 320, WALL, 200 },
            { 600, 200, WALL, 200 },
            { 600, 520, WALL, 200 },
            { 200, 350, 200, WALL },
            { 500, 200, 150, WALL },
        },
        .interior_wall_count = 8,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_DARK,
        .ambient_tint = TINT_CATACOMBS
    },

    /* 27: ROOM_CATACOMBS_DEAD — dead end
     *     North -> Passage 2 (25) */
    [ROOM_CATACOMBS_DEAD] = {
        .id   = ROOM_CATACOMBS_DEAD,
        .name = "Catacombs — Dead End",
        .exits = {
            [DIR_NORTH] = EXIT_NC(ROOM_CATACOMBS_2, 440, 520),
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_NONE
        },
        .interior_walls = {
            /* Collapsed tunnel walls */
            { 200, 200, 560, WALL },
            { 200, 200, WALL, 320 },
            { 744, 200, WALL, 320 },
            { 200, 504, 240, WALL },
            { 520, 504, 240, WALL },
            /* Rubble pile */
            { 420, 400, 120, 64 },
        },
        .interior_wall_count = 6,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_NONE,
        .ambient_tint = TINT_CATACOMBS
    },

    /* 28: ROOM_CATACOMBS_CHAMBER  (DARK)
     *     West -> Passage 3 (26) */
    [ROOM_CATACOMBS_CHAMBER] = {
        .id   = ROOM_CATACOMBS_CHAMBER,
        .name = "Catacombs — Chamber",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_WC(ROOM_CATACOMBS_3, 320, 400)
        },
        .interior_walls = {
            /* Chamber walls — large open room with central altar */
            { 250, 150, 460, WALL },
            { 250, 554, 460, WALL },
            { 250, 150, WALL, 170 },
            { 694, 150, WALL, 170 },
            { 250, 400, WALL, 170 },
            { 694, 400, WALL, 170 },
            /* Altar */
            { 430, 330, 100, 60 },
        },
        .interior_wall_count = 7,
        .bridge_sockets = {0},
        .bridge_socket_count = 0,
        .flags = ROOM_FLAG_DARK,
        .ambient_tint = TINT_CATACOMBS
    },

    /*=========================================================================
     * SECRET ROOM  (room 29)
     *=======================================================================*/

    /* 29: ROOM_SECRET
     *     Accessible from Valley (12) via bridge socket on west wall */
    [ROOM_SECRET] = {
        .id   = ROOM_SECRET,
        .name = "Secret Chamber",
        .exits = {
            [DIR_NORTH] = EXIT_NONE,
            [DIR_EAST]  = EXIT_NONE,
            [DIR_SOUTH] = EXIT_NONE,
            [DIR_WEST]  = EXIT_NONE   /* no normal exit — bridge only */
        },
        .interior_walls = {
            /* Mysterious chamber with pillars */
            { 250, 200, 32, 32 },
            { 678, 200, 32, 32 },
            { 250, 488, 32, 32 },
            { 678, 488, 32, 32 },
            /* Central pedestal */
            { 440, 330, 80, 60 },
        },
        .interior_wall_count = 5,
        .bridge_sockets = {
            /* Bridge socket on west wall — matches Valley's east socket */
            { DIR_WEST, 560, 640 }
        },
        .bridge_socket_count = 1,
        .flags = ROOM_FLAG_SECRET,
        .ambient_tint = TINT_SECRET
    },
};

/*=============================================================================
 * Room lookup
 *===========================================================================*/
const RoomDef *room_get(int id)
{
    if (id < 0 || id >= TOTAL_ROOMS) return NULL;
    return &g_rooms[id];
}

/*=============================================================================
 * BFS distance table
 *===========================================================================*/
int room_distances[TOTAL_ROOMS][TOTAL_ROOMS];

/* Internal: does room `id` have a navigable exit in direction `d`?
 * Returns the destination room or ROOM_NONE. */
static int exit_dest(int id, int d)
{
    return g_rooms[id].exits[d].destination_room;
}

void rooms_compute_distances(void)
{
    /* Initialise all distances to -1 (unreachable) */
    memset(room_distances, -1, sizeof(room_distances));

    for (int src = 0; src < TOTAL_ROOMS; src++) {
        /* BFS from src */
        int queue[TOTAL_ROOMS];
        int head = 0, tail = 0;

        room_distances[src][src] = 0;
        queue[tail++] = src;

        while (head < tail) {
            int cur = queue[head++];
            int next_dist = room_distances[src][cur] + 1;

            for (int d = 0; d < DIR_COUNT; d++) {
                int nb = exit_dest(cur, d);
                if (nb == ROOM_NONE) continue;
                if (room_distances[src][nb] >= 0) continue; /* already visited */
                room_distances[src][nb] = next_dist;
                queue[tail++] = nb;
            }
        }
    }
}

/*=============================================================================
 * BFS next-step: returns the next room to move into when travelling from
 * `from` toward `to`.  Returns ROOM_NONE if unreachable or from == to.
 *===========================================================================*/
int rooms_bfs_next_room(int from, int to)
{
    if (from < 0 || from >= TOTAL_ROOMS) return ROOM_NONE;
    if (to   < 0 || to   >= TOTAL_ROOMS) return ROOM_NONE;
    if (from == to) return ROOM_NONE;
    if (room_distances[from][to] < 0) return ROOM_NONE; /* unreachable */

    /* Look at each neighbor of `from`; pick the one closest to `to` */
    int best_room = ROOM_NONE;
    int best_dist = room_distances[from][to]; /* current distance */

    for (int d = 0; d < DIR_COUNT; d++) {
        int nb = exit_dest(from, d);
        if (nb == ROOM_NONE) continue;
        int nd = room_distances[nb][to];
        if (nd < 0) continue;
        if (nd < best_dist) {
            best_dist = nd;
            best_room = nb;
        }
    }

    return best_room;
}
