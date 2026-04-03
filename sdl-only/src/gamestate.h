#ifndef GAMESTATE_H
#define GAMESTATE_H

typedef struct {
    int score;
    int health;
} GameState;

GameState *gamestate_get(void);
void       gamestate_reset(void);

#endif // GAMESTATE_H
