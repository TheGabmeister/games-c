#ifndef WORLD_INTERNAL_H
#define WORLD_INTERNAL_H

#include "world.h"

bool world_circles_overlap(Vector2 a, float ar, Vector2 b, float br);

void world_add_float_text(Game *game, Vector2 position, int value, Color color);
void world_add_particles(Game *game, Vector2 position, Color color, int count, float speed, float radius);
void world_add_score(Game *game, int value);
void world_update_effects(Game *game, float dt);
void world_draw_effects(Game *game);

void world_spawn_grunt(Game *game, Vector2 position);
void world_spawn_hulk(Game *game, Vector2 position);
void world_spawn_spheroid(Game *game, Vector2 position);
void world_spawn_quark(Game *game, Vector2 position);
void world_spawn_tank(Game *game, Vector2 position);
void world_spawn_brain(Game *game, Vector2 position);
void world_spawn_prog(Game *game, Vector2 position);
void world_spawn_human(Game *game, Vector2 position, int type);
void world_spawn_electrode(Game *game, Vector2 position);
void world_update_humans(Game *game, float dt);
void world_update_grunts_and_hulks(Game *game, float dt);
void world_update_spawners_and_shooters(Game *game, float dt);
void world_update_brains_and_progs(Game *game, float dt);

void world_update_player_and_bullets(Game *game, float dt);
void world_update_projectiles(Game *game, float dt);
void world_spawn_projectile(Game *game, ProjectileType type, Vector2 position, Vector2 velocity);
void world_draw_projectiles(Game *game);
void world_draw_bullets(Game *game);

void world_resolve_bullet_collisions(Game *game);
void world_resolve_human_collisions(Game *game);
void world_resolve_grunt_electrode_collisions(Game *game);
void world_resolve_player_death_collisions(Game *game);

#endif
