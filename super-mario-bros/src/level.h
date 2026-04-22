#ifndef LEVEL_H
#define LEVEL_H

#include "common.h"
#include "entity.h"

typedef struct {
    int type;
    int tile_x;
    int tile_y;
} EntitySpawn;

typedef struct {
    int *tiles;
    int width;
    int height;
    EntitySpawn *spawns;
    int spawn_count;
    int next_spawn;
    Color bg_color;
} Level;

void level_load_test(Level *level);
void level_free(Level *level);
void level_draw(Level *level, float camera_x);
bool tile_is_solid(int tile_type);
int level_get_tile(Level *level, int tx, int ty);
void level_set_tile(Level *level, int tx, int ty, int tile_type);

void level_collide_x(Level *level, Entity *e);
void level_collide_y(Level *level, Entity *e, Game *game);
void level_collide_entity(Level *level, Entity *e, Game *game);

#endif
