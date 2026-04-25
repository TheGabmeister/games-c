#include "player.h"
#include "input.h"

#include "raylib.h"
#include <string.h>

#define PLAYER_DEATH_SEC 0.6f

static bool is_resting(const Actor *actor) {
    return actor->t <= 0.0f && actor->tile_r == actor->dst_r && actor->tile_c == actor->dst_c;
}

static bool tile_is_climbable(TileID tile, bool exit_revealed) {
    return tile == TILE_LADDER || (tile == TILE_EXIT_LADDER && exit_revealed);
}

static bool tile_is_empty_for_dig(TileID tile) {
    return tile == TILE_EMPTY || tile == TILE_HOLE;
}

static bool player_has_support(const World *world, int r, int c) {
    return world_tile_is_support(world_tile_at(world, r + 1, c), world->all_gold_collected);
}

static PlayerState player_state_for_tile(const Player *player, const World *world) {
    TileID tile = world_tile_at(world, player->actor.tile_r, player->actor.tile_c);

    if (tile_is_climbable(tile, world->all_gold_collected)) {
        return PSTATE_CLIMB;
    }
    if (tile == TILE_ROPE) {
        return PSTATE_HANG;
    }
    if (!player_has_support(world, player->actor.tile_r, player->actor.tile_c)) {
        return PSTATE_FALL;
    }
    return PSTATE_WALK;
}

static float speed_for_state(PlayerState state) {
    switch (state) {
        case PSTATE_CLIMB: return CLIMB_SPEED;
        case PSTATE_HANG: return HANG_SPEED;
        case PSTATE_FALL: return FALL_SPEED;
        case PSTATE_WALK:
        default:
            return WALK_SPEED;
    }
}

static void start_step(Player *player, int dr, int dc, PlayerState state) {
    player->state = state;
    player->actor.dst_r = player->actor.tile_r + dr;
    player->actor.dst_c = player->actor.tile_c + dc;
    player->actor.t = 0.001f;

    if (dc < 0) {
        player->actor.facing = DIR_LEFT;
    } else if (dc > 0) {
        player->actor.facing = DIR_RIGHT;
    }
}

static bool can_enter(const World *world, int r, int c) {
    return world_in_bounds(r, c) && world_tile_is_passable(world_tile_at(world, r, c), world->all_gold_collected);
}

static bool can_step_side_from_ladder(const World *world, int r, int c) {
    TileID tile = world_tile_at(world, r, c);
    return can_enter(world, r, c) &&
           (tile_is_climbable(tile, world->all_gold_collected) ||
            tile == TILE_ROPE ||
            player_has_support(world, r, c));
}

static bool try_start_fall(Player *player, const World *world) {
    int r = player->actor.tile_r;
    int c = player->actor.tile_c;

    if (can_enter(world, r + 1, c)) {
        start_step(player, 1, 0, PSTATE_FALL);
        return true;
    }
    return false;
}

static void collect_gold_if_present(Player *player, World *world, PlayerTickResult *result) {
    int r = player->actor.tile_r;
    int c = player->actor.tile_c;

    if (world_tile_at(world, r, c) == TILE_GOLD) {
        world->tiles[r][c] = TILE_EMPTY;
        if (world->gold_remaining > 0) {
            world->gold_remaining--;
        }
        if (world->gold_remaining <= 0) {
            world->all_gold_collected = true;
        }
        result->collected_gold = true;
    }
}

static bool try_dig(Player *player, World *world, int dc, PlayerTickResult *result) {
    int r = player->actor.tile_r;
    int c = player->actor.tile_c;
    int above_r = r;
    int target_r = r + 1;
    int target_c = c + dc;

    if (!world_in_bounds(target_r, target_c) || !world_in_bounds(above_r, target_c)) {
        return false;
    }
    if (player_state_for_tile(player, world) != PSTATE_WALK || !player_has_support(world, r, c)) {
        return false;
    }
    if (world_tile_at(world, target_r, target_c) != TILE_BRICK) {
        return false;
    }
    if (!tile_is_empty_for_dig(world_tile_at(world, above_r, target_c))) {
        return false;
    }

    world->tiles[target_r][target_c] = TILE_HOLE;
    world->hole_timer[target_r][target_c] = DIG_REFILL_SEC;
    player->state = PSTATE_DIG;
    player->dig_lock_timer = DIG_LOCK_SEC;
    player->actor.dst_r = player->actor.tile_r;
    player->actor.dst_c = player->actor.tile_c;
    player->actor.t = 0.0f;
    player->actor.facing = dc < 0 ? DIR_LEFT : DIR_RIGHT;
    result->dug_brick = true;
    return true;
}

static void decide_next_step(Player *player, World *world, PlayerTickResult *result) {
    PlayerState context = player_state_for_tile(player, world);
    player->state = context;

    if (context == PSTATE_FALL) {
        try_start_fall(player, world);
        return;
    }

    if (input_dig_left()) {
        if (try_dig(player, world, -1, result)) {
            return;
        }
    }
    if (input_dig_right()) {
        if (try_dig(player, world, 1, result)) {
            return;
        }
    }

    bool left = input_left();
    bool right = input_right();
    bool up = input_up();
    bool down = input_down();
    int r = player->actor.tile_r;
    int c = player->actor.tile_c;

    if (context == PSTATE_CLIMB) {
        if (up && can_enter(world, r - 1, c)) {
            start_step(player, -1, 0, PSTATE_CLIMB);
            return;
        }
        if (down && can_enter(world, r + 1, c)) {
            start_step(player, 1, 0, PSTATE_CLIMB);
            return;
        }
    } else if (context == PSTATE_HANG) {
        if (down) {
            if (try_start_fall(player, world)) {
                return;
            }
        }
    }

    if (left && !right &&
        (context != PSTATE_CLIMB ? can_enter(world, r, c - 1) : can_step_side_from_ladder(world, r, c - 1))) {
        start_step(player, 0, -1, context == PSTATE_HANG ? PSTATE_HANG : PSTATE_WALK);
        return;
    }
    if (right && !left &&
        (context != PSTATE_CLIMB ? can_enter(world, r, c + 1) : can_step_side_from_ladder(world, r, c + 1))) {
        start_step(player, 0, 1, context == PSTATE_HANG ? PSTATE_HANG : PSTATE_WALK);
        return;
    }
}

void player_spawn(Player *player, const World *world) {
    memset(player, 0, sizeof(*player));
    player->actor.tile_r = world->player_spawn_r;
    player->actor.tile_c = world->player_spawn_c;
    player->actor.dst_r = player->actor.tile_r;
    player->actor.dst_c = player->actor.tile_c;
    player->actor.facing = DIR_RIGHT;
    player->state = PSTATE_WALK;
}

void player_start_death(Player *player) {
    if (player->state == PSTATE_DEAD) {
        return;
    }

    player->state = PSTATE_DEAD;
    player->death_timer = PLAYER_DEATH_SEC;
    player->dig_lock_timer = 0.0f;
    player->actor.dst_r = player->actor.tile_r;
    player->actor.dst_c = player->actor.tile_c;
    player->actor.t = 0.0f;
}

PlayerTickResult player_update(Player *player, World *world, float dt) {
    PlayerTickResult result = { 0 };

    if (player->state == PSTATE_DEAD) {
        player->death_timer -= dt;
        if (player->death_timer <= 0.0f) {
            result.died = true;
        }
        return result;
    }

    if (player->state == PSTATE_DIG) {
        player->dig_lock_timer -= dt;
        if (player->dig_lock_timer > 0.0f) {
            return result;
        }
        player->dig_lock_timer = 0.0f;
    }

    if (!is_resting(&player->actor)) {
        player->actor.t += dt * speed_for_state(player->state);

        if (player->actor.t >= 1.0f) {
            player->actor.tile_r = player->actor.dst_r;
            player->actor.tile_c = player->actor.dst_c;
            player->actor.t = 0.0f;
            result.committed_new_tile = true;

            collect_gold_if_present(player, world, &result);

            if (player->actor.tile_r == 0 && world->all_gold_collected) {
                result.reached_exit = true;
            }
        } else {
            return result;
        }
    } else {
        collect_gold_if_present(player, world, &result);
    }

    decide_next_step(player, world, &result);
    return result;
}

Vector2 player_pixel_position(const Player *player) {
    float t = player->actor.t;
    float r = (float)player->actor.tile_r + ((float)player->actor.dst_r - (float)player->actor.tile_r) * t;
    float c = (float)player->actor.tile_c + ((float)player->actor.dst_c - (float)player->actor.tile_c) * t;

    return (Vector2){
        (float)PLAY_OFFSET_X + c * (float)TILE_SIZE + (float)TILE_SIZE * 0.5f,
        (float)PLAY_OFFSET_Y + r * (float)TILE_SIZE + (float)TILE_SIZE * 0.5f
    };
}

const char *player_state_name(PlayerState state) {
    switch (state) {
        case PSTATE_WALK: return "WALK";
        case PSTATE_CLIMB: return "CLIMB";
        case PSTATE_HANG: return "HANG";
        case PSTATE_FALL: return "FALL";
        case PSTATE_DIG: return "DIG";
        case PSTATE_DEAD: return "DEAD";
        default: return "UNKNOWN";
    }
}
