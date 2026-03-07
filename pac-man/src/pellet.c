#include "pellet.h"
#include "score.h"

void pellet_init(GameState *gs) {
    gs->pellets.power_active = false;
    gs->fruit.active         = false;
    gs->fruit.trigger1_done  = false;
    gs->fruit.trigger2_done  = false;
    gs->fruit.type           = pellet_fruit_for_level(gs->score_state.level);
    gs->fruit.score_value    = pellet_fruit_score(gs->fruit.type);
}

static void start_frightened(GameState *gs) {
    gs->pellets.power_active = true;
    float dur = gs->pellets.frightened_duration;
    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &gs->ghosts[i];
        if (g->mode == GSTATE_EATEN) continue;
        if (g->mode != GSTATE_FRIGHTENED) {
            g->prev_mode = g->mode;
            /* Reverse direction on mode entry */
            switch (g->dir) {
                case DIR_LEFT:  g->dir = DIR_RIGHT; break;
                case DIR_RIGHT: g->dir = DIR_LEFT;  break;
                case DIR_UP:    g->dir = DIR_DOWN;  break;
                case DIR_DOWN:  g->dir = DIR_UP;    break;
                default: break;
            }
        }
        g->mode              = GSTATE_FRIGHTENED;
        g->frightened_timer  = dur;
        g->flash_timer       = 0.0f;
        g->flash_visible     = true;
    }
    score_reset_combo(gs);
}

void pellet_on_eat(GameState *gs, TileType eaten_type) {
    if (eaten_type == TILE_POWER_PELLET) {
        start_frightened(gs);
    }
    /* Check fruit spawn triggers */
    int eaten = gs->pellets.eaten;
    if (!gs->fruit.trigger1_done && eaten >= FRUIT_DOT_TRIGGER1) {
        gs->fruit.active        = true;
        gs->fruit.timer         = FRUIT_TIMER;
        gs->fruit.trigger1_done = true;
    } else if (!gs->fruit.trigger2_done && eaten >= FRUIT_DOT_TRIGGER2) {
        gs->fruit.active        = true;
        gs->fruit.timer         = FRUIT_TIMER;
        gs->fruit.trigger2_done = true;
    }
}

void pellet_update_frightened(GameState *gs, float dt) {
    bool any_frightened = false;
    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &gs->ghosts[i];
        if (g->mode != GSTATE_FRIGHTENED) continue;
        any_frightened = true;
        g->frightened_timer -= dt;
        if (g->frightened_timer <= 0.0f) {
            g->mode        = g->prev_mode;
            g->flash_visible = true;
        } else if (g->frightened_timer <= FRIGHTENED_FLASH_AT) {
            /* Flash */
            g->flash_timer += dt;
            if (g->flash_timer >= FLASH_INTERVAL) {
                g->flash_timer = 0.0f;
                g->flash_visible = !g->flash_visible;
            }
        } else {
            g->flash_visible = true;
        }
    }
    if (!any_frightened)
        gs->pellets.power_active = false;
}

void pellet_update_fruit(GameState *gs, float dt) {
    if (!gs->fruit.active) return;
    gs->fruit.timer -= dt;
    if (gs->fruit.timer <= 0.0f)
        gs->fruit.active = false;
}

FruitType pellet_fruit_for_level(int level) {
    switch (level) {
        case 1:            return FRUIT_CHERRY;
        case 2:            return FRUIT_STRAWBERRY;
        case 3: case 4:    return FRUIT_ORANGE;
        case 5: case 6:    return FRUIT_APPLE;
        case 7: case 8:    return FRUIT_MELON;
        case 9: case 10:   return FRUIT_GALAXIAN;
        case 11: case 12:  return FRUIT_BELL;
        default:           return FRUIT_KEY;
    }
}

int pellet_fruit_score(FruitType type) {
    switch (type) {
        case FRUIT_CHERRY:     return 100;
        case FRUIT_STRAWBERRY: return 300;
        case FRUIT_ORANGE:     return 500;
        case FRUIT_APPLE:      return 700;
        case FRUIT_MELON:      return 1000;
        case FRUIT_GALAXIAN:   return 2000;
        case FRUIT_BELL:       return 3000;
        case FRUIT_KEY:        return 5000;
        default:               return 0;
    }
}
