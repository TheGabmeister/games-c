#ifndef RENDER_FX_H
#define RENDER_FX_H

#include "raylib.h"

void DrawNeonLine(Vector2 start, Vector2 end, Color color);
void DrawNeonCircle(Vector2 center, float radius, Color color);
void DrawNeonTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawNeonLineStrip(const Vector2 *points, int count, Color color);

#endif
