#ifndef LEVEL_H
#define LEVEL_H

#include "common.h"
#include "entity.h"

#define MAX_BLOCK_CONTENTS 128
#define MAX_SPAWNS 64

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
    int *tiles;
    int width;
    int height;

    EntitySpawn spawns[MAX_SPAWNS];
    int spawn_count;

    BlockContent blocks[MAX_BLOCK_CONTENTS];
    int block_count;

    Color bg_color;
} Level;

void level_load_1_1(Level *level);
void level_load_test(Level *level);
void level_free(Level *level);
void level_draw(Level *level, float camera_x);
bool tile_is_solid(int tile_type);
int level_get_tile(Level *level, int tx, int ty);
void level_set_tile(Level *level, int tx, int ty, int tile_type);

int level_get_block_content(Level *level, int tx, int ty);
void level_activate_spawns(Level *level, Entity entities[MAX_ENTITIES], float camera_x);
void level_handle_head_bump(Level *level, Entity *e, int tx, int ty, Game *game);

void level_collide_x(Level *level, Entity *e);
void level_collide_y(Level *level, Entity *e, Game *game);
void level_collide_entity(Level *level, Entity *e, Game *game);

#endif
