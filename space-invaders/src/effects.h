#ifndef EFFECTS_H
#define EFFECTS_H

#include "game_types.h"

void InitStars(Star *stars);
void UpdateStars(Star *stars, float dt);
void DrawStars(const Star *stars);

void UpdateFloatTexts(FloatText *texts, float dt);
void DrawFloatTexts(const FloatText *texts);

#endif // EFFECTS_H
