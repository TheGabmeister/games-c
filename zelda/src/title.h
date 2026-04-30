#ifndef TITLE_H
#define TITLE_H

#include "save.h"

typedef struct TitleState {
    int cursor;
    SaveSlotSummary slots[SAVE_SLOT_COUNT];
} TitleState;

struct Game;

void title_refresh(TitleState *state);
void title_update(struct Game *game);
void title_draw(const TitleState *state);

#endif
