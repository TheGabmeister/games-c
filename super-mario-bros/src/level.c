#include "level.h"
#include "game.h"
#include "sprites.h"
#include "sounds.h"
#include "items.h"
#include "enemies/goomba.h"
#include "enemies/koopa.h"
#include "enemies/piranha.h"
#include "enemies/firebar.h"
#include "enemies/podoboo.h"
#include "enemies/bowser.h"
#include "enemies/paratroopa.h"
#include "enemies/springboard.h"
#include "enemies/blooper.h"
#include "enemies/cheep_cheep.h"
#include "enemies/hammer_bro.h"
#include "enemies/lakitu.h"
#include "enemies/spiny.h"
#include "enemies/buzzy_beetle.h"
#include "enemies/bullet_bill.h"
#include "enemies/vine.h"
#include <stdlib.h>
#include <stdio.h>

bool tile_is_solid(int tile_type) {
    switch (tile_type) {
        case TILE_GROUND:
        case TILE_BRICK:
        case TILE_QUESTION:
        case TILE_USED:
        case TILE_HARD:
        case TILE_VINE_BLOCK:
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

// --- Spawn registry ---
// Maps EntityType to spawn function + metadata. Adding a new enemy type:
// 1. Add the #include above  2. Add one entry here  3. Add to parse_spawn_type

typedef void (*SpawnFunc)(Entity entities[MAX_ENTITIES], float x, float y, int extra);

typedef struct {
    SpawnFunc spawn;
    bool preactivate;
    bool respawns;
} SpawnInfo;

static const SpawnInfo spawn_registry[ENT_TYPE_COUNT] = {
    [ENT_GOOMBA]       = { spawn_goomba,       false, false },
    [ENT_KOOPA]        = { spawn_koopa,         false, false },
    [ENT_PIRANHA]      = { spawn_piranha,       false, false },
    [ENT_FIREBAR]      = { spawn_firebar,       true,  false },
    [ENT_PODOBOO]      = { spawn_podoboo,       true,  false },
    [ENT_BOWSER]       = { spawn_bowser,        false, false },
    [ENT_PARATROOPA]   = { spawn_paratroopa,    false, false },
    [ENT_SPRINGBOARD]  = { spawn_springboard,   false, false },
    [ENT_BLOOPER]      = { spawn_blooper,       false, false },
    [ENT_CHEEP_CHEEP]  = { spawn_cheep_cheep,   false, false },
    [ENT_HAMMER_BRO]   = { spawn_hammer_bro,    false, false },
    [ENT_LAKITU]       = { spawn_lakitu,        true,  true  },
    [ENT_SPINY]        = { spawn_spiny,         false, false },
    [ENT_BUZZY_BEETLE] = { spawn_buzzy_beetle,  false, false },
    [ENT_BULLET_BILL]  = { spawn_bullet_bill,   false, false },
    [ENT_BALANCE_LIFT] = { spawn_balance_lift,   true,  false },
};

void level_activate_spawns(Level *level, Entity entities[MAX_ENTITIES], float camera_x) {
    float activate_x = camera_x + WINDOW_WIDTH + TILE_SIZE;
    for (int i = 0; i < level->spawn_count; i++) {
        EntitySpawn *s = &level->spawns[i];
        // Leaping cheep-cheeps respawn: reset if no matching entity exists
        if (s->activated && s->type == ENT_CHEEP_CHEEP && s->extra == 2) {
            bool still_alive = false;
            for (int j = 0; j < MAX_ENTITIES; j++) {
                if (entities[j].type == ENT_CHEEP_CHEEP && entities[j].state_val == 2) {
                    float dist = fabsf(entities[j].x - (float)(s->tile_x * TILE_SIZE));
                    if (dist < WINDOW_WIDTH) { still_alive = true; break; }
                }
            }
            if (!still_alive) s->activated = false;
        }
        // Respawning entities (e.g. lakitu): reset if no live instance exists
        if (s->activated && spawn_registry[s->type].respawns) {
            bool still_alive = false;
            for (int j = 0; j < MAX_ENTITIES; j++) {
                if (entities[j].type == s->type) { still_alive = true; break; }
            }
            if (!still_alive) s->activated = false;
        }
        if (s->activated) continue;
        float sx = s->tile_x * TILE_SIZE;
        bool in_range = (sx <= activate_x && sx >= camera_x - TILE_SIZE * 2);
        if (in_range || spawn_registry[s->type].preactivate) {
            s->activated = true;
            float spawn_x = (float)(s->tile_x * TILE_SIZE);
            float spawn_y = (float)(s->tile_y * TILE_SIZE);
            if (spawn_registry[s->type].spawn)
                spawn_registry[s->type].spawn(entities, spawn_x, spawn_y, s->extra);
        }
    }
}

void level_update_blasters(Level *level, Entity entities[MAX_ENTITIES], int mario_idx, float camera_x, float *timer) {
    float dt = GetFrameTime();
    *timer += dt;
    if (*timer < BLASTER_FIRE_INTERVAL) return;
    *timer -= BLASTER_FIRE_INTERVAL;

    Entity *mario = &entities[mario_idx];
    int start_tx = (int)(camera_x / TILE_SIZE);
    int end_tx = start_tx + TILES_X + 1;
    if (end_tx > level->width) end_tx = level->width;

    int blasters[32];
    int count = 0;
    for (int ty = 0; ty < level->height && count < 32; ty++) {
        for (int tx = start_tx; tx < end_tx && count < 32; tx++) {
            if (level_get_tile(level, tx, ty) != TILE_BILL_BLASTER) continue;
            float bx = (float)(tx * TILE_SIZE);
            float by = (float)(ty * TILE_SIZE);
            float dx = fabsf(mario->x - bx);
            float dy = fabsf(mario->y - by);
            if (dx < BLASTER_MARIO_DIST && dy < TILE_SIZE * 2) continue;
            blasters[count++] = ty * level->width + tx;
        }
    }
    if (count == 0) return;

    int pick = GetRandomValue(0, count - 1);
    int tx = blasters[pick] % level->width;
    int ty = blasters[pick] / level->width;
    float bx = (float)(tx * TILE_SIZE);
    float by = (float)(ty * TILE_SIZE);
    int dir = (mario->x < bx) ? -1 : 1;
    spawn_bullet_bill(entities, bx, by, dir);
}

PipeWarp *level_get_warp(Level *level, int tx, int ty) {
    for (int i = 0; i < level->warp_count; i++) {
        if (level->warps[i].pipe_tx == tx && level->warps[i].pipe_ty == ty)
            return &level->warps[i];
    }
    return NULL;
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

    if (tile == TILE_VINE_BLOCK) {
        level_set_tile(level, tx, ty, TILE_USED);
        spawn_vine(game->entities, (float)(tx * TILE_SIZE), (float)(ty * TILE_SIZE));
        sound_play(SND_POWERUP);
    } else if (tile == TILE_QUESTION) {
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
        case TILE_BRIDGE:       return COLOR_BRIDGE;
        case TILE_AXE:          return COLOR_AXE;
        case TILE_LAVA:         return COLOR_LAVA;
        case TILE_BILL_BLASTER: return (Color){40, 40, 40, 255};
        case TILE_CORAL:        return (Color){255, 127, 80, 255};
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

            if (tile == TILE_LAVA) {
                float pulse = sinf((float)GetTime() * 3.0f + tx * 0.5f) * 0.15f + 0.85f;
                c.r = (unsigned char)(c.r * pulse);
                DrawRectangle((int)draw_x, (int)draw_y, TILE_SIZE, TILE_SIZE, c);
                Color highlight = {255, 200, 0, 80};
                DrawRectangle((int)draw_x, (int)draw_y, TILE_SIZE, 8, highlight);
            } else if (tile == TILE_BILL_BLASTER) {
                DrawRectangle((int)(draw_x + 8), (int)draw_y, TILE_SIZE - 16, TILE_SIZE, c);
                DrawRectangle((int)(draw_x + 4), (int)draw_y, TILE_SIZE - 8, 16, (Color){60, 60, 60, 255});
                DrawCircle((int)(draw_x + TILE_SIZE / 2), (int)(draw_y + TILE_SIZE / 2), 8, (Color){20, 20, 20, 255});
            } else if (tile == TILE_AXE) {
                float bob = sinf((float)GetTime() * 4.0f) * 4.0f;
                DrawRectangle((int)(draw_x + 16), (int)(draw_y + 8 + bob), 32, 48, c);
                DrawRectangle((int)(draw_x + 8), (int)(draw_y + 8 + bob), 48, 16, c);
            } else {
                DrawRectangle((int)draw_x, (int)draw_y, TILE_SIZE, TILE_SIZE, c);
            }
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

// --- Tile char mapping ---

static int char_to_tile(char c) {
    switch (c) {
        case 'G': return TILE_GROUND;
        case 'B': return TILE_BRICK;
        case 'Q': return TILE_QUESTION;
        case 'U': return TILE_USED;
        case 'H': return TILE_HARD;
        case '[': return TILE_PIPE_TL;
        case ']': return TILE_PIPE_TR;
        case '{': return TILE_PIPE_BL;
        case '}': return TILE_PIPE_BR;
        case 'F': return TILE_FLAGPOLE;
        case 'f': return TILE_FLAGPOLE_BASE;
        case 'D': return TILE_CASTLE_DOOR;
        case 'I': return TILE_INVISIBLE;
        case 'C': return TILE_CORAL;
        case '=': return TILE_BRIDGE;
        case 'X': return TILE_AXE;
        case 'V': return TILE_VINE_BLOCK;
        case 'T': return TILE_BILL_BLASTER;
        case 'L': return TILE_LAVA;
        default:  return TILE_EMPTY;
    }
}

static LevelType parse_level_type(const char *name) {
    if (strcmp(name, "underground") == 0) return LEVEL_UNDERGROUND;
    if (strcmp(name, "castle") == 0) return LEVEL_CASTLE;
    if (strcmp(name, "athletic") == 0) return LEVEL_ATHLETIC;
    if (strcmp(name, "underwater") == 0) return LEVEL_UNDERWATER;
    return LEVEL_OVERWORLD;
}

static int parse_block_content(const char *name) {
    if (strcmp(name, "mushroom") == 0) return BLOCK_MUSHROOM;
    if (strcmp(name, "fire_flower") == 0) return BLOCK_FIRE_FLOWER;
    if (strcmp(name, "starman") == 0) return BLOCK_STARMAN;
    if (strcmp(name, "oneup") == 0) return BLOCK_ONEUP;
    if (strcmp(name, "multi_coin") == 0) return BLOCK_MULTI_COIN;
    return BLOCK_COIN;
}

static int parse_spawn_type(const char *name) {
    if (strcmp(name, "goomba") == 0) return ENT_GOOMBA;
    if (strcmp(name, "koopa") == 0) return ENT_KOOPA;
    if (strcmp(name, "piranha") == 0) return ENT_PIRANHA;
    if (strcmp(name, "firebar") == 0) return ENT_FIREBAR;
    if (strcmp(name, "podoboo") == 0) return ENT_PODOBOO;
    if (strcmp(name, "bowser") == 0) return ENT_BOWSER;
    if (strcmp(name, "lift") == 0) return ENT_BALANCE_LIFT;
    if (strcmp(name, "paratroopa") == 0) return ENT_PARATROOPA;
    if (strcmp(name, "springboard") == 0) return ENT_SPRINGBOARD;
    if (strcmp(name, "blooper") == 0) return ENT_BLOOPER;
    if (strcmp(name, "cheep") == 0) return ENT_CHEEP_CHEEP;
    if (strcmp(name, "hammer_bro") == 0) return ENT_HAMMER_BRO;
    if (strcmp(name, "lakitu") == 0) return ENT_LAKITU;
    if (strcmp(name, "spiny") == 0) return ENT_SPINY;
    if (strcmp(name, "buzzy") == 0) return ENT_BUZZY_BEETLE;
    if (strcmp(name, "bullet_bill") == 0) return ENT_BULLET_BILL;
    return ENT_NONE;
}

// --- File-based level loading ---

#define LINE_BUF 1024

static bool level_load_from_file(Level *level, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;

    char line[LINE_BUF];
    char section[32] = "";

    level->block_count = 0;
    level->spawn_count = 0;
    level->warp_count = 0;
    level->bridge_start_tx = 0;
    level->bridge_end_tx = 0;
    level->bridge_ty = 0;
    level->type = LEVEL_OVERWORLD;
    level->bg_color = COLOR_BG;
    level->tiles = NULL;
    level->width = 0;
    level->height = 0;

    int tile_row_count = 0;
    int max_width = 0;
    char tile_rows[TILES_Y][LINE_BUF];

    while (fgets(line, LINE_BUF, f)) {
        int len = (int)strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';

        if (len == 0 || line[0] == '#') continue;

        if (strcmp(line, "tiles") == 0) { strcpy(section, "tiles"); continue; }
        if (strcmp(line, "blocks") == 0) { strcpy(section, "blocks"); continue; }
        if (strcmp(line, "spawns") == 0) { strcpy(section, "spawns"); continue; }
        if (strcmp(line, "warps") == 0) { strcpy(section, "warps"); continue; }

        if (section[0] == '\0') {
            char key[32], val[64];
            if (sscanf(line, "%31s %63[^\n]", key, val) >= 2) {
                if (strcmp(key, "type") == 0) {
                    level->type = parse_level_type(val);
                } else if (strcmp(key, "bg") == 0) {
                    int r, g, b;
                    if (sscanf(val, "%d %d %d", &r, &g, &b) == 3)
                        level->bg_color = (Color){(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
                } else if (strcmp(key, "bridge") == 0) {
                    sscanf(val, "%d %d %d", &level->bridge_start_tx, &level->bridge_end_tx, &level->bridge_ty);
                }
            }
            continue;
        }

        if (strcmp(section, "tiles") == 0) {
            if (tile_row_count < TILES_Y) {
                strncpy(tile_rows[tile_row_count], line, LINE_BUF - 1);
                tile_rows[tile_row_count][LINE_BUF - 1] = '\0';
                if (len > max_width) max_width = len;
                tile_row_count++;
            }
        } else if (strcmp(section, "blocks") == 0) {
            int tx, ty;
            char content_name[32];
            if (sscanf(line, "%d %d %31s", &tx, &ty, content_name) == 3) {
                set_block_content(level, tx, ty, parse_block_content(content_name));
            }
        } else if (strcmp(section, "spawns") == 0) {
            int tx, ty, extra = 0;
            char type_name[32], extra_str[32] = "";
            int n = sscanf(line, "%d %d %31s %31s", &tx, &ty, type_name, extra_str);
            if (n >= 3) {
                int etype = parse_spawn_type(type_name);
                if (extra_str[0]) {
                    if (strcmp(extra_str, "red") == 0 || strcmp(extra_str, "cw") == 0)
                        extra = 1;
                    else
                        extra = atoi(extra_str);
                }
                if (level->spawn_count < MAX_SPAWNS) {
                    EntitySpawn *s = &level->spawns[level->spawn_count++];
                    s->type = etype;
                    s->tile_x = tx;
                    s->tile_y = ty;
                    s->extra = extra;
                    s->activated = false;
                }
            }
        } else if (strcmp(section, "warps") == 0) {
            int ptx, pty, dw, ds, dtx, dty;
            if (sscanf(line, "%d %d %d %d %d %d", &ptx, &pty, &dw, &ds, &dtx, &dty) == 6) {
                if (level->warp_count < MAX_PIPE_WARPS) {
                    PipeWarp *w = &level->warps[level->warp_count++];
                    w->pipe_tx = ptx;
                    w->pipe_ty = pty;
                    w->dest_world = dw;
                    w->dest_sublevel = ds;
                    w->dest_tx = dtx;
                    w->dest_ty = dty;
                }
            }
        }
    }
    fclose(f);

    level->width = max_width;
    level->height = tile_row_count > 0 ? tile_row_count : TILES_Y;
    level->tiles = calloc(level->width * level->height, sizeof(int));

    for (int ty = 0; ty < tile_row_count; ty++) {
        int row_len = (int)strlen(tile_rows[ty]);
        for (int tx = 0; tx < row_len; tx++) {
            level->tiles[ty * level->width + tx] = char_to_tile(tile_rows[ty][tx]);
        }
    }

    return true;
}

// --- Level loading ---

void level_load(Level *level, int world, int sublevel) {
    char path[128];
    snprintf(path, sizeof(path), "resources/levels/%d-%d.txt", world, sublevel);

    if (level_load_from_file(level, path))
        return;

    // Fallback: load 1-1 if file not found
    if (world != 1 || sublevel != 1) {
        snprintf(path, sizeof(path), "resources/levels/1-1.txt");
        level_load_from_file(level, path);
    }
}

void level_free(Level *level) {
    if (level->tiles) {
        free(level->tiles);
        level->tiles = NULL;
    }
}
