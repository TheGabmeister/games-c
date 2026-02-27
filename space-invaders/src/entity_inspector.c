#include <stdio.h>
#include "components/transform_comp.h"

#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#include "nuklear.h"
#include "entity_inspector.h"

void draw_entity_inspector(struct nk_context *ctx, ecs_world_t *world, ecs_query_t *debug_q) {
    int entity_count = 0;
    ecs_iter_t dit = ecs_query_iter(world, debug_q);
    while (ecs_query_next(&dit)) entity_count += dit.count;

    if (nk_begin(ctx, "Entity Inspector",
                 nk_rect(10, 10, 220, 200),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                 NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
    {
        char count_buf[32];
        snprintf(count_buf, sizeof(count_buf), "Entities: %d", entity_count);
        nk_layout_row_dynamic(ctx, 18, 1);
        nk_label(ctx, count_buf, NK_TEXT_LEFT);

        nk_layout_row_template_begin(ctx, 18);
        nk_layout_row_template_push_static(ctx, 70);
        nk_layout_row_template_push_static(ctx, 65);
        nk_layout_row_template_push_static(ctx, 65);
        nk_layout_row_template_end(ctx);

        nk_label(ctx, "Name", NK_TEXT_LEFT);
        nk_label(ctx, "X",    NK_TEXT_LEFT);
        nk_label(ctx, "Y",    NK_TEXT_LEFT);

        dit = ecs_query_iter(world, debug_q);
        while (ecs_query_next(&dit)) {
            Position *p = ecs_field(&dit, Position, 0);
            for (int i = 0; i < dit.count; i++) {
                const char *name = ecs_get_name(world, dit.entities[i]);
                char xbuf[24], ybuf[24];
                snprintf(xbuf, sizeof(xbuf), "%.1f", p[i].x);
                snprintf(ybuf, sizeof(ybuf), "%.1f", p[i].y);
                nk_label(ctx, name ? name : "unnamed", NK_TEXT_LEFT);
                nk_label(ctx, xbuf, NK_TEXT_LEFT);
                nk_label(ctx, ybuf, NK_TEXT_LEFT);
            }
        }
    }
    nk_end(ctx);
}
