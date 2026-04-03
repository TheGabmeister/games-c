#include "gameplay.h"
#include "particles.h"
#include "drawing.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

// Shield pattern: 1=intact, 0=empty
static const int shieldPattern[SHIELD_ROWS][SHIELD_COLS] = {
    {0,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,0,0,1,1,1},
    {1,1,0,0,0,0,1,1},
};

static const int ufoScores[] = {50, 100, 150, 200, 300};

// Forward declarations
static void InitWave(GameState *g);
static void InitShields(GameState *g);
static void UpdatePlayer(GameState *g, float dt);
static void UpdateAliens(GameState *g, float dt);
static void UpdateEnemyFire(GameState *g, float dt);
static void UpdateBullets(GameState *g, float dt);
static void UpdateUFO(GameState *g, float dt);
static void ResolveCollisions(GameState *g, int *highScore);
static void CheckWaveState(GameState *g);
static void SpawnFloatTextAt(GameState *g, Vector2 pos, Color color, int score);
static void TriggerShake(GameState *g, float amplitude, float duration);
static Vector2 GetShakeOffset(GameState *g);

// --- Init ---

void GameInit(GameState *g)
{
    memset(g, 0, sizeof(*g));
    g->score = 0;
    g->wave = 1;
    g->lives = PLAYER_LIVES;
    g->playerX = SCREEN_W / 2.0f;
    g->playerActive = true;
    g->alienDir = 1.0f;
    g->ufoFromLeft = true;

    InitStars(g->stars);
    InitWave(g);
}

static void InitWave(GameState *g)
{
    // Aliens
    g->aliveCount = ALIEN_COUNT;
    g->animFrame = 0;
    g->alienDir = 1.0f;
    g->stepTimer = 0;
    g->formX = (float)ALIEN_ORIGIN_X;

    float baseY = (float)(ALIEN_ORIGIN_Y + (g->wave - 1) * ALIEN_SPACING_Y);
    if (baseY > 250.0f) baseY = 250.0f;
    g->formY = baseY;

    for (int r = 0; r < ALIEN_ROWS; r++) {
        for (int c = 0; c < ALIEN_COLS; c++) {
            g->aliens[r][c].alive = true;
            g->aliens[r][c].type = AlienTypeForRow(r);
        }
    }

    // Shields
    InitShields(g);

    // Bullets
    g->pBullet.active = false;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) g->eBullets[i].active = false;

    // UFO
    g->ufo.active = false;
    g->ufoTimer = 0;
    g->ufoSpawnTime = (float)GetRandomValue(15, 25);

    // Enemy fire
    g->eFireTimer = 0;

    // Wave state
    g->waveComplete = false;
    g->wavePauseT = 0;

    // Player
    g->playerActive = true;
    g->invulnT = 0;
    g->dead = false;
    g->deathDelayT = 0;
}

static void InitShields(GameState *g)
{
    float shieldW = (float)(SHIELD_COLS * SHIELD_CELL);
    float playW = (float)(PLAY_RIGHT - PLAY_LEFT);
    float gap = (playW - SHIELD_COUNT * shieldW) / (SHIELD_COUNT + 1);

    for (int s = 0; s < SHIELD_COUNT; s++) {
        g->shields[s].x = PLAY_LEFT + gap + s * (shieldW + gap);
        g->shields[s].y = (float)SHIELD_TOP_Y;

        for (int r = 0; r < SHIELD_ROWS; r++) {
            for (int c = 0; c < SHIELD_COLS; c++) {
                g->shields[s].cells[r][c] = shieldPattern[r][c] ? CELL_INTACT : CELL_DEAD;
                g->shields[s].flash[r][c] = 0;
            }
        }
    }
}

// --- Update ---

void GameUpdate(GameState *g, float dt, int *highScore)
{
    // Always update visual effects
    UpdateParticles(g->particles, dt);
    UpdateFloatTexts(g->floatTexts, dt);
    UpdateStars(g->stars, dt);

    // Shield flash timers
    for (int s = 0; s < SHIELD_COUNT; s++) {
        for (int r = 0; r < SHIELD_ROWS; r++) {
            for (int c = 0; c < SHIELD_COLS; c++) {
                if (g->shields[s].cells[r][c] == CELL_FLASH) {
                    g->shields[s].flash[r][c] -= dt;
                    if (g->shields[s].flash[r][c] <= 0)
                        g->shields[s].cells[r][c] = CELL_DEAD;
                }
            }
        }
    }

    // Shake decay
    if (g->shakeT > 0) {
        g->shakeT -= dt;
        if (g->shakeT < 0) g->shakeT = 0;
    }

    // Wave complete pause
    if (g->waveComplete) {
        g->wavePauseT -= dt;
        if (g->wavePauseT <= 0) {
            g->wave++;
            InitWave(g);
        }
        return;
    }

    // Death delay
    if (g->dead) {
        g->deathDelayT -= dt;
        return;
    }

    // Gameplay
    UpdatePlayer(g, dt);
    UpdateAliens(g, dt);
    UpdateEnemyFire(g, dt);
    UpdateBullets(g, dt);
    UpdateUFO(g, dt);
    ResolveCollisions(g, highScore);
    CheckWaveState(g);
}

// --- Player ---

static void UpdatePlayer(GameState *g, float dt)
{
    if (!g->playerActive) return;

    if (g->invulnT > 0) g->invulnT -= dt;

    // Movement
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        g->playerX -= PLAYER_SPEED * dt;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        g->playerX += PLAYER_SPEED * dt;

    // Clamp
    float halfW = PLAYER_W / 2.0f;
    if (g->playerX - halfW < PLAY_LEFT) g->playerX = PLAY_LEFT + halfW;
    if (g->playerX + halfW > PLAY_RIGHT) g->playerX = PLAY_RIGHT - halfW;

    // Fire
    if (IsKeyPressed(KEY_SPACE) && !g->pBullet.active) {
        g->pBullet.active = true;
        g->pBullet.pos = (Vector2){ g->playerX, PLAYER_BASE_Y - PLAYER_H / 2.0f };
        g->pBullet.vel = (Vector2){ 0, -PBULLET_SPEED };
        g->pBullet.color = COL_PLAYER;
        g->pBullet.trailN = 0;
    }
}

// --- Aliens ---

static void UpdateAliens(GameState *g, float dt)
{
    if (g->aliveCount <= 0) return;

    // Step interval
    float stepInterval = 0.02f + ((float)g->aliveCount / 55.0f) * 0.48f;
    float waveMult = 1.0f - (g->wave - 1) * 0.10f;
    if (waveMult < 0.35f) waveMult = 0.35f;
    float finalInterval = stepInterval * waveMult;

    g->stepTimer += dt;
    if (g->stepTimer < finalInterval) return;
    g->stepTimer -= finalInterval;

    // Toggle animation
    g->animFrame = 1 - g->animFrame;

    // Find leftmost and rightmost alive columns
    int leftCol = ALIEN_COLS, rightCol = -1;
    for (int c = 0; c < ALIEN_COLS; c++) {
        for (int r = 0; r < ALIEN_ROWS; r++) {
            if (g->aliens[r][c].alive) {
                if (c < leftCol) leftCol = c;
                if (c > rightCol) rightCol = c;
                break;
            }
        }
    }
    if (rightCol < 0) return;

    // Edge detection
    float nextX = g->formX + ALIEN_STEP_X * g->alienDir;
    float leftEdge = nextX + leftCol * ALIEN_SPACING_X - ALIEN_W / 2.0f;
    float rightEdge = nextX + rightCol * ALIEN_SPACING_X + ALIEN_W / 2.0f;

    if (leftEdge < PLAY_LEFT || rightEdge > PLAY_RIGHT) {
        g->alienDir *= -1;
        g->formY += ALIEN_DROP_Y;
    } else {
        g->formX = nextX;
    }
}

// --- Enemy Fire ---

static void UpdateEnemyFire(GameState *g, float dt)
{
    if (g->aliveCount <= 0) return;

    float shotRate = 0.30f + (1.0f - (float)g->aliveCount / 55.0f) * 0.90f;
    float interval = 1.0f / shotRate;

    g->eFireTimer += dt;
    if (g->eFireTimer < interval) return;
    g->eFireTimer -= interval;

    // Count active enemy bullets
    int activeCount = 0;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
        if (g->eBullets[i].active) activeCount++;
    if (activeCount >= MAX_ENEMY_BULLETS) return;

    // Pick random alive column
    int aliveCols[ALIEN_COLS];
    int numAliveCols = 0;
    for (int c = 0; c < ALIEN_COLS; c++) {
        for (int r = 0; r < ALIEN_ROWS; r++) {
            if (g->aliens[r][c].alive) {
                aliveCols[numAliveCols++] = c;
                break;
            }
        }
    }
    if (numAliveCols == 0) return;

    int col = aliveCols[GetRandomValue(0, numAliveCols - 1)];

    // Find bottom-most alive alien in column
    int botRow = -1;
    for (int r = ALIEN_ROWS - 1; r >= 0; r--) {
        if (g->aliens[r][col].alive) { botRow = r; break; }
    }
    if (botRow < 0) return;

    // Spawn bullet
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!g->eBullets[i].active) {
            float ax = g->formX + col * ALIEN_SPACING_X;
            float ay = g->formY + botRow * ALIEN_SPACING_Y + ALIEN_H / 2.0f;
            g->eBullets[i].active = true;
            g->eBullets[i].pos = (Vector2){ ax, ay };
            g->eBullets[i].vel = (Vector2){ 0, EBULLET_SPEED };
            g->eBullets[i].color = COL_ENEMY_FIRE;
            g->eBullets[i].trailN = 0;
            break;
        }
    }
}

// --- Bullets ---

static void UpdateBulletTrail(Bullet *b)
{
    for (int i = TRAIL_COUNT - 1; i > 0; i--)
        b->trail[i] = b->trail[i - 1];
    b->trail[0] = b->pos;
    if (b->trailN < TRAIL_COUNT) b->trailN++;
}

static void UpdateBullets(GameState *g, float dt)
{
    if (g->pBullet.active) {
        UpdateBulletTrail(&g->pBullet);
        g->pBullet.pos.x += g->pBullet.vel.x * dt;
        g->pBullet.pos.y += g->pBullet.vel.y * dt;
        if (g->pBullet.pos.y < -10) g->pBullet.active = false;
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!g->eBullets[i].active) continue;
        UpdateBulletTrail(&g->eBullets[i]);
        g->eBullets[i].pos.x += g->eBullets[i].vel.x * dt;
        g->eBullets[i].pos.y += g->eBullets[i].vel.y * dt;
        if (g->eBullets[i].pos.y > SCREEN_H + 10)
            g->eBullets[i].active = false;
    }
}

// --- UFO ---

static void UpdateUFO(GameState *g, float dt)
{
    if (g->ufo.active) {
        g->ufo.x += UFO_SPEED * g->ufo.dir * dt;
        g->ufo.glowT += dt;
        if (g->ufo.dir > 0 && g->ufo.x > SCREEN_W + UFO_W)
            g->ufo.active = false;
        if (g->ufo.dir < 0 && g->ufo.x < -UFO_W)
            g->ufo.active = false;
    } else {
        g->ufoTimer += dt;
        if (g->ufoTimer >= g->ufoSpawnTime) {
            g->ufo.active = true;
            g->ufo.dir = g->ufoFromLeft ? 1.0f : -1.0f;
            g->ufo.x = g->ufoFromLeft ? (float)(-UFO_W / 2) : (float)(SCREEN_W + UFO_W / 2);
            g->ufo.y = (float)UFO_LANE_Y;
            g->ufo.score = ufoScores[GetRandomValue(0, 4)];
            g->ufo.glowT = 0;
            g->ufoFromLeft = !g->ufoFromLeft;
            g->ufoTimer = 0;
            g->ufoSpawnTime = (float)GetRandomValue(15, 25);
        }
    }
}

// --- Collision Helpers ---

static Rectangle BulletRect(Bullet *b)
{
    return (Rectangle){ b->pos.x - BULLET_W / 2.0f, b->pos.y - BULLET_H / 2.0f, BULLET_W, BULLET_H };
}

static Rectangle AlienRect(GameState *g, int r, int c)
{
    float ax = g->formX + c * ALIEN_SPACING_X;
    float ay = g->formY + r * ALIEN_SPACING_Y;
    return (Rectangle){ ax - ALIEN_W / 2.0f, ay - ALIEN_H / 2.0f, ALIEN_W, ALIEN_H };
}

static Rectangle PlayerRect(GameState *g)
{
    return (Rectangle){ g->playerX - PLAYER_W / 2.0f, PLAYER_BASE_Y - PLAYER_H / 2.0f, PLAYER_W, PLAYER_H };
}

static Rectangle UFORect(GameState *g)
{
    return (Rectangle){ g->ufo.x - UFO_W / 2.0f, g->ufo.y - UFO_H / 2.0f, UFO_W, UFO_H };
}

static void KillAlien(GameState *g, int r, int c, int *highScore)
{
    g->aliens[r][c].alive = false;
    g->aliveCount--;

    int pts = AlienScore(g->aliens[r][c].type);
    g->score += pts;
    if (g->score > *highScore) *highScore = g->score;

    Vector2 pos = { g->formX + c * ALIEN_SPACING_X, g->formY + r * ALIEN_SPACING_Y };
    Color col = AlienColor(g->aliens[r][c].type);

    // Alien death particles
    SpawnParticles(g->particles, pos, col,
                   GetRandomValue(15, 25), 80, 200, 0.3f, 0.6f, 2, 4);
    SpawnFloatTextAt(g, pos, col, pts);
}

static void HitPlayer(GameState *g)
{
    if (g->invulnT > 0) return;

    g->lives--;

    Vector2 pos = { g->playerX, (float)PLAYER_BASE_Y };

    // Player death particles
    SpawnParticles(g->particles, pos, COL_PLAYER,
                   GetRandomValue(30, 40), 100, 300, 0.4f, 0.8f, 3, 5);
    TriggerShake(g, 4, 0.30f);

    // Clear all bullets
    g->pBullet.active = false;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
        g->eBullets[i].active = false;

    if (g->lives <= 0) {
        g->dead = true;
        g->deathDelayT = DEATH_DELAY;
        g->playerActive = false;
    } else {
        g->invulnT = INVULN_DURATION;
    }
}

static bool HitShield(GameState *g, Bullet *b)
{
    Rectangle br = BulletRect(b);

    for (int s = 0; s < SHIELD_COUNT; s++) {
        for (int r = 0; r < SHIELD_ROWS; r++) {
            for (int c = 0; c < SHIELD_COLS; c++) {
                if (g->shields[s].cells[r][c] != CELL_INTACT) continue;

                Rectangle cr = {
                    g->shields[s].x + c * SHIELD_CELL,
                    g->shields[s].y + r * SHIELD_CELL,
                    SHIELD_CELL, SHIELD_CELL
                };

                if (CheckCollisionRecs(br, cr)) {
                    g->shields[s].cells[r][c] = CELL_FLASH;
                    g->shields[s].flash[r][c] = 0.1f;

                    Vector2 hp = { cr.x + SHIELD_CELL / 2.0f, cr.y + SHIELD_CELL / 2.0f };
                    SpawnParticles(g->particles, hp, COL_SHIELD,
                                   GetRandomValue(5, 8), 40, 100, 0.2f, 0.4f, 2, 3);
                    TriggerShake(g, 2, 0.10f);

                    b->active = false;
                    return true;
                }
            }
        }
    }
    return false;
}

// --- Collision Resolution ---

static void ResolveCollisions(GameState *g, int *highScore)
{
    // Player bullet: priority UFO > alien > shield
    if (g->pBullet.active) {
        Rectangle pb = BulletRect(&g->pBullet);

        // 1. UFO
        if (g->ufo.active && CheckCollisionRecs(pb, UFORect(g))) {
            g->score += g->ufo.score;
            if (g->score > *highScore) *highScore = g->score;

            Vector2 upos = { g->ufo.x, g->ufo.y };
            SpawnParticles(g->particles, upos, (Color){ 255, 50, 50, 255 },
                           GetRandomValue(15, 25), 80, 200, 0.3f, 0.6f, 2, 4);
            SpawnFloatTextAt(g, upos, (Color){ 255, 50, 50, 255 }, g->ufo.score);

            g->ufo.active = false;
            g->pBullet.active = false;
        }

        // 2. Aliens (bottom rows first — nearest hit)
        if (g->pBullet.active) {
            bool hit = false;
            for (int r = ALIEN_ROWS - 1; r >= 0 && !hit; r--) {
                for (int c = 0; c < ALIEN_COLS && !hit; c++) {
                    if (!g->aliens[r][c].alive) continue;
                    if (CheckCollisionRecs(BulletRect(&g->pBullet), AlienRect(g, r, c))) {
                        // Bullet impact particles
                        SpawnParticles(g->particles, g->pBullet.pos, g->pBullet.color,
                                       GetRandomValue(8, 12), 60, 150, 0.15f, 0.3f, 1, 2);
                        KillAlien(g, r, c, highScore);
                        g->pBullet.active = false;
                        hit = true;
                    }
                }
            }
        }

        // 3. Shield
        if (g->pBullet.active) {
            HitShield(g, &g->pBullet);
        }
    }

    // Enemy bullets: player, shield
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!g->eBullets[i].active) continue;

        // Player
        if (g->playerActive && g->invulnT <= 0) {
            if (CheckCollisionRecs(BulletRect(&g->eBullets[i]), PlayerRect(g))) {
                g->eBullets[i].active = false;
                HitPlayer(g);
                continue;
            }
        }

        // Shield
        HitShield(g, &g->eBullets[i]);
    }
}

// --- Wave State ---

static void CheckWaveState(GameState *g)
{
    // Invasion check
    for (int r = ALIEN_ROWS - 1; r >= 0; r--) {
        for (int c = 0; c < ALIEN_COLS; c++) {
            if (!g->aliens[r][c].alive) continue;
            float alienBottom = g->formY + r * ALIEN_SPACING_Y + ALIEN_H / 2.0f;
            if (alienBottom >= SHIELD_TOP_Y) {
                g->dead = true;
                g->deathDelayT = DEATH_DELAY;
                g->playerActive = false;
                g->lives = 0;
                return;
            }
        }
    }

    // Wave complete
    if (g->aliveCount <= 0 && !g->waveComplete) {
        g->waveComplete = true;
        g->wavePauseT = WAVE_PAUSE;
        g->pBullet.active = false;
        for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
            g->eBullets[i].active = false;
        g->ufo.active = false;
    }
}

// --- Effects ---

static void SpawnFloatTextAt(GameState *g, Vector2 pos, Color color, int score)
{
    for (int i = 0; i < FLOAT_TEXT_MAX; i++) {
        if (!g->floatTexts[i].active) {
            g->floatTexts[i].active = true;
            g->floatTexts[i].pos = pos;
            g->floatTexts[i].color = color;
            g->floatTexts[i].life = 1.0f;
            snprintf(g->floatTexts[i].text, sizeof(g->floatTexts[i].text), "+%d", score);
            break;
        }
    }
}

static void TriggerShake(GameState *g, float amplitude, float duration)
{
    g->shakeAmp = amplitude;
    g->shakeT = duration;
}

static Vector2 GetShakeOffset(GameState *g)
{
    if (g->shakeT <= 0) return (Vector2){ 0, 0 };
    float t = g->shakeT / (g->shakeAmp > 3 ? 0.30f : 0.10f); // normalized decay
    if (t > 1.0f) t = 1.0f;
    float ox = (float)GetRandomValue(-100, 100) / 100.0f * g->shakeAmp * t;
    float oy = (float)GetRandomValue(-100, 100) / 100.0f * g->shakeAmp * t;
    return (Vector2){ ox, oy };
}

// --- Draw ---

void GameDraw(GameState *g)
{
    Vector2 shake = GetShakeOffset(g);

    // 1-2. Starfield (no shake)
    DrawStars(g->stars);

    // 3. Shields
    for (int s = 0; s < SHIELD_COUNT; s++)
        DrawShieldBlock(&g->shields[s], shake);

    // 4. Aliens
    for (int r = 0; r < ALIEN_ROWS; r++) {
        for (int c = 0; c < ALIEN_COLS; c++) {
            if (!g->aliens[r][c].alive) continue;
            float ax = g->formX + c * ALIEN_SPACING_X + shake.x;
            float ay = g->formY + r * ALIEN_SPACING_Y + shake.y;
            DrawAlienShape(g->aliens[r][c].type, ax, ay, g->animFrame);
        }
    }

    // 5. Mystery ship
    if (g->ufo.active) {
        UFOShip tempUfo = g->ufo;
        tempUfo.x += shake.x;
        tempUfo.y += shake.y;
        DrawUFOShape(&tempUfo);
    }

    // 6. Bullets + trails
    if (g->pBullet.active) {
        Bullet tb = g->pBullet;
        tb.pos.x += shake.x;
        tb.pos.y += shake.y;
        for (int t = 0; t < tb.trailN; t++) {
            tb.trail[t].x += shake.x;
            tb.trail[t].y += shake.y;
        }
        DrawBulletObj(&tb, true);
    }
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!g->eBullets[i].active) continue;
        Bullet tb = g->eBullets[i];
        tb.pos.x += shake.x;
        tb.pos.y += shake.y;
        for (int t = 0; t < tb.trailN; t++) {
            tb.trail[t].x += shake.x;
            tb.trail[t].y += shake.y;
        }
        DrawBulletObj(&tb, false);
    }

    // 7. Player
    if (g->playerActive) {
        bool blink = false;
        if (g->invulnT > 0)
            blink = ((int)(g->invulnT * 10) % 2) == 0;
        DrawPlayerShip(g->playerX + shake.x, PLAYER_BASE_Y + shake.y, blink);
    }

    // 8. Particles
    DrawParticles(g->particles);

    // 9. Floating score text
    DrawFloatTexts(g->floatTexts);

    // 10. Ground line
    DrawGroundLine(shake);

    // 11. HUD (no shake)
    DrawHUD(g->score, g->wave, g->lives);

    // 12. Overlay text
    if (g->waveComplete) {
        const char *text = "WAVE COMPLETE";
        int tw = MeasureText(text, 40);
        DrawText(text, SCREEN_W / 2 - tw / 2 + 2, SCREEN_H / 2 - 20 + 2, 40, Fade(COL_UI_CYAN, 0.3f));
        DrawText(text, SCREEN_W / 2 - tw / 2, SCREEN_H / 2 - 20, 40, WHITE);
    }

    if (g->dead) {
        const char *text = "GAME OVER";
        int tw = MeasureText(text, 40);
        DrawText(text, SCREEN_W / 2 - tw / 2 + 2, SCREEN_H / 2 - 20 + 2, 40, Fade((Color){ 255, 50, 50, 255 }, 0.3f));
        DrawText(text, SCREEN_W / 2 - tw / 2, SCREEN_H / 2 - 20, 40, (Color){ 255, 50, 50, 255 });
    }
}

bool GameShouldEnd(GameState *g)
{
    return g->dead && g->deathDelayT <= 0;
}
