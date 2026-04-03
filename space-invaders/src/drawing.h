#ifndef DRAWING_H
#define DRAWING_H

#include "game_types.h"

void DrawPlayerShip(float x, float y, bool blink);
void DrawAlienShape(AlienType type, float x, float y, int frame);
void DrawShieldBlock(Shield *s, Vector2 offset);
void DrawBulletObj(Bullet *b, bool isPlayer);
void DrawUFOShape(UFOShip *u);
void DrawHUD(int score, int wave, int lives);
void DrawGroundLine(Vector2 offset);

void InitStars(Star *stars);
void UpdateStars(Star *stars, float dt);
void DrawStars(Star *stars);

void UpdateFloatTexts(FloatText *texts, float dt);
void DrawFloatTexts(FloatText *texts);

#endif // DRAWING_H
