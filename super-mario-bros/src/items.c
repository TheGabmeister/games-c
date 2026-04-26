#include "items.h"
#include "game.h"
#include "sounds.h"

// --- Floating Coin (collectible in air) ---

static void coin_update(Entity *self, Game *game) {
    (void)game;
    self->anim_timer += GetFrameTime();
    if (self->x < game->camera_x - TILE_SIZE * 2)
        entity_deactivate(self);
}

static void coin_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    float wobble = sinf(self->anim_timer * 6.0f) * 0.3f + 0.7f;
    float w = self->w * wobble;
    DrawRectangle((int)(dx + (self->w - w) / 2), (int)dy, (int)w, (int)self->h, COLOR_COIN_ENT);
}

static void coin_touch(Entity *self, Entity *other, Game *game) {
    if (other->type == ENT_MARIO) {
        game->coins++;
        game->score += SCORE_COIN;
        sound_play(SND_COIN);
        if (game->coins >= 100) {
            game->coins -= 100;
            game->lives++;
        }
        entity_deactivate(self);
    }
}

static const EntityVtab coin_vtab = {
    .update = coin_update,
    .draw   = coin_draw,
    .touch  = coin_touch,
};

void spawn_coin_entity(Entity entities[MAX_ENTITIES], float x, float y) {
    Entity *e = entity_alloc(entities);
    if (!e) return;
    e->type = ENT_COIN;
    e->vtab = &coin_vtab;
    e->x = x;
    e->y = y;
    e->w = COIN_W;
    e->h = COIN_H;
    e->active = true;
}

// --- Mushroom (Super Mushroom) ---

static void mushroom_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    // Rise from block
    if (self->state_val == 0) {
        self->state_timer += dt;
        self->y -= 60.0f * dt;
        if (self->state_timer >= 0.8f) {
            self->state_val = 1;
            self->self_moving = false;
            self->vx = MUSHROOM_SPEED;
            self->facing = DIR_RIGHT;
        }
        return;
    }
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;
    if (self->x < game->camera_x - WINDOW_WIDTH)
        entity_deactivate(self);
}

static void mushroom_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_MUSHROOM);
    // White spots
    DrawCircle((int)(dx + self->w * 0.3f), (int)(dy + self->h * 0.3f), 6, WHITE);
    DrawCircle((int)(dx + self->w * 0.7f), (int)(dy + self->h * 0.3f), 6, WHITE);
}

static void mushroom_touch(Entity *self, Entity *other, Game *game) {
    if (other->type == ENT_MARIO) {
        game->score += SCORE_POWERUP;
        sound_play(SND_POWERUP);
        if (other->power == MARIO_SMALL) {
            other->power = MARIO_BIG;
            other->y -= (MARIO_BIG_H - MARIO_SMALL_H);
            other->h = MARIO_BIG_H;
        }
        entity_deactivate(self);
    }
}

static const EntityVtab mushroom_vtab = {
    .update = mushroom_update,
    .draw   = mushroom_draw,
    .touch  = mushroom_touch,
};

void spawn_mushroom(Entity entities[MAX_ENTITIES], float x, float y) {
    Entity *e = entity_alloc(entities);
    if (!e) return;
    e->type = ENT_MUSHROOM;
    e->vtab = &mushroom_vtab;
    e->x = x;
    e->y = y;
    e->w = ITEM_W;
    e->h = ITEM_H;
    e->active = true;
    e->state_val = 0;
    e->state_timer = 0;
    e->self_moving = true;
}

// --- Fire Flower ---

static void flower_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    // Rise from block
    if (self->state_val == 0) {
        self->state_timer += dt;
        self->y -= 60.0f * dt;
        if (self->state_timer >= 0.8f) {
            self->state_val = 1;
            self->self_moving = false;
        }
        return;
    }
    self->anim_timer += dt;
}

static void flower_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_FLOWER);
    // Petals
    float pulse = sinf(self->anim_timer * 4.0f) * 4.0f;
    DrawCircle((int)(dx + self->w / 2), (int)(dy + 8 + pulse), 8, YELLOW);
}

static void flower_touch(Entity *self, Entity *other, Game *game) {
    if (other->type == ENT_MARIO) {
        game->score += SCORE_POWERUP;
        sound_play(SND_POWERUP);
        if (other->power == MARIO_SMALL) {
            other->power = MARIO_BIG;
            other->y -= (MARIO_BIG_H - MARIO_SMALL_H);
            other->h = MARIO_BIG_H;
        } else {
            other->power = MARIO_FIRE;
        }
        entity_deactivate(self);
    }
}

static const EntityVtab flower_vtab = {
    .update = flower_update,
    .draw   = flower_draw,
    .touch  = flower_touch,
};

void spawn_fire_flower(Entity entities[MAX_ENTITIES], float x, float y) {
    Entity *e = entity_alloc(entities);
    if (!e) return;
    e->type = ENT_FIRE_FLOWER;
    e->vtab = &flower_vtab;
    e->x = x;
    e->y = y;
    e->w = ITEM_W;
    e->h = ITEM_H;
    e->active = true;
    e->state_val = 0;
    e->state_timer = 0;
    e->self_moving = true;
}

// --- Starman ---

static void star_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    // Rise from block
    if (self->state_val == 0) {
        self->state_timer += dt;
        self->y -= 60.0f * dt;
        if (self->state_timer >= 0.8f) {
            self->state_val = 1;
            self->self_moving = false;
            self->vx = STARMAN_SPEED;
            self->vy = STARMAN_BOUNCE_VEL;
        }
        return;
    }
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;
    // Bounce on ground
    if (self->on_ground) {
        self->vy = STARMAN_BOUNCE_VEL;
    }
    self->anim_timer += dt;
    if (self->x < game->camera_x - WINDOW_WIDTH)
        entity_deactivate(self);
}

static void star_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    float flash = sinf(self->anim_timer * 12.0f);
    Color c = (flash > 0) ? COLOR_STARMAN : WHITE;
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, c);
}

static void star_touch(Entity *self, Entity *other, Game *game) {
    if (other->type == ENT_MARIO) {
        game->score += SCORE_STAR;
        sound_play(SND_POWERUP);
        other->star_active = true;
        other->star_timer = STAR_DURATION;
        other->invincible_timer = 0;
        entity_deactivate(self);
    }
}

static const EntityVtab star_vtab = {
    .update = star_update,
    .draw   = star_draw,
    .touch  = star_touch,
};

void spawn_starman(Entity entities[MAX_ENTITIES], float x, float y) {
    Entity *e = entity_alloc(entities);
    if (!e) return;
    e->type = ENT_STARMAN;
    e->vtab = &star_vtab;
    e->x = x;
    e->y = y;
    e->w = ITEM_W;
    e->h = ITEM_H;
    e->active = true;
    e->state_val = 0;
    e->state_timer = 0;
    e->self_moving = true;
}

// --- 1-Up Mushroom ---

static void oneup_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    if (self->state_val == 0) {
        self->state_timer += dt;
        self->y -= 60.0f * dt;
        if (self->state_timer >= 0.8f) {
            self->state_val = 1;
            self->self_moving = false;
            self->vx = MUSHROOM_SPEED;
            self->facing = DIR_RIGHT;
        }
        return;
    }
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;
    if (self->x < game->camera_x - WINDOW_WIDTH)
        entity_deactivate(self);
}

static void oneup_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_ONEUP);
    DrawCircle((int)(dx + self->w * 0.3f), (int)(dy + self->h * 0.3f), 6, WHITE);
    DrawCircle((int)(dx + self->w * 0.7f), (int)(dy + self->h * 0.3f), 6, WHITE);
}

static void oneup_touch(Entity *self, Entity *other, Game *game) {
    if (other->type == ENT_MARIO) {
        game->lives++;
        sound_play(SND_POWERUP);
        entity_deactivate(self);
    }
}

static const EntityVtab oneup_vtab = {
    .update = oneup_update,
    .draw   = oneup_draw,
    .touch  = oneup_touch,
};

void spawn_oneup(Entity entities[MAX_ENTITIES], float x, float y) {
    Entity *e = entity_alloc(entities);
    if (!e) return;
    e->type = ENT_ONEUP;
    e->vtab = &oneup_vtab;
    e->x = x;
    e->y = y;
    e->w = ITEM_W;
    e->h = ITEM_H;
    e->active = true;
    e->state_val = 0;
    e->state_timer = 0;
    e->self_moving = true;
}

// --- Fireball ---

static void fireball_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;
    if (self->on_ground) {
        self->vy = FIREBALL_BOUNCE_VEL;
    }
    self->anim_timer += dt;

    // Despawn offscreen or after too long
    if (self->x < game->camera_x - TILE_SIZE || self->x > game->camera_x + WINDOW_WIDTH + TILE_SIZE)
        entity_deactivate(self);
    if (self->state_timer > 3.0f)
        entity_deactivate(self);
    self->state_timer += dt;
}

static void fireball_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    DrawCircle((int)(dx + self->w / 2), (int)(dy + self->h / 2), self->w / 2, COLOR_FIREBALL);
}

static const EntityVtab fireball_vtab = {
    .update = fireball_update,
    .draw   = fireball_draw,
};

void spawn_fireball(Entity entities[MAX_ENTITIES], float x, float y, Direction dir) {
    // Count active fireballs
    int count = 0;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].type == ENT_FIREBALL) count++;
    }
    if (count >= MAX_FIREBALLS) return;

    Entity *e = entity_alloc(entities);
    if (!e) return;
    e->type = ENT_FIREBALL;
    e->vtab = &fireball_vtab;
    e->x = x;
    e->y = y;
    e->w = FIREBALL_SIZE;
    e->h = FIREBALL_SIZE;
    e->vx = (dir == DIR_RIGHT) ? FIREBALL_SPEED_X : -FIREBALL_SPEED_X;
    e->vy = 0;
    e->active = true;
    e->facing = dir;
}

// --- Brick Debris ---

static void debris_update(Entity *self, Game *game) {
    (void)game;
    float dt = GetFrameTime();
    self->vy += GRAVITY * dt;
    self->x += self->vx * dt;
    self->y += self->vy * dt;
    self->state_timer += dt;
    if (self->state_timer > 1.5f)
        entity_deactivate(self);
}

static void debris_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    DrawRectangle((int)dx, (int)dy, 16, 16, COLOR_BRICK);
}

static const EntityVtab debris_vtab = {
    .update = debris_update,
    .draw   = debris_draw,
};

void spawn_brick_debris(Entity entities[MAX_ENTITIES], float x, float y) {
    float speeds[][2] = {{-120, -600}, {120, -600}, {-80, -500}, {80, -500}};
    for (int i = 0; i < 4; i++) {
        Entity *e = entity_alloc(entities);
        if (!e) return;
        e->type = ENT_BRICK_DEBRIS;
        e->vtab = &debris_vtab;
        e->x = x + (i % 2) * 32;
        e->y = y + (i / 2) * 32;
        e->w = 16;
        e->h = 16;
        e->vx = speeds[i][0];
        e->vy = speeds[i][1];
        e->active = true;
        e->self_moving = true;
    }
}

// --- Coin Popup (from blocks) ---

static void coin_popup_update(Entity *self, Game *game) {
    (void)game;
    float dt = GetFrameTime();
    self->vy += GRAVITY * 1.5f * dt;
    self->y += self->vy * dt;
    self->state_timer += dt;
    if (self->state_timer > 0.6f)
        entity_deactivate(self);
}

static void coin_popup_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    float wobble = sinf(self->anim_timer + self->state_timer * 12.0f) * 0.4f + 0.6f;
    float w = 24 * wobble;
    DrawRectangle((int)(dx + (32 - w) / 2), (int)dy, (int)w, 32, COLOR_COIN_ENT);
}

static const EntityVtab coin_popup_vtab = {
    .update = coin_popup_update,
    .draw   = coin_popup_draw,
};

void spawn_coin_popup(Entity entities[MAX_ENTITIES], float x, float y) {
    Entity *e = entity_alloc(entities);
    if (!e) return;
    e->type = ENT_COIN_POPUP;
    e->vtab = &coin_popup_vtab;
    e->x = x + 16;
    e->y = y;
    e->w = 32;
    e->h = 32;
    e->vy = -800.0f;
    e->active = true;
    e->self_moving = true;
}

// --- Score Popup ---

static void score_popup_update(Entity *self, Game *game) {
    (void)game;
    float dt = GetFrameTime();
    self->y -= 80.0f * dt;
    self->state_timer += dt;
    if (self->state_timer > 1.0f)
        entity_deactivate(self);
}

static void score_popup_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    float alpha = 1.0f - (self->state_timer / 1.0f);
    Color c = COLOR_TEXT;
    c.a = (unsigned char)(alpha * 255);
    DrawText(TextFormat("%d", self->state_val), (int)dx, (int)dy, 16, c);
}

static const EntityVtab score_popup_vtab = {
    .update = score_popup_update,
    .draw   = score_popup_draw,
};

void spawn_score_popup(Entity entities[MAX_ENTITIES], float x, float y, int score) {
    Entity *e = entity_alloc(entities);
    if (!e) return;
    e->type = ENT_SCORE_POPUP;
    e->vtab = &score_popup_vtab;
    e->x = x;
    e->y = y;
    e->active = true;
    e->state_val = score;
    e->self_moving = true;
}

// --- Balance Lift ---
// Pair of platforms on a pulley. When Mario stands on one, it sinks and the other rises.
// state_val = pair_id, anim_timer = base_y

static Entity *find_lift_partner(Entity entities[MAX_ENTITIES], Entity *self) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &entities[i];
        if (e == self || e->type != ENT_BALANCE_LIFT) continue;
        if (e->state_val == self->state_val) return e;
    }
    return NULL;
}

static bool mario_on_lift(Entity *mario, Entity *lift) {
    return mario->on_ground &&
           mario->x + mario->w > lift->x && mario->x < lift->x + lift->w &&
           fabsf((mario->y + mario->h) - lift->y) < 8.0f;
}

static void lift_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    Entity *mario = &game->entities[game->mario];
    Entity *partner = find_lift_partner(game->entities, self);

    bool mario_on_me = mario_on_lift(mario, self);

    if (mario_on_me) {
        self->vy = LIFT_FALL_SPEED;
        if (partner) partner->vy = -LIFT_FALL_SPEED;
    } else if (partner && mario_on_lift(mario, partner)) {
        self->vy = -LIFT_FALL_SPEED;
    } else {
        // Drift back toward base
        float base_y = self->anim_timer;
        float diff = base_y - self->y;
        if (fabsf(diff) > 2.0f)
            self->vy = (diff > 0) ? LIFT_ROPE_SPEED : -LIFT_ROPE_SPEED;
        else
            self->vy = 0;
    }

    self->y += self->vy * dt;

    // Carry Mario
    if (mario_on_me) {
        mario->y = self->y - mario->h;
        mario->vy = 0;
        mario->on_ground = true;
    }

    // Despawn if fallen way off screen
    if (self->y > game->level.height * TILE_SIZE + 200)
        entity_deactivate(self);
}

static void lift_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_LIFT);
    // Rope going up
    float cx = dx + self->w / 2;
    DrawLine((int)cx, 0, (int)cx, (int)dy, GRAY);
}

const EntityVtab lift_vtab = {
    .update = lift_update,
    .draw   = lift_draw,
};

void spawn_balance_lift(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    Entity *e = entity_alloc(entities);
    if (!e) return;
    e->type = ENT_BALANCE_LIFT;
    e->vtab = &lift_vtab;
    e->x = x;
    e->y = y;
    e->w = LIFT_W;
    e->h = LIFT_H;
    e->active = true;
    e->state_val = extra;
    e->anim_timer = y;
    e->self_moving = true;
}
