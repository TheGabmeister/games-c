#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "game_config.h"

typedef struct DialogueState {
    char text[CAVE_TEXT_MAX];
} DialogueState;

struct Game;

void dialogue_start(struct Game *game, const char *text);
void dialogue_update(struct Game *game);
void dialogue_draw(const DialogueState *state);

#endif
