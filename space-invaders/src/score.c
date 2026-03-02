#include "score.h"
#include <stdio.h>

#define HIGH_SCORE_FILE "highscore.txt"

static int current_score = 0;
static int high_score    = 0;

void score_reset(void)        { current_score = 0; }
void score_add(int points)    { current_score += points; }
int  score_get(void)          { return current_score; }
int  score_get_high(void)     { return high_score; }

void score_update_high(void)
{
    if (current_score > high_score)
        high_score = current_score;
}

void score_load_high(void)
{
    FILE *f = fopen(HIGH_SCORE_FILE, "r");
    if (f) {
        fscanf(f, "%d", &high_score);
        fclose(f);
    }
}

void score_save_high(void)
{
    FILE *f = fopen(HIGH_SCORE_FILE, "w");
    if (f) {
        fprintf(f, "%d\n", high_score);
        fclose(f);
    }
}
