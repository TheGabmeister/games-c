#include "level.h"
#include "game.h"
#include "sprites.h"
#include "sounds.h"
#include "items.h"
#include "enemies/goomba.h"
#include "enemies/koopa.h"
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

int level_get_block_content(Level *level, int tx, int ty) {
    for (int i = 0; i < level->block_count; i++) {
        if (level->blocks[i].tile_x == tx && level->blocks[i].tile_y == ty)
            return level->blocks[i].content;
    }
    return BLOCK_COIN;
}

static void set_block_content(Level *level, int tx, int ty, int content) {
    if (level->block_count >= MAX_BLOCK_CONTENTS) return;
    BlockContent *b = &level->blocks[level->block_count++];
    b->tile_x = tx;
    b->tile_y = ty;
    b->content = content;
}

static void add_spawn(Level *level, int type, int tx, int ty, int extra) {
    if (level->spawn_count >= MAX_SPAWNS) return;
    EntitySpawn *s = &level->spawns[level->spawn_count++];
    s->type = type;
    s->tile_x = tx;
    s->tile_y = ty;
    s->extra = extra;
    s->activated = false;
}

void level_activate_spawns(Level *level, Entity entities[MAX_ENTITIES], float camera_x) {
    float activate_x = camera_x + WINDOW_WIDTH + TILE_SIZE;
    for (int i = 0; i < level->spawn_count; i++) {
        EntitySpawn *s = &level->spawns[i];
        if (s->activated) continue;
        float sx = s->tile_x * TILE_SIZE;
        if (sx <= activate_x && sx >= camera_x - TILE_SIZE * 2) {
            s->activated = true;
            float spawn_x = (float)(s->tile_x * TILE_SIZE);
            float spawn_y = (float)(s->tile_y * TILE_SIZE);
            switch (s->type) {
                case ENT_GOOMBA:
                    spawn_goomba(entities, spawn_x, spawn_y);
                    break;
                case ENT_KOOPA:
                    spawn_koopa(entities, spawn_x, spawn_y, s->extra != 0);
                    break;
            }
        }
    }
}

static void spawn_item_from_block(Level *level, int tx, int ty, Entity *mario, Game *game) {
    int content = level_get_block_content(level, tx, ty);
    float x = (float)(tx * TILE_SIZE);
    float y = (float)(ty * TILE_SIZE);

    switch (content) {
        case BLOCK_COIN:
            game->coins++;
            game->score += SCORE_COIN;
            sound_play(SND_COIN);
            if (game->coins >= 100) {
                game->coins -= 100;
                game->lives++;
            }
            spawn_coin_popup(game->entities, x, y);
            break;
        case BLOCK_MUSHROOM:
            if (mario->power >= MARIO_BIG)
                spawn_fire_flower(game->entities, x, y);
            else
                spawn_mushroom(game->entities, x, y);
            break;
        case BLOCK_FIRE_FLOWER:
            if (mario->power >= MARIO_BIG)
                spawn_fire_flower(game->entities, x, y);
            else
                spawn_mushroom(game->entities, x, y);
            break;
        case BLOCK_STARMAN:
            spawn_starman(game->entities, x, y);
            break;
        case BLOCK_ONEUP:
            spawn_oneup(game->entities, x, y);
            break;
        case BLOCK_MULTI_COIN:
            game->coins++;
            game->score += SCORE_COIN;
            sound_play(SND_COIN);
            if (game->coins >= 100) {
                game->coins -= 100;
                game->lives++;
            }
            spawn_coin_popup(game->entities, x, y);
            break;
    }
}

static void kill_enemies_on_tile(Game *game, int tx, int ty) {
    float tile_l = (float)(tx * TILE_SIZE);
    float tile_r = tile_l + TILE_SIZE;
    float tile_t = (float)(ty * TILE_SIZE);

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (e->type == ENT_NONE || e->type == ENT_MARIO) continue;
        if (!e->damages_mario) continue;
        // Check if enemy is standing on this tile
        if (e->on_ground &&
            e->x + e->w > tile_l && e->x < tile_r &&
            fabsf((e->y + e->h) - tile_t) < 4.0f) {
            if (e->vtab && e->vtab->bumped)
                e->vtab->bumped(e, game);
        }
    }
}

void level_handle_head_bump(Level *level, Entity *e, int tx, int ty, Game *game) {
    int tile = level_get_tile(level, tx, ty);

    if (tile == TILE_QUESTION) {
        level_set_tile(level, tx, ty, TILE_USED);
        spawn_item_from_block(level, tx, ty, e, game);
        kill_enemies_on_tile(game, tx, ty);
        sound_play(SND_BUMP);
    } else if (tile == TILE_BRICK) {
        kill_enemies_on_tile(game, tx, ty);
        if (e->power >= MARIO_BIG) {
            level_set_tile(level, tx, ty, TILE_EMPTY);
            spawn_brick_debris(game->entities, (float)(tx * TILE_SIZE), (float)(ty * TILE_SIZE));
            sound_play(SND_BRICK_BREAK);
        } else {
            sound_play(SND_BUMP);
        }
    }
}

// --- Drawing ---

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

            Color c = tile_color(tile);
            if (c.a == 0) continue;
            DrawRectangle((int)draw_x, (int)draw_y, TILE_SIZE, TILE_SIZE, c);
            Color border = {0, 0, 0, 60};
            DrawRectangleLines((int)draw_x, (int)draw_y, TILE_SIZE, TILE_SIZE, border);
        }
    }
}

// --- Collision ---

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

            float overlap_l = e->x + e->w - tile_l;
            float overlap_r = tile_r - e->x;

            if (overlap_l > 0 && overlap_r > 0) {
                if (e->vx > 0) {
                    e->x = tile_l - e->w;
                    e->vx = (e->type == ENT_MARIO) ? 0 : -e->vx;
                    if (e->type != ENT_MARIO) e->facing = (e->vx < 0) ? DIR_LEFT : DIR_RIGHT;
                } else if (e->vx < 0) {
                    e->x = tile_r;
                    e->vx = (e->type == ENT_MARIO) ? 0 : -e->vx;
                    if (e->type != ENT_MARIO) e->facing = (e->vx < 0) ? DIR_LEFT : DIR_RIGHT;
                } else {
                    if (overlap_l < overlap_r)
                        e->x = tile_l - e->w;
                    else
                        e->x = tile_r;
                }
            }
        }
    }

    // Fireball: despawn on wall hit
    if (e->type == ENT_FIREBALL) {
        int fl = (int)(e->x / TILE_SIZE);
        int fr = (int)((e->x + e->w - 1) / TILE_SIZE);
        int ft = (int)(e->y / TILE_SIZE);
        int fb = (int)((e->y + e->h - 1) / TILE_SIZE);
        for (int ty = ft; ty <= fb; ty++) {
            for (int tx = fl; tx <= fr; tx++) {
                if (tile_is_solid(level_get_tile(level, tx, ty))) {
                    entity_deactivate(e);
                    return;
                }
            }
        }
    }
}

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
                        level_handle_head_bump(level, e, tx, ty, game);
                    }
                }
            }
        }
    }

    // Pit death
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

// --- Level 1-1 ---

static void set_tile(Level *level, int tx, int ty, int type) {
    level_set_tile(level, tx, ty, type);
}

static void fill_ground(Level *level, int x_start, int x_end) {
    for (int x = x_start; x <= x_end; x++) {
        set_tile(level, x, 13, TILE_GROUND);
        set_tile(level, x, 14, TILE_GROUND);
    }
}

static void place_pipe(Level *level, int tx, int height) {
    int top_y = 13 - height;
    set_tile(level, tx, top_y, TILE_PIPE_TL);
    set_tile(level, tx + 1, top_y, TILE_PIPE_TR);
    for (int y = top_y + 1; y <= 12; y++) {
        set_tile(level, tx, y, TILE_PIPE_BL);
        set_tile(level, tx + 1, y, TILE_PIPE_BR);
    }
}

static void place_qblock(Level *level, int tx, int ty, int content) {
    set_tile(level, tx, ty, TILE_QUESTION);
    set_block_content(level, tx, ty, content);
}

void level_load_1_1(Level *level) {
    level->width = 224;
    level->height = TILES_Y;
    level->tiles = calloc(level->width * level->height, sizeof(int));
    level->bg_color = COLOR_BG;
    level->block_count = 0;
    level->spawn_count = 0;

    // Ground (with gaps for pits)
    fill_ground(level, 0, 68);
    // Pit at 69-70
    fill_ground(level, 71, 85);
    // Pit at 86-88
    fill_ground(level, 89, 152);
    // Pit at 153-154
    fill_ground(level, 155, 223);

    // Question block with coin at (16, 9)
    place_qblock(level, 16, 9, BLOCK_COIN);

    // Blocks at y=9: brick, question(mushroom), brick, question(coin), brick
    set_tile(level, 20, 9, TILE_BRICK);
    place_qblock(level, 21, 9, BLOCK_MUSHROOM);
    set_tile(level, 22, 9, TILE_BRICK);
    place_qblock(level, 23, 9, BLOCK_COIN);
    set_tile(level, 24, 9, TILE_BRICK);

    // Hidden 1-Up above question row
    place_qblock(level, 22, 5, BLOCK_ONEUP);

    // Pipe 1 (height 2) at x=28
    place_pipe(level, 28, 2);
    // Pipe 2 (height 3) at x=38
    place_pipe(level, 38, 3);
    // Pipe 3 (height 4) at x=46
    place_pipe(level, 46, 4);
    // Pipe 4 (height 4) at x=57
    place_pipe(level, 57, 4);

    // Bricks and question blocks above first ground gap area
    set_tile(level, 77, 9, TILE_BRICK);
    place_qblock(level, 78, 9, BLOCK_MUSHROOM);
    set_tile(level, 79, 9, TILE_BRICK);

    // Elevated bricks at y=5
    set_tile(level, 80, 5, TILE_BRICK);
    set_tile(level, 81, 5, TILE_BRICK);
    set_tile(level, 82, 5, TILE_BRICK);
    set_tile(level, 83, 5, TILE_BRICK);
    set_tile(level, 84, 5, TILE_BRICK);
    set_tile(level, 85, 5, TILE_BRICK);
    set_tile(level, 86, 5, TILE_BRICK);
    set_tile(level, 87, 5, TILE_BRICK);

    // After second pit: bricks at y=5 and y=9
    set_tile(level, 91, 5, TILE_BRICK);
    set_tile(level, 92, 5, TILE_BRICK);
    set_tile(level, 93, 5, TILE_BRICK);

    place_qblock(level, 94, 5, BLOCK_COIN);

    set_tile(level, 94, 9, TILE_BRICK);

    // Star block
    place_qblock(level, 100, 9, BLOCK_STARMAN);

    // Question blocks and bricks mid-level
    set_tile(level, 106, 9, TILE_BRICK);
    set_tile(level, 107, 9, TILE_BRICK);
    place_qblock(level, 109, 9, BLOCK_COIN);
    set_tile(level, 110, 9, TILE_BRICK);

    place_qblock(level, 109, 5, BLOCK_COIN);

    set_tile(level, 118, 9, TILE_BRICK);
    set_tile(level, 119, 9, TILE_BRICK);
    set_tile(level, 120, 9, TILE_BRICK);

    set_tile(level, 128, 9, TILE_BRICK);
    place_qblock(level, 129, 9, BLOCK_COIN);
    place_qblock(level, 130, 9, BLOCK_COIN);
    set_tile(level, 131, 9, TILE_BRICK);

    // Staircase 1: ascending (x=134-137)
    for (int step = 0; step < 4; step++) {
        int x = 134 + step;
        for (int row = 12 - step; row <= 12; row++)
            set_tile(level, x, row, TILE_GROUND);
    }

    // Staircase 1: descending (x=140-143)
    for (int step = 0; step < 4; step++) {
        int x = 140 + step;
        for (int row = 9 + step; row <= 12; row++)
            set_tile(level, x, row, TILE_GROUND);
    }

    // Staircase 2: ascending (x=148-152)
    for (int step = 0; step < 5; step++) {
        int x = 148 + step;
        for (int row = 12 - step; row <= 12; row++)
            set_tile(level, x, row, TILE_GROUND);
    }

    // Staircase 2: descending (x=155-158)
    for (int step = 0; step < 4; step++) {
        int x = 155 + step;
        for (int row = 9 + step; row <= 12; row++)
            set_tile(level, x, row, TILE_GROUND);
    }

    // More blocks
    set_tile(level, 168, 9, TILE_BRICK);
    set_tile(level, 169, 9, TILE_BRICK);
    place_qblock(level, 170, 9, BLOCK_COIN);
    set_tile(level, 171, 9, TILE_BRICK);

    // Pipe near end
    place_pipe(level, 179, 2);

    // Brick rows
    set_tile(level, 189, 9, TILE_BRICK);
    set_tile(level, 190, 9, TILE_BRICK);
    place_qblock(level, 191, 9, BLOCK_COIN);

    // Final staircase to flagpole (x=196-203)
    for (int step = 0; step < 8; step++) {
        int x = 196 + step;
        for (int row = 12 - step; row <= 12; row++)
            set_tile(level, x, row, TILE_GROUND);
    }

    // Flagpole at x=206
    for (int y = 3; y <= 12; y++)
        set_tile(level, 206, y, TILE_FLAGPOLE);
    set_tile(level, 206, 12, TILE_FLAGPOLE_BASE);

    // --- Enemy spawns ---
    add_spawn(level, ENT_GOOMBA, 22, 12, 0);
    add_spawn(level, ENT_GOOMBA, 40, 12, 0);
    add_spawn(level, ENT_GOOMBA, 51, 12, 0);
    add_spawn(level, ENT_GOOMBA, 52, 12, 0);
    add_spawn(level, ENT_KOOPA, 107, 12, 0); // green koopa
    add_spawn(level, ENT_GOOMBA, 80, 4, 0);
    add_spawn(level, ENT_GOOMBA, 82, 4, 0);
    add_spawn(level, ENT_GOOMBA, 97, 12, 0);
    add_spawn(level, ENT_GOOMBA, 98, 12, 0);
    add_spawn(level, ENT_GOOMBA, 114, 12, 0);
    add_spawn(level, ENT_GOOMBA, 115, 12, 0);
    add_spawn(level, ENT_GOOMBA, 124, 12, 0);
    add_spawn(level, ENT_GOOMBA, 125, 12, 0);
    add_spawn(level, ENT_GOOMBA, 128, 12, 0);
    add_spawn(level, ENT_GOOMBA, 129, 12, 0);
    add_spawn(level, ENT_GOOMBA, 174, 12, 0);
    add_spawn(level, ENT_GOOMBA, 175, 12, 0);
}

void level_load_test(Level *level) {
    level_load_1_1(level);
}

void level_free(Level *level) {
    if (level->tiles) {
        free(level->tiles);
        level->tiles = NULL;
    }
}
