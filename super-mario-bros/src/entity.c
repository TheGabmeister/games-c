#include "entity.h"

Entity *entity_alloc(Entity entities[MAX_ENTITIES]) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].type == ENT_NONE) {
            memset(&entities[i], 0, sizeof(Entity));
            return &entities[i];
        }
    }
    return NULL;
}

void entity_deactivate(Entity *e) {
    e->type = ENT_NONE;
    e->active = false;
}

bool entity_overlap(Entity *a, Entity *b) {
    return a->x < b->x + b->w &&
           a->x + a->w > b->x &&
           a->y < b->y + b->h &&
           a->y + a->h > b->y;
}
