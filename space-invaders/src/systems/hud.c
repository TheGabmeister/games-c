#include "hud.h"
#include "../tags.h"
#include "../settings.h"
#include "../score.h"
#include "../components/health.h"
#include <string.h>

/* Terms: [Player(tag), Health] — indices 0 (tag), 1 */
static ecs_query_t  *player_health_query = NULL;
static SDL_Renderer *hud_renderer        = NULL;

void hud_system_init(ecs_world_t *world, SDL_Renderer *renderer)
{
    hud_renderer = renderer;
    player_health_query = ecs_query(world, {
        .terms = { { .id = Player }, { .id = ecs_id(Health) } }
    });
}

int hud_system_get_player_lives(void)
{
    int lives = 0;
    ecs_iter_t it = ecs_query_iter(
        ecs_get_world(player_health_query), player_health_query);
    while (ecs_query_next(&it)) {
        Health *h = ecs_field(&it, Health, 1);
        if (it.count > 0) lives = h[0].current;
    }
    return lives;
}

void hud_system_run(void)
{
    char buf[32];
    const float scale  = 1.5f;
    const float char_w = 8.0f * scale;
    const float y_screen = 8.0f;

    SDL_SetRenderScale(hud_renderer, scale, scale);
    SDL_SetRenderDrawColor(hud_renderer, 255, 255, 255, 255);

    /* Left: score */
    SDL_snprintf(buf, sizeof(buf), "SCORE: %05d", score_get());
    SDL_RenderDebugText(hud_renderer, 8.0f / scale, y_screen / scale, buf);

    /* Center: hi-score */
    SDL_snprintf(buf, sizeof(buf), "HI: %05d", score_get_high());
    float hi_w = (float)strlen(buf) * char_w;
    SDL_RenderDebugText(hud_renderer,
        ((float)WINDOW_WIDTH - hi_w) * 0.5f / scale,
        y_screen / scale, buf);

    /* Right: lives */
    SDL_snprintf(buf, sizeof(buf), "LIVES: %d", hud_system_get_player_lives());
    float lives_w = (float)strlen(buf) * char_w;
    SDL_RenderDebugText(hud_renderer,
        ((float)WINDOW_WIDTH - lives_w - 8.0f) / scale,
        y_screen / scale, buf);

    SDL_SetRenderScale(hud_renderer, 1.0f, 1.0f);
}

void hud_system_destroy(void)
{
    ecs_query_fini(player_health_query);
    player_health_query = NULL;
}
