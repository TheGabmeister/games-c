#include "render_fx.h"

void DrawNeonLine(Vector2 start, Vector2 end, Color color)
{
    DrawLineEx(start, end, 3.0f, ColorAlpha(color, 0.3f));
    DrawLineEx(start, end, 1.0f, color);
}

void DrawNeonCircle(Vector2 center, float radius, Color color)
{
    DrawCircleLinesV(center, radius + 1.0f, ColorAlpha(color, 0.3f));
    DrawCircleLinesV(center, radius, color);
}

void DrawNeonTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    DrawNeonLine(v1, v2, color);
    DrawNeonLine(v2, v3, color);
    DrawNeonLine(v3, v1, color);
}

void DrawNeonLineStrip(const Vector2 *points, int count, Color color)
{
    for (int i = 0; i < count - 1; i++) {
        DrawNeonLine(points[i], points[i + 1], color);
    }
}
