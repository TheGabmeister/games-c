#ifndef LEVEL_H
#define LEVEL_H

#include "common.h"
#include "entity.h"

#define MAX_BLOCK_CONTENTS 128
#define MAX_SPAWNS 64
#define MAX_PIPE_WARPS 8

typedef enum {
    LEVEL_OVERWORLD,
    LEVEL_UNDERGROUND,
    LEVEL_CASTLE,
    LEVEL_ATHLETIC,
    LEVEL_UNDERWATER,
} LevelType;

typedef struct {
    int tile_x, tile_y;
    int content;
} BlockContent;

typedef struct {
    int type;
    int tile_x;
    int tile_y;
    int extra;
    bool activated;
} EntitySpawn;

typedef struct {
    int pipe_tx, pipe_ty;
    int dest_world, dest_sublevel;
    int dest_tx, dest_ty;
} PipeWarp;

typedef struct {
    int *tiles;
    int width;
    int height;
    LevelType type;

    EntitySpawn spawns[MAX_SPAWNS];
    int spawn_count;

    BlockContent blocks[MAX_BLOCK_CONTENTS];
    int block_count;

    PipeWarp warps[MAX_PIPE_WARPS];
    int warp_count;

    Color bg_color;

    int bridge_start_tx;
    int bridge_end_tx;
    int bridge_ty;
} Level;

void level_load(Level *level, int world, int sublevel);
void level_free(Level *level);
void level_draw(Level *level, float camera_x);
bool tile_is_solid(int tile_type);
int level_get_tile(Level *level, int tx, int ty);
void level_set_tile(Level *level, int tx, int ty, int tile_type);

int level_get_block_content(Level *level, int tx, int ty);
void level_activate_spawns(Level *level, Entity entities[MAX_ENTITIES], float camera_x);
void level_handle_head_bump(Level *level, Entity *e, int tx, int ty, Game *game);

PipeWarp *level_get_warp(Level *level, int tx, int ty);
void level_update_blasters(Level *level, Entity entities[MAX_ENTITIES], int mario_idx, float camera_x, float *timer);

void level_collide_x(Level *level, Entity *e);
void level_collide_y(Level *level, Entity *e, Game *game);
void level_collide_entity(Level *level, Entity *e, Game *game);

#endif
