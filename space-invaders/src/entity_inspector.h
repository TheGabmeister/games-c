#pragma once

#include <flecs.h>

struct nk_context;

void draw_entity_inspector(struct nk_context *ctx, ecs_world_t *world, ecs_query_t *debug_q);
