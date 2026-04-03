#ifndef DRAWING_H
#define DRAWING_H

#include "game_types.h"

void DrawPlayerShip(float x, float y, bool blink);
void DrawAlienShape(AlienType type, float x, float y, int frame);
void DrawShieldBlock(const Shield *s, Vector2 offset);
void DrawBulletObj(const Bullet *b, bool isPlayer);
void DrawUFOShape(const UFOShip *u);
void DrawHUD(int score, int wave, int lives);
void DrawGroundLine(Vector2 offset);

#endif // DRAWING_H
