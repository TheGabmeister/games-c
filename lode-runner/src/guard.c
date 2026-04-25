#include "guard.h"

#include <stdlib.h>
#include <string.h>

static GuardState state_for_tile(const Guard *guard, const World *world) {
    TileID tile = world_tile_at(world, guard->actor.tile_r, guard->actor.tile_c);

    if (world_tile_is_climbable(tile, world->all_gold_collected)) {
        return GSTATE_CLIMB;
    }
    if (tile == TILE_ROPE) {
        return GSTATE_HANG;
    }
    if (!world_has_support(world, guard->actor.tile_r, guard->actor.tile_c)) {
        return GSTATE_FALL;
    }
    return GSTATE_WALK;
}

static float speed_for_state(GuardState state) {
    switch (state) {
        case GSTATE_CLIMB: return CLIMB_SPEED * GUARD_SPEED_MUL;
        case GSTATE_HANG: return HANG_SPEED * GUARD_SPEED_MUL;
        case GSTATE_FALL: return FALL_SPEED;
        case GSTATE_WALK:
        default:
            return WALK_SPEED * GUARD_SPEED_MUL;
    }
}

static void start_step(Guard *guard, int dr, int dc, GuardState state) {
    guard->state = state;
    guard->actor.dst_r = guard->actor.tile_r + dr;
    guard->actor.dst_c = guard->actor.tile_c + dc;
    guard->actor.t = 0.001f;

    if (dc < 0) {
        guard->actor.facing = DIR_LEFT;
    } else if (dc > 0) {
        guard->actor.facing = DIR_RIGHT;
    }
}

static void reset_guard_to_spawn(Guard *guard) {
    guard->actor.tile_r = guard->spawn_r;
    guard->actor.tile_c = guard->spawn_c;
    guard->actor.dst_r = guard->spawn_r;
    guard->actor.dst_c = guard->spawn_c;
    guard->actor.t = 0.0f;
    guard->actor.facing = DIR_LEFT;
    guard->prev_r = guard->spawn_r;
    guard->prev_c = guard->spawn_c;
    guard->state = GSTATE_WALK;
    guard->hole_timer = 0.0f;
    guard->respawn_timer = 0.0f;
    guard->active = true;
}

static uint32_t hash_guard(uint32_t seed, int r, int c) {
    uint32_t h = seed ^ 0x9E3779B9u;
    h ^= (uint32_t)(r + 101) * 0x85EBCA6Bu;
    h ^= (uint32_t)(c + 211) * 0xC2B2AE35u;
    h ^= h >> 16;
    h *= 0x7FEB352Du;
    h ^= h >> 15;
    return h;
}

static bool tile_occupied_by_guard(const Guard guards[MAX_GUARDS], int guard_count, int self_index, int r, int c) {
    for (int i = 0; i < guard_count; i++) {
        if (i == self_index || !guards[i].active || guards[i].state == GSTATE_RESPAWN) {
            continue;
        }

        if ((guards[i].actor.tile_r == r && guards[i].actor.tile_c == c) ||
            (guards[i].actor.dst_r == r && guards[i].actor.dst_c == c)) {
            return true;
        }
    }
    return false;
}

static bool can_move_dir(const Guard *guard, const World *world, GuardState context, PursuitDir dir, int *dr, int *dc) {
    int r = guard->actor.tile_r;
    int c = guard->actor.tile_c;

    switch (dir) {
        case PURSUE_LEFT: *dr = 0; *dc = -1; break;
        case PURSUE_RIGHT: *dr = 0; *dc = 1; break;
        case PURSUE_UP: *dr = -1; *dc = 0; break;
        case PURSUE_DOWN: *dr = 1; *dc = 0; break;
        case PURSUE_NONE:
        default:
            return false;
    }

    if (context == GSTATE_FALL) {
        return dir == PURSUE_DOWN && world_can_enter(world, r + 1, c);
    }
    if (context == GSTATE_CLIMB && (*dc == 0) && (*dr == -1 || *dr == 1)) {
        return world_can_enter(world, r + *dr, c);
    }
    if (context == GSTATE_HANG && dir == PURSUE_DOWN) {
        return world_can_enter(world, r + 1, c);
    }
    if (*dr == 0 && (*dc == -1 || *dc == 1)) {
        if (context == GSTATE_CLIMB) {
            return world_can_step_side(world, r, c + *dc);
        }
        return world_can_enter(world, r, c + *dc);
    }

    return false;
}

static int score_move(const Guard *guard, int guard_index, const Guard guards[MAX_GUARDS], int guard_count, const Player *player, PursuitDir pursuit, PursuitDir dir, int dr, int dc) {
    int r = guard->actor.tile_r;
    int c = guard->actor.tile_c;
    int nr = r + dr;
    int nc = c + dc;
    int score = 0;

    if (dir == pursuit) {
        score += 100;
    }
    if (abs(nr - player->actor.tile_r) < abs(r - player->actor.tile_r)) {
        score += 10;
    }
    if (abs(nc - player->actor.tile_c) < abs(c - player->actor.tile_c)) {
        score += 5;
    }
    if (tile_occupied_by_guard(guards, guard_count, guard_index, nr, nc)) {
        score -= 15;
    }
    if (dir == PURSUE_DOWN && abs(nr - player->actor.tile_r) <= 4) {
        score += 20;
    }

    score += (int)(hash_guard(guard->seed, r, c) % 4u) - 2;
    return score;
}

static bool gold_drop_tile_ok(const World *world, int r, int c) {
    if (!world_in_bounds(r, c)) {
        return false;
    }

    return world_tile_at(world, r, c) == TILE_EMPTY;
}

static bool guard_drop_gold(Guard *guard, World *world, int preferred_r, int preferred_c) {
    if (!guard->carries_gold) {
        return false;
    }

    const int drs[5] = { 0, 0, 0, -1, 1 };
    const int dcs[5] = { 0, -1, 1, 0, 0 };
    for (int i = 0; i < 5; i++) {
        int r = preferred_r + drs[i];
        int c = preferred_c + dcs[i];
        if (gold_drop_tile_ok(world, r, c)) {
            world->tiles[r][c] = TILE_GOLD;
            guard->carries_gold = false;
            return true;
        }
    }

    return false;
}

static bool should_pick_up_gold(const Guard *guard) {
    return (hash_guard(guard->seed, guard->actor.tile_r, guard->actor.tile_c) % 100u) < 50u;
}

static bool should_random_drop(float dt) {
    int threshold = (int)(GUARD_DROP_PROB * dt * 1000000.0f);
    return threshold > 0 && GetRandomValue(0, 999999) < threshold;
}

static void handle_gold_on_tile(Guard *guard, World *world, GuardTickResult *result, float dt) {
    if (guard->carries_gold) {
        if (should_random_drop(dt) && guard_drop_gold(guard, world, guard->actor.tile_r, guard->actor.tile_c)) {
            result->dropped_gold = true;
        }
        return;
    }

    if (world_tile_at(world, guard->actor.tile_r, guard->actor.tile_c) == TILE_GOLD && should_pick_up_gold(guard)) {
        world->tiles[guard->actor.tile_r][guard->actor.tile_c] = TILE_EMPTY;
        guard->carries_gold = true;
        result->picked_up_gold = true;
    }
}

static void decide_next_step(Guard *guard, int guard_index, const Guard guards[MAX_GUARDS], const World *world, const Player *player, const PursuitDir pursuit[GRID_ROWS][GRID_COLS]) {
    GuardState context = state_for_tile(guard, world);
    PursuitDir best_dir = PURSUE_NONE;
    int best_score = -100000;
    int best_dr = 0;
    int best_dc = 0;
    int guard_count = world->guard_count;
    PursuitDir preferred = pursuit[guard->actor.tile_r][guard->actor.tile_c];
    const PursuitDir dirs[4] = { PURSUE_LEFT, PURSUE_RIGHT, PURSUE_UP, PURSUE_DOWN };

    guard->state = context;

    if (context == GSTATE_FALL) {
        if (world_can_enter(world, guard->actor.tile_r + 1, guard->actor.tile_c)) {
            start_step(guard, 1, 0, GSTATE_FALL);
        }
        return;
    }

    for (int i = 0; i < 4; i++) {
        int dr = 0;
        int dc = 0;
        PursuitDir dir = dirs[i];
        if (!can_move_dir(guard, world, context, dir, &dr, &dc)) {
            continue;
        }

        int score = score_move(guard, guard_index, guards, guard_count, player, preferred, dir, dr, dc);
        if (score > best_score) {
            best_score = score;
            best_dir = dir;
            best_dr = dr;
            best_dc = dc;
        }
    }

    if (best_dir != PURSUE_NONE) {
        GuardState step_state = GSTATE_WALK;
        if (best_dir == PURSUE_UP || (best_dir == PURSUE_DOWN && context == GSTATE_CLIMB)) {
            step_state = GSTATE_CLIMB;
        } else if (best_dir == PURSUE_DOWN && context == GSTATE_HANG) {
            step_state = GSTATE_FALL;
        } else if (context == GSTATE_HANG) {
            step_state = GSTATE_HANG;
        } else if (best_dir == PURSUE_DOWN && !world_has_support(world, guard->actor.tile_r, guard->actor.tile_c)) {
            step_state = GSTATE_FALL;
        }
        start_step(guard, best_dr, best_dc, step_state);
    }
}

void guards_spawn_from_world(Guard guards[MAX_GUARDS], const World *world) {
    memset(guards, 0, sizeof(Guard) * MAX_GUARDS);

    for (int i = 0; i < world->guard_count && i < MAX_GUARDS; i++) {
        guards[i].spawn_r = world->guard_spawn_r[i];
        guards[i].spawn_c = world->guard_spawn_c[i];
        guards[i].seed = 0xA511E9B3u + (uint32_t)i * 0x45D9F3Bu;
        reset_guard_to_spawn(&guards[i]);
        guards[i].carries_gold = false;
    }
}

bool guard_kill_in_refill(Guard *guard, World *world) {
    if (!guard->active || guard->state == GSTATE_RESPAWN) {
        return false;
    }

    bool dropped_gold = guard_drop_gold(guard, world, guard->prev_r, guard->prev_c);
    guard->state = GSTATE_RESPAWN;
    guard->respawn_timer = GUARD_RESPAWN_SEC;
    guard->hole_timer = 0.0f;
    guard->actor.t = 0.0f;
    guard->actor.dst_r = guard->actor.tile_r;
    guard->actor.dst_c = guard->actor.tile_c;
    return dropped_gold;
}

GuardTickResult guard_update(Guard *guard, int guard_index, const Guard guards[MAX_GUARDS], World *world, const Player *player, const PursuitDir pursuit[GRID_ROWS][GRID_COLS], float dt) {
    GuardTickResult result = { 0 };

    if (!guard->active) {
        return result;
    }

    if (guard->state == GSTATE_RESPAWN) {
        guard->respawn_timer -= dt;
        if (guard->respawn_timer <= 0.0f) {
            reset_guard_to_spawn(guard);
            result.respawned = true;
        }
        return result;
    }

    if (guard->state == GSTATE_TRAPPED) {
        guard->hole_timer -= dt;
        handle_gold_on_tile(guard, world, &result, dt);
        if (guard->hole_timer <= 0.0f && world_can_enter(world, guard->actor.tile_r - 1, guard->actor.tile_c)) {
            start_step(guard, -1, 0, GSTATE_CLIMB);
        }
        return result;
    }

    if (!actor_is_resting(&guard->actor)) {
        guard->actor.t += dt * speed_for_state(guard->state);
        if (guard->actor.t >= 1.0f) {
            guard->prev_r = guard->actor.tile_r;
            guard->prev_c = guard->actor.tile_c;
            guard->actor.tile_r = guard->actor.dst_r;
            guard->actor.tile_c = guard->actor.dst_c;
            guard->actor.t = 0.0f;
            result.committed_new_tile = true;

            if (world_tile_at(world, guard->actor.tile_r, guard->actor.tile_c) == TILE_HOLE) {
                if (guard_drop_gold(guard, world, guard->prev_r, guard->prev_c)) {
                    result.dropped_gold = true;
                }
                guard->state = GSTATE_TRAPPED;
                guard->hole_timer = GUARD_HOLE_SEC;
                result.fell_in_hole = true;
                return result;
            }
        } else {
            return result;
        }
    } else if (world_tile_at(world, guard->actor.tile_r, guard->actor.tile_c) == TILE_HOLE) {
        if (guard_drop_gold(guard, world, guard->prev_r, guard->prev_c)) {
            result.dropped_gold = true;
        }
        guard->state = GSTATE_TRAPPED;
        guard->hole_timer = GUARD_HOLE_SEC;
        result.fell_in_hole = true;
        return result;
    }

    handle_gold_on_tile(guard, world, &result, dt);
    decide_next_step(guard, guard_index, guards, world, player, pursuit);
    return result;
}

bool guard_can_catch_player(const Guard *guard, const Player *player) {
    if (!guard->active || guard->state == GSTATE_RESPAWN || guard->state == GSTATE_TRAPPED || player->state == PSTATE_DEAD) {
        return false;
    }

    return guard->actor.tile_r == player->actor.tile_r && guard->actor.tile_c == player->actor.tile_c;
}

const char *guard_state_name(GuardState state) {
    switch (state) {
        case GSTATE_WALK: return "WALK";
        case GSTATE_CLIMB: return "CLIMB";
        case GSTATE_HANG: return "HANG";
        case GSTATE_FALL: return "FALL";
        case GSTATE_TRAPPED: return "TRAPPED";
        case GSTATE_RESPAWN: return "RESPAWN";
        default: return "UNKNOWN";
    }
}
