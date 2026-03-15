#ifndef SCORE_H
#define SCORE_H

typedef struct {
    int p1;
    int p2;
} Score;

void         score_init(void);
const Score *score_get(void);

#endif
