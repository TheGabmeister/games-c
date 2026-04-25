#include "mario.h"
#include "game.h"
#include "level.h"
#include "sprites.h"
#include "sounds.h"
#include "items.h"

typedef struct {
    bool left;
    bool right;
    bool jump;
    bool jump_held;
    bool run;
    bool fire;
} MarioInput;

static MarioInput read_input(void) {
    MarioInput in = {0};

    in.left      = IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A);
    in.right     = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
    in.jump      = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
    in.jump_held = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    in.run       = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    in.fire      = IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT);

    if (IsGamepadAvailable(0)) {
        float axis_x = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        if (axis_x < -GAMEPAD_DEADZONE) in.left = true;
        if (axis_x > GAMEPAD_DEADZONE)  in.right = true;

        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))  in.jump = true;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))     in.jump_held = true;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))    in.run = true;
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) in.fire = true;

        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP))     in.jump = true;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP))        in.jump_held = true;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT))      in.left = true;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))     in.right = true;
    }

    return in;
}

static void mario_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    if (dt <= 0) return;

    MarioInput in = read_input();
    bool underwater = (game->level.type == LEVEL_UNDERWATER);

    float max_speed = underwater ? UNDERWATER_WALK_SPEED :
                      (in.run ? MARIO_RUN_SPEED : MARIO_WALK_SPEED);

    // Horizontal movement
    if (in.right && !in.left) {
        self->facing = DIR_RIGHT;
        if (self->vx < 0) {
            self->vx += MARIO_SKID_DECEL * dt;
            if (self->vx > 0) self->vx = 0;
        } else {
            self->vx += MARIO_ACCEL * dt;
            if (self->vx > max_speed) self->vx = max_speed;
        }
    } else if (in.left && !in.right) {
        self->facing = DIR_LEFT;
        if (self->vx > 0) {
            self->vx -= MARIO_SKID_DECEL * dt;
            if (self->vx < 0) self->vx = 0;
        } else {
            self->vx -= MARIO_ACCEL * dt;
            if (self->vx < -max_speed) self->vx = -max_speed;
        }
    } else {
        if (self->vx > 0) {
            self->vx -= MARIO_DECEL * dt;
            if (self->vx < 0) self->vx = 0;
        } else if (self->vx < 0) {
            self->vx += MARIO_DECEL * dt;
            if (self->vx > 0) self->vx = 0;
        }
    }

    if (underwater) {
        // Swimming: press jump to get upward impulse
        if (in.jump) {
            self->vy = UNDERWATER_SWIM_VEL;
            sound_play(SND_JUMP);
        }
        self->vy += UNDERWATER_GRAVITY * dt;
        if (self->vy > UNDERWATER_MAX_FALL) self->vy = UNDERWATER_MAX_FALL;
        // Ceiling clamp
        if (self->y < 0) { self->y = 0; self->vy = 0; }
    } else {
        // Jump
        if (in.jump && self->on_ground) {
            self->vy = MARIO_JUMP_VEL;
            self->jumping = true;
            self->on_ground = false;
            sound_play(SND_JUMP);
        }

        // Variable-height jump
        if (self->jumping && in.jump_held && self->vy < 0) {
            self->vy += (GRAVITY + MARIO_JUMP_SUSTAIN) * dt;
        } else {
            self->vy += GRAVITY * dt;
            if (!in.jump_held) self->jumping = false;
        }

        if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;
    }

    // Camera left-edge constraint
    if (self->x < game->camera_x) {
        self->x = game->camera_x;
        self->vx = 0;
    }

    // Fireball throwing
    if (in.fire && self->power == MARIO_FIRE) {
        float fx = (self->facing == DIR_RIGHT) ? self->x + self->w : self->x - FIREBALL_SIZE;
        float fy = self->y + self->h / 2;
        spawn_fireball(game->entities, fx, fy, self->facing);
        sound_play(SND_FIREBALL);
    }

    // Invincibility timer
    if (self->invincible_timer > 0) {
        self->invincible_timer -= dt;
        if (self->invincible_timer < 0) self->invincible_timer = 0;
    }

    // Animation timer
    self->anim_timer += dt;
}

static SpriteID mario_sprite(Entity *self) {
    if (!self->on_ground) return SPR_MARIO_JUMP;
    if (fabsf(self->vx) > 20.0f)
        return SPR_MARIO_WALK1;
    return SPR_MARIO_STAND;
}

static void mario_draw(Entity *self, float camera_x) {
    float draw_x = self->x - camera_x;
    float draw_y = self->y;

    // Flashing when invincible (damage)
    if (self->invincible_timer > 0) {
        if ((int)(self->invincible_timer * 10) % 2 == 0) return;
    }

    // Star flashing
    Color tint = WHITE;
    if (self->star_active) {
        float t = self->star_timer;
        int phase = (int)(t * 8) % 4;
        Color star_colors[] = {RED, GREEN, BLUE, YELLOW};
        tint = star_colors[phase];
    }

    SpriteID sid = mario_sprite(self);
    Texture2D tex = sprites_get(sid);
    bool flip = (self->facing == DIR_LEFT);

    if (tex.id > 0) {
        Rectangle src = {0, 0, (float)tex.width, (float)tex.height};
        if (flip) src.width = -src.width;
        Rectangle dst = {draw_x, draw_y, self->w, self->h};
        DrawTexturePro(tex, src, dst, (Vector2){0, 0}, 0, tint);
    } else {
        Color color = COLOR_MARIO;
        if (self->power == MARIO_FIRE) color = WHITE;
        if (self->star_active) color = tint;

        DrawRectangle((int)draw_x, (int)draw_y, (int)self->w, (int)self->h, color);

        float eye_x = draw_x + (self->facing == DIR_RIGHT ? self->w * 0.65f : self->w * 0.15f);
        float eye_y = draw_y + self->h * 0.2f;
        DrawCircle((int)eye_x, (int)eye_y, 4, WHITE);
        float pupil_offset = (self->facing == DIR_RIGHT) ? 2.0f : -2.0f;
        DrawCircle((int)(eye_x + pupil_offset), (int)eye_y, 2, BLACK);

        // Big Mario: draw body extension
        if (self->power >= MARIO_BIG) {
            DrawRectangle((int)draw_x + 4, (int)(draw_y + 48), (int)self->w - 8, 48,
                          self->star_active ? tint : (Color){0, 0, 200, 255});
        }
    }
}

const EntityVtab mario_vtab = {
    .update = mario_update,
    .draw   = mario_draw,
};

void spawn_mario(Entity entities[MAX_ENTITIES], int *mario_idx, float x, float y) {
    Entity *m = entity_alloc(entities);
    if (!m) return;

    m->type = ENT_MARIO;
    m->vtab = &mario_vtab;
    m->x = x;
    m->y = y;
    m->w = MARIO_SMALL_W;
    m->h = MARIO_SMALL_H;
    m->facing = DIR_RIGHT;
    m->active = true;
    m->power = MARIO_SMALL;

    *mario_idx = (int)(m - entities);
}

bool mario_is_stomping(Entity *mario, Entity *enemy) {
    return mario->vy > 0 &&
           (mario->y + mario->h) > enemy->y &&
           (mario->y + mario->h) < enemy->y + enemy->h * 0.6f;
}

void mario_take_damage(Entity *mario, Game *game) {
    if (mario->invincible_timer > 0 || mario->star_active) return;

    if (mario->power > MARIO_SMALL) {
        mario->power = MARIO_SMALL;
        mario->y += (mario->h - MARIO_SMALL_H);
        mario->h = MARIO_SMALL_H;
        mario->invincible_timer = DAMAGE_INVINCIBLE_TIME;
        sound_play(SND_BUMP);
    } else {
        game->state = STATE_DYING;
        game->state_timer = 0;
        sound_play(SND_DEATH);
    }
}
