#ifndef SCORE_H
#define SCORE_H

void score_reset(void);
void score_add(int points);
int  score_get(void);
int  score_get_high(void);
void score_update_high(void);
void score_load_high(void);
void score_save_high(void);

#endif
