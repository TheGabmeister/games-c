#pragma once
#include "game.h"

/* Master draw call — call once per frame inside BeginDrawing/EndDrawing */
void renderer_draw(const GameState *gs);
