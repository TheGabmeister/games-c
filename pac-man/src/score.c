#include "score.h"

/* Ghost eating scores: index by ghost_combo (0-3) */
static const int GHOST_SCORES[4] = { 200, 400, 800, 1600 };

/* Level speed / duration table */
typedef struct {
    float pacman_speed;
    float ghost_speed;
    float frightened_duration;
} LevelParams;

static const LevelParams LEVEL_PARAMS[] = {
    /* level 1  */ { 0.80f, 0.75f, 6.0f },
    /* level 2  */ { 0.90f, 0.85f, 5.0f },
    /* level 3  */ { 0.90f, 0.85f, 4.0f },
    /* level 4  */ { 0.90f, 0.85f, 3.0f },
    /* level 5  */ { 1.00f, 0.95f, 2.0f },
    /* level 6  */ { 1.00f, 0.95f, 2.0f },
    /* level 7  */ { 1.00f, 0.95f, 2.0f },
    /* level 8  */ { 1.00f, 0.95f, 2.0f },
    /* level 9  */ { 1.00f, 0.95f, 1.0f },
    /* level 10 */ { 1.00f, 0.95f, 1.0f },
    /* level 11 */ { 1.00f, 0.95f, 1.0f },
    /* level 12 */ { 1.00f, 0.95f, 1.0f },
    /* level 13 */ { 1.00f, 0.95f, 1.0f },
    /* level 14 */ { 1.00f, 0.95f, 1.0f },
    /* level 15 */ { 1.00f, 0.95f, 0.0f },
    /* level 16 */ { 1.00f, 0.95f, 0.0f },
    /* level 17 */ { 1.00f, 0.95f, 0.0f },
    /* level 18 */ { 1.00f, 0.95f, 0.0f },
    /* level 19 */ { 1.00f, 0.95f, 0.0f },
    /* level 20 */ { 1.00f, 0.95f, 0.0f },
};
#define LEVEL_PARAMS_COUNT (int)(sizeof(LEVEL_PARAMS)/sizeof(LEVEL_PARAMS[0]))

#define BASE_TILE_SPEED  9.5f   /* tiles/s at 100% — speed fractions scale from this */

static void apply_level_params(GameState *gs) {
    int idx = gs->score_state.level - 1;
    if (idx >= LEVEL_PARAMS_COUNT) idx = LEVEL_PARAMS_COUNT - 1;
    const LevelParams *p = &LEVEL_PARAMS[idx];
    gs->pacman_speed           = p->pacman_speed * BASE_TILE_SPEED * TILE_SIZE;
    gs->ghost_speed            = p->ghost_speed  * BASE_TILE_SPEED * TILE_SIZE;
    gs->frightened_duration    = p->frightened_duration;
    gs->pellets.frightened_duration = p->frightened_duration;
}

void score_init(GameState *gs, int starting_lives) {
    gs->score_state.score              = 0;
    gs->score_state.lives              = starting_lives;
    gs->score_state.level              = 1;
    gs->score_state.ghost_combo        = 0;
    gs->score_state.extra_life_granted = false;
    /* high_score persists; don't reset it here */
    apply_level_params(gs);
}

void score_add(GameState *gs, int points) {
    int prev = gs->score_state.score;
    gs->score_state.score += points;
    if (gs->score_state.score > gs->score_state.high_score)
        gs->score_state.high_score = gs->score_state.score;
    /* Extra life at 10000 */
    if (!gs->score_state.extra_life_granted &&
        prev < EXTRA_LIFE_SCORE &&
        gs->score_state.score >= EXTRA_LIFE_SCORE) {
        gs->score_state.lives++;
        gs->score_state.extra_life_granted = true;
    }
}

void score_ghost_eaten(GameState *gs) {
    int idx = gs->score_state.ghost_combo;
    if (idx > 3) idx = 3;
    score_add(gs, GHOST_SCORES[idx]);
    gs->score_state.ghost_combo++;
}

void score_reset_combo(GameState *gs) {
    gs->score_state.ghost_combo = 0;
}

void score_lose_life(GameState *gs) {
    gs->score_state.lives--;
}

void score_next_level(GameState *gs) {
    gs->score_state.level++;
    gs->score_state.ghost_combo = 0;
    apply_level_params(gs);
}
