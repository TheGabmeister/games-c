#include "collisions.h"

#include "asteroids.h"
#include "particles.h"

#define MAX_COLLIDERS 128
#define MAX_EVENTS    256

typedef struct Collider {
    Vector2 position;
    float radius;
    uint32_t layer;
    uint32_t mask;
    CollisionTag tag;
    int poolIndex;
} Collider;

typedef struct CollisionEvent {
    CollisionTag tagA;
    CollisionTag tagB;
    int indexA;
    int indexB;
} CollisionEvent;

static Collider s_colliders[MAX_COLLIDERS];
static int s_colliderCount;

static CollisionEvent s_events[MAX_EVENTS];
static int s_eventCount;

// --- Gather: build collider list from entity pools ---

static void CollisionsGather(const GameContext *ctx)
{
    s_colliderCount = 0;

    // Ship (only when alive and vulnerable)
    if (ctx->ship.alive && ctx->ship.invulnTimer <= 0) {
        s_colliders[s_colliderCount++] = (Collider){
            .position  = ctx->ship.position,
            .radius    = SHIP_SIZE * 0.5f,
            .layer     = COLLISION_LAYER_PLAYER,
            .mask      = COLLISION_LAYER_ASTEROID,
            .tag       = COLLISION_TAG_SHIP,
            .poolIndex = 0,
        };
    }

    // Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!ctx->bullets[i].active) continue;
        s_colliders[s_colliderCount++] = (Collider){
            .position  = ctx->bullets[i].position,
            .radius    = BULLET_RADIUS,
            .layer     = COLLISION_LAYER_BULLET,
            .mask      = COLLISION_LAYER_ASTEROID,
            .tag       = COLLISION_TAG_BULLET,
            .poolIndex = i,
        };
    }

    // Asteroids
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!ctx->asteroids[i].active) continue;
        s_colliders[s_colliderCount++] = (Collider){
            .position  = ctx->asteroids[i].position,
            .radius    = ctx->asteroids[i].radius,
            .layer     = COLLISION_LAYER_ASTEROID,
            .mask      = COLLISION_LAYER_PLAYER | COLLISION_LAYER_BULLET,
            .tag       = COLLISION_TAG_ASTEROID,
            .poolIndex = i,
        };
    }
}

// --- Detect: pairwise checks with layer filtering ---

static void CollisionsDetect(void)
{
    s_eventCount = 0;

    for (int i = 0; i < s_colliderCount; i++) {
        for (int j = i + 1; j < s_colliderCount; j++) {
            Collider *a = &s_colliders[i];
            Collider *b = &s_colliders[j];

            if (!(a->layer & b->mask) || !(b->layer & a->mask)) continue;

            if (!CheckCollisionCircles(a->position, a->radius,
                                       b->position, b->radius)) continue;

            if (s_eventCount >= MAX_EVENTS) return;

            if (a->tag <= b->tag) {
                s_events[s_eventCount++] = (CollisionEvent){
                    .tagA = a->tag,  .tagB = b->tag,
                    .indexA = a->poolIndex, .indexB = b->poolIndex,
                };
            } else {
                s_events[s_eventCount++] = (CollisionEvent){
                    .tagA = b->tag,  .tagB = a->tag,
                    .indexA = b->poolIndex, .indexB = a->poolIndex,
                };
            }
        }
    }
}

// --- Response handlers ---

static void OnBulletHitAsteroid(GameContext *ctx, int bulletIdx, int asteroidIdx)
{
    if (!ctx->bullets[bulletIdx].active) return;
    if (!ctx->asteroids[asteroidIdx].active) return;

    ctx->bullets[bulletIdx].active = false;

    switch (ctx->asteroids[asteroidIdx].size) {
        case ASTEROID_LARGE:  ctx->game.score += SCORE_LARGE;  break;
        case ASTEROID_MEDIUM: ctx->game.score += SCORE_MEDIUM; break;
        case ASTEROID_SMALL:  ctx->game.score += SCORE_SMALL;  break;
    }

    AsteroidsSplit(ctx, asteroidIdx);
}

static void OnShipHitAsteroid(GameContext *ctx, int asteroidIdx)
{
    (void)asteroidIdx;
    if (!ctx->ship.alive) return;

    ctx->ship.alive = false;
    ctx->ship.respawnTimer = 2.0f;
    ctx->game.lives--;
    ParticlesSpawn(ctx, ctx->ship.position, 35, 150.0f, 1.0f, 3.0f, GREEN);
    ParticlesSpawn(ctx, ctx->ship.position, 10, 80.0f, 2.0f, 4.0f, LIME);
    ctx->game.screenShakeTimer = SCREEN_SHAKE_DURATION;
    ctx->game.screenShakeMagnitude = SCREEN_SHAKE_MAGNITUDE * 1.5f;
}

// --- Resolve: dispatch events to handlers ---

static void CollisionsResolve(GameContext *ctx)
{
    for (int i = 0; i < s_eventCount; i++) {
        CollisionEvent *e = &s_events[i];

        if (e->tagA == COLLISION_TAG_SHIP && e->tagB == COLLISION_TAG_ASTEROID) {
            OnShipHitAsteroid(ctx, e->indexB);
        }
        else if (e->tagA == COLLISION_TAG_BULLET && e->tagB == COLLISION_TAG_ASTEROID) {
            OnBulletHitAsteroid(ctx, e->indexA, e->indexB);
        }
    }
}

// --- Public API (unchanged) ---

void CollisionsUpdate(GameContext *ctx)
{
    CollisionsGather(ctx);
    CollisionsDetect();
    CollisionsResolve(ctx);
}
