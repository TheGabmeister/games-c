#ifndef COMBAT_H
#define COMBAT_H

struct Game;

void combat_check(struct Game *game);
void combat_check_bombs(struct Game *game);
void combat_check_pickups(struct Game *game);

#endif
