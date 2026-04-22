#include "level.h"
#include "game.h"
#include "sprites.h"
#include "sounds.h"
#include <stdlib.h>

bool tile_is_solid(int tile_type) {
    switch (tile_type) {
        case TILE_GROUND:
        case TILE_BRICK:
        case TILE_QUESTION:
        case TILE_USED:
        case TILE_HARD:
        case TILE_PIPE_TL:
        case TILE_PIPE_TR:
        case TILE_PIPE_BL:
        case TILE_PIPE_BR:
        case TILE_FLAGPOLE_BASE:
        case TILE_CORAL:
        case TILE_BRIDGE:
        case TILE_BILL_BLASTER:
            return true;
        default:
            return false;
    }
}

int level_get_tile(Level *level, int tx, int ty) {
    if (tx < 0 || tx >= level->width || ty < 0 || ty >= level->height)
        return TILE_EMPTY;
    return level->tiles[ty * level->width + tx];
}

void level_set_tile(Level *level, int tx, int ty, int tile_type) {
    if (tx < 0 || tx >= level->width || ty < 0 || ty >= level->height)
        return;
    level->tiles[ty * level->width + tx] = tile_type;
}

static Color tile_color(int tile_type) {
    switch (tile_type) {
        case TILE_GROUND:       return COLOR_GROUND;
        case TILE_BRICK:        return COLOR_BRICK;
        case TILE_QUESTION:     return COLOR_QBLOCK;
        case TILE_USED:         return COLOR_USED;
        case TILE_HARD:         return COLOR_HARD;
        case TILE_PIPE_TL:
        case TILE_PIPE_TR:
        case TILE_PIPE_BL:
        case TILE_PIPE_BR:      return COLOR_PIPE;
        case TILE_FLAGPOLE:     return GRAY;
        case TILE_FLAGPOLE_BASE: return COLOR_GROUND;
        default:                return BLANK;
    }
}

static SpriteID tile_sprite(int tile_type) {
    switch (tile_type) {
        case TILE_GROUND:       return SPR_TILE_GROUND;
        case TILE_BRICK:        return SPR_TILE_BRICK;
        case TILE_QUESTION:     return SPR_TILE_QUESTION;
        case TILE_USED:         return SPR_TILE_USED;
        case TILE_HARD:         return SPR_TILE_HARD;
        case TILE_PIPE_TL:      return SPR_TILE_PIPE_TL;
        case TILE_PIPE_TR:      return SPR_TILE_PIPE_TR;
        case TILE_PIPE_BL:      return SPR_TILE_PIPE_BL;
        case TILE_PIPE_BR:      return SPR_TILE_PIPE_BR;
        case TILE_FLAGPOLE:     return SPR_TILE_FLAGPOLE;
        case TILE_FLAGPOLE_BASE: return SPR_TILE_GROUND;
        default:                return -1;
    }
}

void level_draw(Level *level, float camera_x) {
    int start_tx = (int)(camera_x / TILE_SIZE);
    int end_tx = start_tx + TILES_X + 1;
    if (end_tx > level->width) end_tx = level->width;

    for (int ty = 0; ty < level->height; ty++) {
        for (int tx = start_tx; tx < end_tx; tx++) {
            int tile = level_get_tile(level, tx, ty);
            if (tile == TILE_EMPTY) continue;

            float draw_x = tx * TILE_SIZE - camera_x;
            float draw_y = ty * TILE_SIZE;

            SpriteID sid = tile_sprite(tile);
            if (sid >= 0 && sid < SPR_COUNT) {
                Texture2D tex = sprites_get(sid);
                if (tex.id > 0) {
                    DrawTexturePro(tex,
                        (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                        (Rectangle){draw_x, draw_y, TILE_SIZE, TILE_SIZE},
                        (Vector2){0, 0}, 0, WHITE);
                    continue;
                }
            }

            // Fallback: colored rectangle
            Color c = tile_color(tile);
            if (c.a == 0) continue;
            DrawRectangle((int)draw_x, (int)draw_y, TILE_SIZE, TILE_SIZE, c);
            Color border = {0, 0, 0, 60};
            DrawRectangleLines((int)draw_x, (int)draw_y, TILE_SIZE, TILE_SIZE, border);
        }
    }
}

// Resolve horizontal tile overlaps after X movement
void level_collide_x(Level *level, Entity *e) {
    int top    = (int)(e->y / TILE_SIZE);
    int bottom = (int)((e->y + e->h - 1) / TILE_SIZE);
    int left   = (int)(e->x / TILE_SIZE);
    int right  = (int)((e->x + e->w - 1) / TILE_SIZE);

    for (int ty = top; ty <= bottom; ty++) {
        for (int tx = left; tx <= right; tx++) {
            if (!tile_is_solid(level_get_tile(level, tx, ty))) continue;

            float tile_l = (float)(tx * TILE_SIZE);
            float tile_r = tile_l + TILE_SIZE;

            // Only resolve if actually overlapping
            float overlap_l = e->x + e->w - tile_l;
            float overlap_r = tile_r - e->x;

            if (overlap_l > 0 && overlap_r > 0) {
                if (e->vx > 0) {
                    e->x = tile_l - e->w;
                    e->vx = 0;
                } else if (e->vx < 0) {
                    e->x = tile_r;
                    e->vx = 0;
                } else {
                    // Nudge out by smallest overlap
                    if (overlap_l < overlap_r)
                        e->x = tile_l - e->w;
                    else
                        e->x = tile_r;
                }
            }
        }
    }
}

// Resolve vertical tile overlaps after Y movement
void level_collide_y(Level *level, Entity *e, Game *game) {
    e->on_ground = false;

    int left   = (int)(e->x / TILE_SIZE);
    int right  = (int)((e->x + e->w - 1) / TILE_SIZE);
    int top    = (int)(e->y / TILE_SIZE);
    int bottom = (int)((e->y + e->h - 1) / TILE_SIZE);

    for (int ty = top; ty <= bottom; ty++) {
        for (int tx = left; tx <= right; tx++) {
            if (!tile_is_solid(level_get_tile(level, tx, ty))) continue;

            float tile_t = (float)(ty * TILE_SIZE);
            float tile_b = tile_t + TILE_SIZE;

            float overlap_t = e->y + e->h - tile_t;
            float overlap_b = tile_b - e->y;

            if (overlap_t > 0 && overlap_b > 0) {
                if (e->vy >= 0 && overlap_t <= overlap_b) {
                    e->y = tile_t - e->h;
                    e->vy = 0;
                    e->on_ground = true;
                } else if (e->vy < 0 && overlap_b <= overlap_t) {
                    e->y = tile_b;
                    e->vy = 0;

                    if (e->type == ENT_MARIO) {
                        int tile = level_get_tile(level, tx, ty);
                        if (tile == TILE_QUESTION) {
                            level_set_tile(level, tx, ty, TILE_USED);
                            sound_play(SND_BUMP);
                        } else if (tile == TILE_BRICK) {
                            if (e->power >= MARIO_BIG) {
                                level_set_tile(level, tx, ty, TILE_EMPTY);
                                sound_play(SND_BRICK_BREAK);
                            } else {
                                sound_play(SND_BUMP);
                            }
                        }
                    }
                }
            }
        }
    }

    // Pit death: fell below level
    if (e->y > level->height * TILE_SIZE) {
        if (e->type == ENT_MARIO) {
            if (game) {
                game->state = STATE_DYING;
                game->state_timer = 0;
                sound_play(SND_DEATH);
            }
        } else {
            entity_deactivate(e);
        }
    }
}

void level_collide_entity(Level *level, Entity *e, Game *game) {
    level_collide_x(level, e);
    level_collide_y(level, e, game);
}

// Test level: a simple platforming course for Phase 1
void level_load_test(Level *level) {
    level->width = 100;
    level->height = TILES_Y;
    level->tiles = calloc(level->width * level->height, sizeof(int));
    level->bg_color = COLOR_BG;
    level->spawns = NULL;
    level->spawn_count = 0;
    level->next_spawn = 0;

    // Ground: rows 13 and 14 (bottom two rows)
    for (int x = 0; x < level->width; x++) {
        level->tiles[13 * level->width + x] = TILE_GROUND;
        level->tiles[14 * level->width + x] = TILE_GROUND;
    }

    // Pit 1: x = 18-19
    for (int x = 18; x <= 19; x++) {
        level->tiles[13 * level->width + x] = TILE_EMPTY;
        level->tiles[14 * level->width + x] = TILE_EMPTY;
    }

    // Pit 2: x = 40-42
    for (int x = 40; x <= 42; x++) {
        level->tiles[13 * level->width + x] = TILE_EMPTY;
        level->tiles[14 * level->width + x] = TILE_EMPTY;
    }

    // Question blocks row at y=9, x = 5,6,7
    level->tiles[9 * level->width + 5] = TILE_QUESTION;
    level->tiles[9 * level->width + 6] = TILE_BRICK;
    level->tiles[9 * level->width + 7] = TILE_QUESTION;

    // Brick platform at y=9, x = 12-16
    for (int x = 12; x <= 16; x++) {
        level->tiles[9 * level->width + x] = TILE_BRICK;
    }

    // Elevated bricks at y=6, x = 22-25
    for (int x = 22; x <= 25; x++) {
        level->tiles[6 * level->width + x] = TILE_BRICK;
    }

    // Staircase: x=28-32 (ascending right)
    for (int step = 0; step < 5; step++) {
        int x = 28 + step;
        for (int row = 12 - step; row <= 12; row++) {
            level->tiles[row * level->width + x] = TILE_GROUND;
        }
    }

    // Pipe at x=10
    level->tiles[11 * level->width + 10] = TILE_PIPE_TL;
    level->tiles[11 * level->width + 11] = TILE_PIPE_TR;
    level->tiles[12 * level->width + 10] = TILE_PIPE_BL;
    level->tiles[12 * level->width + 11] = TILE_PIPE_BR;

    // Taller pipe at x=35
    level->tiles[10 * level->width + 35] = TILE_PIPE_TL;
    level->tiles[10 * level->width + 36] = TILE_PIPE_TR;
    level->tiles[11 * level->width + 35] = TILE_PIPE_BL;
    level->tiles[11 * level->width + 36] = TILE_PIPE_BR;
    level->tiles[12 * level->width + 35] = TILE_PIPE_BL;
    level->tiles[12 * level->width + 36] = TILE_PIPE_BR;

    // More question blocks near pit 2, y=9 x=38
    level->tiles[9 * level->width + 38] = TILE_QUESTION;

    // Hard blocks at y=9, x=50-52
    for (int x = 50; x <= 52; x++) {
        level->tiles[9 * level->width + x] = TILE_HARD;
    }

    // Bricks at y=5, x=55-60
    for (int x = 55; x <= 60; x++) {
        level->tiles[5 * level->width + x] = TILE_BRICK;
    }

    // Question blocks at y=9, x=55, 58
    level->tiles[9 * level->width + 55] = TILE_QUESTION;
    level->tiles[9 * level->width + 58] = TILE_QUESTION;

    // Staircase descending: x=70-74
    for (int step = 0; step < 5; step++) {
        int x = 70 + step;
        for (int row = 8 + step; row <= 12; row++) {
            level->tiles[row * level->width + x] = TILE_GROUND;
        }
    }

    // Flagpole at x=90
    for (int y = 3; y <= 12; y++) {
        level->tiles[y * level->width + 90] = TILE_FLAGPOLE;
    }
    level->tiles[12 * level->width + 90] = TILE_FLAGPOLE_BASE;
}

void level_free(Level *level) {
    if (level->tiles) {
        free(level->tiles);
        level->tiles = NULL;
    }
    if (level->spawns) {
        free(level->spawns);
        level->spawns = NULL;
    }
}
