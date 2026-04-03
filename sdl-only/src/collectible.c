#include "collectible.h"
#include "draw.h"

static Collectible collectibles[MAX_COLLECTIBLES];

static bool rects_overlap(rectangle a, rectangle b)
{
    return a.x < b.x + b.w && a.x + a.w > b.x
        && a.y < b.y + b.h && a.y + a.h > b.y;
}

void collectibles_init(void)
{
    for (int i = 0; i < MAX_COLLECTIBLES; i++)
        collectibles[i].active = false;
}

void collectible_spawn(CollectibleType type, float x, float y)
{
    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        if (!collectibles[i].active) {
            collectibles[i] = (Collectible){
                .rect   = { x, y, 20, 20 },
                .type   = type,
                .active = true
            };
            return;
        }
    }
}

void collectibles_update(rectangle player_rect, GameState *state)
{
    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        if (!collectibles[i].active) continue;
        if (!rects_overlap(player_rect, collectibles[i].rect)) continue;

        collectibles[i].active = false;

        switch (collectibles[i].type) {
            case COLLECTIBLE_COIN:   state->score  += 10; break;
            case COLLECTIBLE_HEALTH: state->health += 1;  break;
            case COLLECTIBLE_STAR:   state->score  += 50; break;
            default: break;
        }
    }
}

void collectibles_draw(void)
{
    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        if (!collectibles[i].active) continue;

        Collectible *c = &collectibles[i];
        float cx = c->rect.x + c->rect.w / 2.0f;
        float cy = c->rect.y + c->rect.h / 2.0f;
        float r  = c->rect.w / 2.0f;

        switch (c->type) {
            case COLLECTIBLE_COIN:
                draw_circle_filled((vector2){ cx, cy }, r, COLOR_YELLOW);
                break;
            case COLLECTIBLE_HEALTH:
                draw_rect_filled((rectangle){ cx - 3, c->rect.y + 2, 6, 16 }, COLOR_RED);
                draw_rect_filled((rectangle){ c->rect.x + 2, cy - 3, 16, 6 }, COLOR_RED);
                break;
            case COLLECTIBLE_STAR:
                draw_circle((vector2){ cx, cy }, r, COLOR_WHITE);
                draw_circle((vector2){ cx, cy }, r * 0.5f, COLOR_WHITE);
                break;
            default: break;
        }
    }
}

int collectibles_remaining(void)
{
    int count = 0;
    for (int i = 0; i < MAX_COLLECTIBLES; i++)
        if (collectibles[i].active) count++;
    return count;
}
