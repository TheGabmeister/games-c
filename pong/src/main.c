#include "game.h"

int main(void)
{
    if (!game_init())
        return 1;

    game_loop();
    game_fini();
    return 0;
}
