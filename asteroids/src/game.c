#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <string.h>

//------------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------------
#define SCREEN_WIDTH        800
#define SCREEN_HEIGHT       450

#define MAX_BULLETS         30
#define MAX_ASTEROIDS       64
#define MAX_PARTICLES       256
#define MAX_STARS           100
#define MAX_ASTEROID_VERTS  12

#define SHIP_SIZE           20.0f
#define SHIP_ROTATION_SPEED 270.0f
#define SHIP_THRUST_ACCEL   300.0f
#define SHIP_MAX_SPEED      400.0f
#define SHIP_DRAG           0.98f
#define SHIP_INVULN_TIME    3.0f
#define SHIP_BLINK_RATE     0.1f

#define BULLET_SPEED        500.0f
#define BULLET_LIFETIME     1.5f
#define BULLET_RADIUS       2.0f
#define FIRE_COOLDOWN       0.15f

#define ASTEROID_SPEED_MIN  30.0f
#define ASTEROID_SPEED_MAX  80.0f
#define ASTEROID_LARGE_R    40.0f
#define ASTEROID_MEDIUM_R   20.0f
#define ASTEROID_SMALL_R    10.0f
#define ASTEROID_VERT_MIN   8
#define ASTEROID_VERT_MAX   12
#define ASTEROID_JAGGEDNESS 0.4f

#define PARTICLE_LIFETIME_MIN 0.3f
#define PARTICLE_LIFETIME_MAX 1.2f

#define SCORE_LARGE         20
#define SCORE_MEDIUM        50
#define SCORE_SMALL         100
#define EXTRA_LIFE_SCORE    10000

#define INITIAL_LIVES       3
#define WAVE_DELAY          2.0f
#define SCREEN_SHAKE_DURATION 0.3f
#define SCREEN_SHAKE_MAGNITUDE 6.0f

#define STAR_LAYERS         3

//------------------------------------------------------------------------------------
// Types and Structures
//------------------------------------------------------------------------------------
typedef enum ActionType {
    NO_ACTION = 0,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_FIRE,
    MAX_ACTION
} ActionType;

typedef struct ActionInput {
    int key;
    int button;
} ActionInput;

typedef enum GameState {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_GAMEOVER,
} GameState;

typedef enum AsteroidSize {
    ASTEROID_LARGE = 0,
    ASTEROID_MEDIUM,
    ASTEROID_SMALL,
} AsteroidSize;

typedef struct Ship {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float invulnTimer;
    bool alive;
    bool thrusting;
    float fireCooldown;
    float respawnTimer;
} Ship;

typedef struct Bullet {
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    bool active;
} Bullet;

typedef struct Asteroid {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float rotationSpeed;
    float radius;
    AsteroidSize size;
    int vertCount;
    float vertDistances[MAX_ASTEROID_VERTS];
    bool active;
} Asteroid;

typedef struct Particle {
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    float maxLifetime;
    Color color;
    float size;
    bool active;
} Particle;

typedef struct Star {
    Vector2 position;
    float brightness;
    float size;
    int layer;
} Star;

typedef struct Game {
    GameState state;
    int score;
    int highScore;
    int lives;
    int wave;
    float waveTimer;
    bool waveCleared;
    float screenShakeTimer;
    float screenShakeMagnitude;
    int nextExtraLife;
    float gameOverTimer;
} Game;

//------------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------------
static int gamepadIndex = 0;
static ActionInput actionInputs[MAX_ACTION] = { 0 };

static Game game = { 0 };
static Ship ship = { 0 };
static Bullet bullets[MAX_BULLETS] = { 0 };
static Asteroid asteroids[MAX_ASTEROIDS] = { 0 };
static Particle particles[MAX_PARTICLES] = { 0 };
static Star stars[MAX_STARS] = { 0 };

//------------------------------------------------------------------------------------
// Action Input Functions
//------------------------------------------------------------------------------------
static bool IsActionPressed(int action)
{
    bool result = false;
    if (action < MAX_ACTION)
        result = (IsKeyPressed(actionInputs[action].key) ||
                  IsGamepadButtonPressed(gamepadIndex, actionInputs[action].button));
    return result;
}

static bool IsActionReleased(int action)
{
    bool result = false;
    if (action < MAX_ACTION)
        result = (IsKeyReleased(actionInputs[action].key) ||
                  IsGamepadButtonReleased(gamepadIndex, actionInputs[action].button));
    return result;
}

static bool IsActionDown(int action)
{
    bool result = false;
    if (action < MAX_ACTION)
        result = (IsKeyDown(actionInputs[action].key) ||
                  IsGamepadButtonDown(gamepadIndex, actionInputs[action].button));
    return result;
}

static void SetActionsDefault(void)
{
    actionInputs[ACTION_UP].key = KEY_W;
    actionInputs[ACTION_DOWN].key = KEY_S;
    actionInputs[ACTION_LEFT].key = KEY_A;
    actionInputs[ACTION_RIGHT].key = KEY_D;
    actionInputs[ACTION_FIRE].key = KEY_SPACE;

    actionInputs[ACTION_UP].button = GAMEPAD_BUTTON_LEFT_FACE_UP;
    actionInputs[ACTION_DOWN].button = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
    actionInputs[ACTION_LEFT].button = GAMEPAD_BUTTON_LEFT_FACE_LEFT;
    actionInputs[ACTION_RIGHT].button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
    actionInputs[ACTION_FIRE].button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
}

static void SetActionsCursor(void)
{
    actionInputs[ACTION_UP].key = KEY_UP;
    actionInputs[ACTION_DOWN].key = KEY_DOWN;
    actionInputs[ACTION_LEFT].key = KEY_LEFT;
    actionInputs[ACTION_RIGHT].key = KEY_RIGHT;
    actionInputs[ACTION_FIRE].key = KEY_SPACE;

    actionInputs[ACTION_UP].button = GAMEPAD_BUTTON_RIGHT_FACE_UP;
    actionInputs[ACTION_DOWN].button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
    actionInputs[ACTION_LEFT].button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT;
    actionInputs[ACTION_RIGHT].button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT;
    actionInputs[ACTION_FIRE].button = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
}

//------------------------------------------------------------------------------------
// Utility Functions
//------------------------------------------------------------------------------------
static Vector2 WrapPosition(Vector2 pos)
{
    float margin = ASTEROID_LARGE_R;
    if (pos.x < -margin) pos.x += SCREEN_WIDTH + 2 * margin;
    if (pos.x > SCREEN_WIDTH + margin) pos.x -= SCREEN_WIDTH + 2 * margin;
    if (pos.y < -margin) pos.y += SCREEN_HEIGHT + 2 * margin;
    if (pos.y > SCREEN_HEIGHT + margin) pos.y -= SCREEN_HEIGHT + 2 * margin;
    return pos;
}

static Vector2 RotateOffset(Vector2 offset, float angleDeg)
{
    float rad = angleDeg * DEG2RAD;
    float c = cosf(rad);
    float s = sinf(rad);
    return (Vector2){ offset.x * c - offset.y * s, offset.x * s + offset.y * c };
}

static Vector2 ShipVertex(int index)
{
    // 0 = nose, 1 = left wing, 2 = right wing
    Vector2 offsets[3] = {
        { 0.0f, -SHIP_SIZE },
        { -SHIP_SIZE * 0.6f, SHIP_SIZE * 0.7f },
        { SHIP_SIZE * 0.6f, SHIP_SIZE * 0.7f },
    };
    Vector2 rotated = RotateOffset(offsets[index], ship.rotation);
    return (Vector2){ ship.position.x + rotated.x, ship.position.y + rotated.y };
}

//------------------------------------------------------------------------------------
// Neon Drawing Helpers
//------------------------------------------------------------------------------------
static void DrawNeonLine(Vector2 start, Vector2 end, Color color)
{
    DrawLineEx(start, end, 3.0f, ColorAlpha(color, 0.3f));
    DrawLineEx(start, end, 1.0f, color);
}

static void DrawNeonCircle(Vector2 center, float radius, Color color)
{
    DrawCircleLinesV(center, radius + 1.0f, ColorAlpha(color, 0.3f));
    DrawCircleLinesV(center, radius, color);
}

static void DrawNeonTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    DrawNeonLine(v1, v2, color);
    DrawNeonLine(v2, v3, color);
    DrawNeonLine(v3, v1, color);
}

static void DrawNeonLineStrip(const Vector2 *points, int count, Color color)
{
    for (int i = 0; i < count - 1; i++) {
        DrawNeonLine(points[i], points[i + 1], color);
    }
}

//------------------------------------------------------------------------------------
// Star Field
//------------------------------------------------------------------------------------
static void InitStars(void)
{
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].position = (Vector2){
            (float)GetRandomValue(0, SCREEN_WIDTH),
            (float)GetRandomValue(0, SCREEN_HEIGHT)
        };
        stars[i].layer = GetRandomValue(0, STAR_LAYERS - 1);
        stars[i].brightness = 0.2f + 0.3f * (float)stars[i].layer;
        stars[i].size = 1.0f + (float)stars[i].layer * 0.5f;
    }
}

static void DrawStarfield(void)
{
    for (int i = 0; i < MAX_STARS; i++) {
        float ox = -ship.velocity.x * 0.001f * (float)(stars[i].layer + 1);
        float oy = -ship.velocity.y * 0.001f * (float)(stars[i].layer + 1);
        float sx = stars[i].position.x + ox;
        float sy = stars[i].position.y + oy;
        // Wrap star drawing position
        if (sx < 0) sx += SCREEN_WIDTH;
        if (sx > SCREEN_WIDTH) sx -= SCREEN_WIDTH;
        if (sy < 0) sy += SCREEN_HEIGHT;
        if (sy > SCREEN_HEIGHT) sy -= SCREEN_HEIGHT;
        DrawCircleV((Vector2){ sx, sy }, stars[i].size,
                     ColorAlpha(WHITE, stars[i].brightness));
    }
}

//------------------------------------------------------------------------------------
// Particle System
//------------------------------------------------------------------------------------
static void SpawnParticles(Vector2 pos, int count, float speed,
                           float sizeMin, float sizeMax, Color color)
{
    for (int i = 0; i < MAX_PARTICLES && count > 0; i++) {
        if (particles[i].active) continue;
        float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
        float spd = speed * (0.3f + 0.7f * (float)GetRandomValue(0, 100) / 100.0f);
        particles[i].position = pos;
        particles[i].velocity = (Vector2){ cosf(angle) * spd, sinf(angle) * spd };
        particles[i].maxLifetime = PARTICLE_LIFETIME_MIN +
            (PARTICLE_LIFETIME_MAX - PARTICLE_LIFETIME_MIN) * (float)GetRandomValue(0, 100) / 100.0f;
        particles[i].lifetime = particles[i].maxLifetime;
        particles[i].size = sizeMin + (sizeMax - sizeMin) * (float)GetRandomValue(0, 100) / 100.0f;
        particles[i].color = color;
        particles[i].active = true;
        count--;
    }
}

static void UpdateParticles(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        particles[i].position.x += particles[i].velocity.x * dt;
        particles[i].position.y += particles[i].velocity.y * dt;
        particles[i].velocity.x *= 0.99f;
        particles[i].velocity.y *= 0.99f;
        particles[i].lifetime -= dt;
        if (particles[i].lifetime <= 0) particles[i].active = false;
    }
}

static void DrawParticles(void)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        float alpha = particles[i].lifetime / particles[i].maxLifetime;
        DrawCircleV(particles[i].position, particles[i].size,
                     ColorAlpha(particles[i].color, alpha));
    }
}

//------------------------------------------------------------------------------------
// Ship
//------------------------------------------------------------------------------------
static void InitShip(void)
{
    ship.position = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    ship.velocity = (Vector2){ 0, 0 };
    ship.rotation = 0;
    ship.alive = true;
    ship.thrusting = false;
    ship.invulnTimer = SHIP_INVULN_TIME;
    ship.fireCooldown = 0;
    ship.respawnTimer = 0;
}

static void FireBullet(void)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) continue;
        Vector2 nose = ShipVertex(0);
        float rad = ship.rotation * DEG2RAD;
        Vector2 dir = { sinf(rad), -cosf(rad) };
        bullets[i].position = nose;
        bullets[i].velocity = (Vector2){
            dir.x * BULLET_SPEED + ship.velocity.x,
            dir.y * BULLET_SPEED + ship.velocity.y
        };
        bullets[i].lifetime = BULLET_LIFETIME;
        bullets[i].active = true;
        break;
    }
}

static void UpdateShip(float dt)
{
    if (!ship.alive) {
        ship.respawnTimer -= dt;
        if (ship.respawnTimer <= 0) {
            if (game.lives > 0) {
                InitShip();
            } else {
                game.state = STATE_GAMEOVER;
                game.gameOverTimer = 2.0f;
                if (game.score > game.highScore) game.highScore = game.score;
            }
        }
        return;
    }

    // Rotation
    if (IsActionDown(ACTION_LEFT)) ship.rotation -= SHIP_ROTATION_SPEED * dt;
    if (IsActionDown(ACTION_RIGHT)) ship.rotation += SHIP_ROTATION_SPEED * dt;

    // Thrust
    ship.thrusting = IsActionDown(ACTION_UP);
    if (ship.thrusting) {
        float rad = ship.rotation * DEG2RAD;
        Vector2 dir = { sinf(rad), -cosf(rad) };
        ship.velocity.x += dir.x * SHIP_THRUST_ACCEL * dt;
        ship.velocity.y += dir.y * SHIP_THRUST_ACCEL * dt;
        float speed = Vector2Length(ship.velocity);
        if (speed > SHIP_MAX_SPEED) {
            ship.velocity = Vector2Scale(ship.velocity, SHIP_MAX_SPEED / speed);
        }
    }

    // Drag
    ship.velocity = Vector2Scale(ship.velocity, SHIP_DRAG);

    // Position
    ship.position.x += ship.velocity.x * dt;
    ship.position.y += ship.velocity.y * dt;
    ship.position = WrapPosition(ship.position);

    // Invulnerability
    if (ship.invulnTimer > 0) ship.invulnTimer -= dt;

    // Firing
    ship.fireCooldown -= dt;
    if (IsActionPressed(ACTION_FIRE) && ship.fireCooldown <= 0) {
        FireBullet();
        ship.fireCooldown = FIRE_COOLDOWN;
    }
}

static void DrawShip(void)
{
    if (!ship.alive) return;

    // Blinking during invulnerability
    if (ship.invulnTimer > 0) {
        if (fmodf(ship.invulnTimer, SHIP_BLINK_RATE * 2.0f) > SHIP_BLINK_RATE) return;
    }

    Vector2 v0 = ShipVertex(0);
    Vector2 v1 = ShipVertex(1);
    Vector2 v2 = ShipVertex(2);
    DrawNeonTriangleLines(v0, v1, v2, GREEN);
}

static void DrawThrustFlame(void)
{
    if (!ship.alive || !ship.thrusting) return;
    if (ship.invulnTimer > 0 && fmodf(ship.invulnTimer, SHIP_BLINK_RATE * 2.0f) > SHIP_BLINK_RATE) return;

    Vector2 left = ShipVertex(1);
    Vector2 right = ShipVertex(2);
    // Inset the base slightly
    Vector2 baseLeft = {
        left.x * 0.7f + right.x * 0.3f,
        left.y * 0.7f + right.y * 0.3f
    };
    Vector2 baseRight = {
        left.x * 0.3f + right.x * 0.7f,
        left.y * 0.3f + right.y * 0.7f
    };

    float flameLen = SHIP_SIZE * (0.5f + 0.5f * (float)GetRandomValue(50, 100) / 100.0f);
    float rad = ship.rotation * DEG2RAD;
    Vector2 tailDir = { -sinf(rad), cosf(rad) };
    Vector2 mid = {
        (baseLeft.x + baseRight.x) * 0.5f,
        (baseLeft.y + baseRight.y) * 0.5f
    };
    Vector2 tip = { mid.x + tailDir.x * flameLen, mid.y + tailDir.y * flameLen };

    Color flameColor = GetRandomValue(0, 1) ? ORANGE : YELLOW;
    DrawNeonLine(baseLeft, tip, flameColor);
    DrawNeonLine(baseRight, tip, flameColor);

    // Thrust particles
    if (GetRandomValue(0, 1)) {
        SpawnParticles(mid, 1, 50.0f, 1.0f, 2.0f, ORANGE);
    }
}

//------------------------------------------------------------------------------------
// Bullets
//------------------------------------------------------------------------------------
static void UpdateBullets(float dt)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        bullets[i].position.x += bullets[i].velocity.x * dt;
        bullets[i].position.y += bullets[i].velocity.y * dt;
        bullets[i].position = WrapPosition(bullets[i].position);
        bullets[i].lifetime -= dt;
        if (bullets[i].lifetime <= 0) bullets[i].active = false;
    }
}

static void DrawBullets(void)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        DrawNeonCircle(bullets[i].position, BULLET_RADIUS, WHITE);
        // Short trail
        Vector2 vel = bullets[i].velocity;
        float len = Vector2Length(vel);
        if (len > 0) {
            Vector2 trail = {
                bullets[i].position.x - (vel.x / len) * 8.0f,
                bullets[i].position.y - (vel.y / len) * 8.0f
            };
            DrawNeonLine(bullets[i].position, trail, ColorAlpha(WHITE, 0.5f));
        }
    }
}

//------------------------------------------------------------------------------------
// Asteroids
//------------------------------------------------------------------------------------
static void GenerateAsteroidShape(Asteroid *a)
{
    a->vertCount = GetRandomValue(ASTEROID_VERT_MIN, ASTEROID_VERT_MAX);
    for (int i = 0; i < a->vertCount; i++) {
        float jag = (float)GetRandomValue(-100, 100) / 100.0f * ASTEROID_JAGGEDNESS;
        a->vertDistances[i] = a->radius * (1.0f + jag);
    }
}

static float AsteroidRadius(AsteroidSize size)
{
    switch (size) {
        case ASTEROID_LARGE:  return ASTEROID_LARGE_R;
        case ASTEROID_MEDIUM: return ASTEROID_MEDIUM_R;
        case ASTEROID_SMALL:  return ASTEROID_SMALL_R;
    }
    return ASTEROID_LARGE_R;
}

static void SpawnAsteroid(AsteroidSize size, Vector2 pos, Vector2 vel)
{
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) continue;
        asteroids[i].position = pos;
        asteroids[i].velocity = vel;
        asteroids[i].size = size;
        asteroids[i].radius = AsteroidRadius(size);
        asteroids[i].rotation = (float)GetRandomValue(0, 360);
        asteroids[i].rotationSpeed = (float)GetRandomValue(-90, 90);
        asteroids[i].active = true;
        GenerateAsteroidShape(&asteroids[i]);
        break;
    }
}

static void SpawnAsteroidWave(int count)
{
    for (int n = 0; n < count; n++) {
        Vector2 pos;
        int edge = GetRandomValue(0, 3);
        switch (edge) {
            case 0: pos = (Vector2){ (float)GetRandomValue(0, SCREEN_WIDTH), -ASTEROID_LARGE_R }; break;
            case 1: pos = (Vector2){ (float)GetRandomValue(0, SCREEN_WIDTH), SCREEN_HEIGHT + ASTEROID_LARGE_R }; break;
            case 2: pos = (Vector2){ -ASTEROID_LARGE_R, (float)GetRandomValue(0, SCREEN_HEIGHT) }; break;
            default: pos = (Vector2){ SCREEN_WIDTH + ASTEROID_LARGE_R, (float)GetRandomValue(0, SCREEN_HEIGHT) }; break;
        }
        // Aim roughly toward center with some randomness
        Vector2 center = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        Vector2 dir = Vector2Subtract(center, pos);
        float angle = atan2f(dir.y, dir.x) + (float)GetRandomValue(-30, 30) * DEG2RAD;
        float speed = ASTEROID_SPEED_MIN +
            (ASTEROID_SPEED_MAX - ASTEROID_SPEED_MIN) * (float)GetRandomValue(0, 100) / 100.0f;
        speed *= 1.0f + (game.wave - 1) * 0.1f;
        Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
        SpawnAsteroid(ASTEROID_LARGE, pos, vel);
    }
}

static void SplitAsteroid(int index)
{
    Asteroid *a = &asteroids[index];
    Vector2 pos = a->position;
    Vector2 vel = a->velocity;
    AsteroidSize size = a->size;
    a->active = false;

    // Particles
    int pcount;
    Color pcolor;
    switch (size) {
        case ASTEROID_LARGE:  pcount = 25; break;
        case ASTEROID_MEDIUM: pcount = 14; break;
        default:              pcount = 7;  break;
    }
    Color colors[] = { WHITE, LIGHTGRAY, (Color){ 100, 200, 220, 255 } };
    pcolor = colors[GetRandomValue(0, 2)];
    SpawnParticles(pos, pcount, 80.0f, 1.0f, 3.0f, pcolor);

    // Screen shake
    if (size == ASTEROID_LARGE) {
        game.screenShakeTimer = SCREEN_SHAKE_DURATION;
        game.screenShakeMagnitude = SCREEN_SHAKE_MAGNITUDE;
    } else if (size == ASTEROID_MEDIUM) {
        game.screenShakeTimer = SCREEN_SHAKE_DURATION * 0.5f;
        game.screenShakeMagnitude = SCREEN_SHAKE_MAGNITUDE * 0.5f;
    }

    // Split into smaller pieces
    if (size == ASTEROID_LARGE || size == ASTEROID_MEDIUM) {
        AsteroidSize newSize = (size == ASTEROID_LARGE) ? ASTEROID_MEDIUM : ASTEROID_SMALL;
        for (int i = 0; i < 2; i++) {
            float angleOff = (float)GetRandomValue(30, 60) * (i == 0 ? 1.0f : -1.0f) * DEG2RAD;
            float baseAngle = atan2f(vel.y, vel.x) + angleOff;
            float speed = Vector2Length(vel) * 1.5f;
            if (speed < ASTEROID_SPEED_MIN) speed = ASTEROID_SPEED_MIN;
            Vector2 newVel = { cosf(baseAngle) * speed, sinf(baseAngle) * speed };
            SpawnAsteroid(newSize, pos, newVel);
        }
    }
}

static int CountActiveAsteroids(void)
{
    int count = 0;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) count++;
    }
    return count;
}

static void UpdateAsteroids(float dt)
{
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].active) continue;
        asteroids[i].position.x += asteroids[i].velocity.x * dt;
        asteroids[i].position.y += asteroids[i].velocity.y * dt;
        asteroids[i].position = WrapPosition(asteroids[i].position);
        asteroids[i].rotation += asteroids[i].rotationSpeed * dt;
    }
}

static void DrawAsteroids(void)
{
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].active) continue;
        Asteroid *a = &asteroids[i];
        Vector2 points[MAX_ASTEROID_VERTS + 1];
        for (int v = 0; v < a->vertCount; v++) {
            float angleDeg = (360.0f / (float)a->vertCount) * (float)v + a->rotation;
            float rad = angleDeg * DEG2RAD;
            points[v] = (Vector2){
                a->position.x + cosf(rad) * a->vertDistances[v],
                a->position.y + sinf(rad) * a->vertDistances[v]
            };
        }
        points[a->vertCount] = points[0]; // close the shape
        DrawNeonLineStrip(points, a->vertCount + 1, LIGHTGRAY);
    }
}

//------------------------------------------------------------------------------------
// Collisions
//------------------------------------------------------------------------------------
static void UpdateCollisions(void)
{
    // Bullets vs Asteroids
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!bullets[b].active) continue;
        for (int a = 0; a < MAX_ASTEROIDS; a++) {
            if (!asteroids[a].active) continue;
            if (CheckCollisionCircles(bullets[b].position, BULLET_RADIUS,
                                      asteroids[a].position, asteroids[a].radius)) {
                bullets[b].active = false;
                switch (asteroids[a].size) {
                    case ASTEROID_LARGE:  game.score += SCORE_LARGE;  break;
                    case ASTEROID_MEDIUM: game.score += SCORE_MEDIUM; break;
                    case ASTEROID_SMALL:  game.score += SCORE_SMALL;  break;
                }
                SplitAsteroid(a);
                break;
            }
        }
    }

    // Ship vs Asteroids
    if (ship.alive && ship.invulnTimer <= 0) {
        for (int a = 0; a < MAX_ASTEROIDS; a++) {
            if (!asteroids[a].active) continue;
            if (CheckCollisionCircles(ship.position, SHIP_SIZE * 0.5f,
                                      asteroids[a].position, asteroids[a].radius)) {
                ship.alive = false;
                ship.respawnTimer = 2.0f;
                game.lives--;
                SpawnParticles(ship.position, 35, 150.0f, 1.0f, 3.0f, GREEN);
                SpawnParticles(ship.position, 10, 80.0f, 2.0f, 4.0f, LIME);
                game.screenShakeTimer = SCREEN_SHAKE_DURATION;
                game.screenShakeMagnitude = SCREEN_SHAKE_MAGNITUDE * 1.5f;
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------------
// Wave Progression
//------------------------------------------------------------------------------------
static void UpdateWaveProgression(float dt)
{
    if (game.waveCleared) {
        game.waveTimer -= dt;
        if (game.waveTimer <= 0) {
            game.wave++;
            int count = 2 + game.wave;
            if (count > 12) count = 12;
            SpawnAsteroidWave(count);
            game.waveCleared = false;
        }
    } else {
        if (CountActiveAsteroids() == 0) {
            game.waveCleared = true;
            game.waveTimer = WAVE_DELAY;
        }
    }
}

//------------------------------------------------------------------------------------
// Screen Shake
//------------------------------------------------------------------------------------
static void UpdateScreenShake(float dt)
{
    if (game.screenShakeTimer > 0) game.screenShakeTimer -= dt;
    if (game.screenShakeTimer < 0) game.screenShakeTimer = 0;
}

//------------------------------------------------------------------------------------
// HUD
//------------------------------------------------------------------------------------
static void DrawHUD(void)
{
    // Score
    DrawText(TextFormat("%d", game.score), 10, 10, 20, WHITE);

    // High score
    const char *hiText = TextFormat("HI %d", game.highScore);
    int hiWidth = MeasureText(hiText, 20);
    DrawText(hiText, SCREEN_WIDTH / 2 - hiWidth / 2, 10, 20, GRAY);

    // Wave
    DrawText(TextFormat("WAVE %d", game.wave), SCREEN_WIDTH - 110, 10, 20, GRAY);

    // Lives (small ship icons)
    for (int i = 0; i < game.lives; i++) {
        float cx = 20.0f + (float)i * 22.0f;
        float cy = 42.0f;
        float s = 7.0f;
        Vector2 lv0 = { cx, cy - s };
        Vector2 lv1 = { cx - s * 0.6f, cy + s * 0.7f };
        Vector2 lv2 = { cx + s * 0.6f, cy + s * 0.7f };
        DrawTriangleLines(lv0, lv2, lv1, GREEN);
    }
}

//------------------------------------------------------------------------------------
// Title Screen
//------------------------------------------------------------------------------------
static void DrawTitleScreen(void)
{
    // Title
    const char *title = "ASTEROIDS";
    int titleWidth = MeasureText(title, 60);
    // Glow
    DrawText(title, SCREEN_WIDTH / 2 - titleWidth / 2 + 1, SCREEN_HEIGHT / 2 - 80 + 1, 60,
             ColorAlpha(GREEN, 0.3f));
    DrawText(title, SCREEN_WIDTH / 2 - titleWidth / 2, SCREEN_HEIGHT / 2 - 80, 60, GREEN);

    // Blinking start text
    if (sinf((float)GetTime() * 5.0f) > 0) {
        const char *start = "PRESS FIRE TO START";
        int startWidth = MeasureText(start, 20);
        DrawText(start, SCREEN_WIDTH / 2 - startWidth / 2, SCREEN_HEIGHT / 2, 20, WHITE);
    }

    // Controls
    const char *ctrl = "WASD / ARROWS to move   SPACE to fire   TAB to switch";
    int ctrlWidth = MeasureText(ctrl, 14);
    DrawText(ctrl, SCREEN_WIDTH / 2 - ctrlWidth / 2, SCREEN_HEIGHT / 2 + 50, 14, GRAY);

    // High score
    if (game.highScore > 0) {
        const char *hi = TextFormat("HIGH SCORE: %d", game.highScore);
        int hiWidth = MeasureText(hi, 18);
        DrawText(hi, SCREEN_WIDTH / 2 - hiWidth / 2, SCREEN_HEIGHT / 2 + 80, 18, YELLOW);
    }
}

//------------------------------------------------------------------------------------
// Game Over Screen
//------------------------------------------------------------------------------------
static void DrawGameOverScreen(void)
{
    const char *go = "GAME OVER";
    int goWidth = MeasureText(go, 50);
    DrawText(go, SCREEN_WIDTH / 2 - goWidth / 2 + 1, SCREEN_HEIGHT / 2 - 60 + 1, 50,
             ColorAlpha(RED, 0.3f));
    DrawText(go, SCREEN_WIDTH / 2 - goWidth / 2, SCREEN_HEIGHT / 2 - 60, 50, RED);

    const char *sc = TextFormat("SCORE: %d", game.score);
    int scWidth = MeasureText(sc, 24);
    DrawText(sc, SCREEN_WIDTH / 2 - scWidth / 2, SCREEN_HEIGHT / 2, 24, WHITE);

    if (game.gameOverTimer <= 0 && sinf((float)GetTime() * 5.0f) > 0) {
        const char *cont = "PRESS FIRE TO CONTINUE";
        int contWidth = MeasureText(cont, 18);
        DrawText(cont, SCREEN_WIDTH / 2 - contWidth / 2, SCREEN_HEIGHT / 2 + 40, 18, GRAY);
    }
}

//------------------------------------------------------------------------------------
// Game State Updates
//------------------------------------------------------------------------------------
static void InitGame(void)
{
    game.score = 0;
    game.lives = INITIAL_LIVES;
    game.wave = 0;
    game.waveCleared = false;
    game.waveTimer = 0;
    game.screenShakeTimer = 0;
    game.screenShakeMagnitude = 0;
    game.nextExtraLife = EXTRA_LIFE_SCORE;
    game.gameOverTimer = 0;
    memset(bullets, 0, sizeof(bullets));
    memset(asteroids, 0, sizeof(asteroids));
    memset(particles, 0, sizeof(particles));
    InitShip();
}

static void StartGame(void)
{
    InitGame();
    game.state = STATE_PLAYING;
    game.wave = 1;
    SpawnAsteroidWave(3);
}

static void UpdateTitleScreen(float dt)
{
    (void)dt;
    if (IsActionPressed(ACTION_FIRE)) {
        StartGame();
    }
}

static void UpdatePlaying(float dt)
{
    UpdateShip(dt);
    UpdateBullets(dt);
    UpdateAsteroids(dt);
    UpdateCollisions();
    UpdateWaveProgression(dt);

    // Extra life
    if (game.score >= game.nextExtraLife) {
        game.lives++;
        game.nextExtraLife += EXTRA_LIFE_SCORE;
    }
}

static void UpdateGameOver(float dt)
{
    // Keep updating visuals
    UpdateBullets(dt);
    UpdateAsteroids(dt);

    game.gameOverTimer -= dt;
    if (game.gameOverTimer <= 0 && IsActionPressed(ACTION_FIRE)) {
        game.state = STATE_TITLE;
    }
}

//------------------------------------------------------------------------------------
// Main Entry Point
//------------------------------------------------------------------------------------
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Asteroids");
    SetTargetFPS(60);

    SetActionsDefault();
    InitStars();
    game.state = STATE_TITLE;
    game.highScore = 0;

    char actionSet = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        gamepadIndex = 0;

        // TAB to toggle keyset
        if (IsKeyPressed(KEY_TAB)) {
            actionSet = !actionSet;
            if (actionSet == 0) SetActionsDefault();
            else SetActionsCursor();
        }

        // Update
        UpdateScreenShake(dt);
        UpdateParticles(dt);

        switch (game.state) {
            case STATE_TITLE:    UpdateTitleScreen(dt); break;
            case STATE_PLAYING:  UpdatePlaying(dt);     break;
            case STATE_GAMEOVER: UpdateGameOver(dt);     break;
        }

        // Draw
        BeginDrawing();
        ClearBackground(BLACK);

        // Camera for screen shake
        Camera2D camera = { 0 };
        camera.target = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        camera.zoom = 1.0f;
        if (game.screenShakeTimer > 0) {
            float intensity = game.screenShakeMagnitude * (game.screenShakeTimer / SCREEN_SHAKE_DURATION);
            camera.target.x += (float)GetRandomValue(-10, 10) / 10.0f * intensity;
            camera.target.y += (float)GetRandomValue(-10, 10) / 10.0f * intensity;
        }
        BeginMode2D(camera);

        DrawStarfield();

        if (game.state == STATE_PLAYING || game.state == STATE_GAMEOVER) {
            DrawAsteroids();
            DrawBullets();
            DrawParticles();
            if (ship.alive) {
                DrawShip();
                DrawThrustFlame();
            } else {
                // Still draw particles when ship is dead
                DrawParticles();
            }
        }

        EndMode2D();

        // HUD and overlay screens (outside camera — no shake)
        switch (game.state) {
            case STATE_PLAYING:  DrawHUD();            break;
            case STATE_TITLE:    DrawTitleScreen();     break;
            case STATE_GAMEOVER: DrawGameOverScreen();  break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
