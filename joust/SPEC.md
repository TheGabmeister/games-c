# Joust Specification

## Goal

Build a modernized 2D arcade game inspired by the 1982 arcade game Joust using this raylib C template. The game should capture the core feel of mounted aerial dueling: flap-driven flight, platform arenas, enemies that can be defeated only by colliding from above, collectible eggs, escalating waves, and quick restarts.

All art, sound, level layouts, text, and presentation must be original to this project. Do not copy original arcade ROM assets, sprites, audio, exact level data, branding screens, or code. Do not use other projects in this repository as implementation reference.

## Target Experience

- Fast single-screen arcade action.
- A readable 4:3 arena at `1200x900`, closer to the original arcade presentation.
- Sprite-based characters and props with crisp silhouettes.
- Simple controls that are easy to learn and hard to master.
- Short waves with escalating enemy counts, faster hazards, and more aggressive AI.
- Runs from the existing CMake/raylib template with assets loaded from `assets/...` at runtime.

## Scope

First complete version:

- Single-player only.
- Keyboard controls required.
- Gamepad controls optional but planned.
- One fixed arena layout.
- Original sprites and generated sounds.
- Session high score only.

Out of scope for the first complete version:

- Network play.
- Online leaderboards.
- Exact recreation of original level layouts, graphics, names, audio, or ROM behavior.
- Advanced menus, settings screens, or save profiles.

## Resolution And Coordinates

Use a fixed virtual resolution that matches the window size.

```c
#define WINDOW_WIDTH   1200
#define WINDOW_HEIGHT  900
#define TARGET_FPS     60
```

All gameplay positions, platform rectangles, UI coordinates, and collisions use this coordinate space directly. The camera is fixed. Do not add scrolling for the first complete version.

If fullscreen or resizing is added later, render the game to a `RenderTexture2D` at `1200x900`, then letterbox/pillarbox it to the real window while preserving the 4:3 aspect ratio.

## Core Game Loop

1. Show title screen.
2. Start game with player lives, score, and wave 1.
3. Spawn player and enemies into a fixed arena.
4. Player flaps, steers, lands on platforms, and jousts enemies.
5. Defeated enemies drop eggs.
6. Player collects eggs for points before they hatch.
7. When all enemies and eggs are cleared, advance to the next wave.
8. If the player loses all lives, show game over and allow restart.

## Game States

Use a small enum in `game.h`.

```c
typedef enum GameMode {
    GAME_MODE_TITLE,
    GAME_MODE_PLAYING,
    GAME_MODE_PAUSED,
    GAME_MODE_WAVE_CLEAR,
    GAME_MODE_PLAYER_DEAD,
    GAME_MODE_GAME_OVER
} GameMode;
```

State behavior:

- `TITLE`: animated background, start prompt, high score.
- `PLAYING`: normal simulation.
- `PAUSED`: draw frozen gameplay with pause overlay; `P` or gamepad start resumes.
- `WAVE_CLEAR`: short pause, score bonus, then next wave.
- `PLAYER_DEAD`: short pause, respawn if lives remain.
- `GAME_OVER`: final score, restart prompt.

Use a `mode_timer` float for timed states instead of scattering independent timers through the code.

## Controls

Keyboard:

- `Left` / `A`: steer left.
- `Right` / `D`: steer right.
- `Space` / `W` / `Up`: flap.
- `Enter`: start or restart.
- `P`: pause.
- `Esc`: quit through raylib window behavior.

Gamepad, if available:

- Left stick / D-pad: steer.
- South face button: flap.
- Start: start or pause.

Input should be edge-triggered for start, pause, and debug actions. Flap should be edge-triggered with a cooldown, not continuous while held.

## Player Mechanics

The player rides a flying mount and uses physics-based movement.

- Horizontal control applies acceleration, not direct position changes.
- Flap applies upward impulse with a short cooldown.
- Gravity constantly pulls downward.
- Air drag limits top speed.
- Ground/platform contact allows stable landing.
- Horizontal screen edges wrap.
- Falling below the arena bottom costs a life unless landing on a safe floor/lava rule says otherwise.

Suggested constants:

```c
#define PLAYER_RADIUS          18.0f
#define PLAYER_FLAP_VELOCITY  -430.0f
#define PLAYER_GRAVITY        900.0f
#define PLAYER_ACCEL          850.0f
#define PLAYER_MAX_SPEED_X    260.0f
#define PLAYER_MAX_SPEED_Y    620.0f
#define PLAYER_AIR_DRAG       0.92f
#define PLAYER_FLAP_COOLDOWN  0.12f
#define PLAYER_INVULN_TIME     2.0f
```

Clamp `GetFrameTime()` to a maximum of `1.0f / 30.0f` before simulation so a dragged window or debug break does not launch actors through platforms.

## Enemy Mechanics

Enemies are rival riders with the same broad physics rules as the player.

Enemy types:

- `GRUNT`: slow, common, predictable.
- `HUNTER`: faster, tracks player height more aggressively.
- `CHAMPION`: durable-feeling behavior through speed and evasive movement, not hit points.

Enemies should:

- Navigate toward a target height near the player.
- Flap when below their desired height.
- Steer toward the player with limited reaction speed.
- Avoid getting stuck under platforms by periodically choosing side escape targets.
- Wrap horizontally at screen edges.

Enemies are defeated by vertical advantage:

- On rider collision, compare the two entity positions.
- The rider whose mount/body center is higher on the screen wins.
- Require a minimum vertical separation to avoid unfair ties.
- If separation is too small, both riders bounce apart and neither dies.

Suggested collision rule:

```c
float vertical_delta = other.position.y - actor.position.y;
bool actor_wins = vertical_delta > JOUST_WIN_HEIGHT;
```

Suggested constants:

```c
#define ENEMY_RADIUS       18.0f
#define JOUST_WIN_HEIGHT   10.0f
#define JOUST_BOUNCE_SPEED 220.0f
```

If the player is invulnerable after respawn, player-vs-enemy joust collisions should bounce but not kill either rider.

## Eggs

When an enemy is defeated:

- Spawn an egg at the enemy position.
- Egg falls with gravity and lands on platforms.
- Player collects egg on contact for score.
- Egg hatch timer starts when it comes to rest.
- If not collected, egg hatches into a new enemy.

Egg phases:

- `FALLING`
- `RESTING`
- `HATCHING`

Suggested timings:

- Resting before hatch warning: `5.0s`.
- Hatching warning duration: `2.0s`.
- Spawn enemy at hatch completion.

If no enemy slot is free when an egg hatches, keep the egg in `HATCHING` and retry next frame. Do not allocate memory dynamically for overflow cases.

## Hazards

Lava/floor hazard:

- Bottom area is dangerous.
- Touching it kills the player after a small grace threshold.
- Enemies that touch it may bounce upward or respawn depending on wave tuning.

Optional late-wave predator:

- A fast invincible hazard appears when a wave drags too long.
- It chases the player and forces action.
- It should be visually and mechanically distinct from normal enemies.

## Arena

The game uses single-screen arenas built from simple rectangular platforms.

Initial arena:

- Top-left platform.
- Top-right platform.
- Middle center platform.
- Lower-left platform.
- Lower-right platform.
- Hazard band at bottom.

Implementation:

```c
#define MAX_PLATFORMS 12
#define LAVA_HEIGHT   64.0f

typedef struct Platform {
    Rectangle bounds;
} Platform;
```

Initial platform rectangles:

```c
static const Platform INITIAL_PLATFORMS[] = {
    { {  90, 230, 260, 28 } },
    { { 850, 230, 260, 28 } },
    { { 430, 420, 340, 28 } },
    { { 120, 640, 300, 28 } },
    { { 780, 640, 300, 28 } }
};
```

Lava rectangle:

```c
Rectangle lava = { 0, WINDOW_HEIGHT - LAVA_HEIGHT, WINDOW_WIDTH, LAVA_HEIGHT };
```

Platform behavior:

- Characters collide only when falling or standing.
- Characters can jump/fly up through platforms from below.
- Landing sets vertical velocity to `0`.
- Horizontal movement is not blocked by platform sides.
- Platforms are terrain, not decorative sprites; gameplay rectangles are authoritative even if the drawn sprite has transparent pixels.

## Scoring

Base values:

- Defeat grunt: `500`.
- Defeat hunter: `750`.
- Defeat champion: `1000`.
- Collect egg: `250`.
- Wave clear bonus: `1000 + wave * 250`.

Combo:

- Consecutive enemy defeats before touching ground increase a small multiplier.
- Reset multiplier when landing, dying, or clearing a wave.

High score:

- Keep session high score in memory.
- File persistence is optional for the first complete version.

HUD:

- Top-left: current score.
- Top-center: wave number.
- Top-right: lives and high score.
- During gameplay, HUD should stay outside the main collision-heavy play area as much as possible.

## Lives And Respawn

- Start with `3` lives.
- After death, pause briefly and respawn at center-top or a safe platform.
- Give `2.0s` of spawn invulnerability.
- During invulnerability, player sprite flickers.
- Enemies remain active unless wave balance later requires clearing nearby threats.

## Visual Direction

Sprites should be original, readable at game speed, and designed for 20px source cells scaled up cleanly.

Style:

- Modern pixel-art or crisp hand-drawn sprites.
- Dark readable arena background.
- Bright player silhouette.
- Distinct enemy colors per type.
- Clear egg and warning states.
- Minimal UI that does not cover the playfield.

Sprite scale:

- Source sprites may be 20px cells, following the repository sprite workflow.
- Draw gameplay sprites at `3x` or `4x` scale with nearest-neighbor filtering.
- Keep collision circles tuned to gameplay, not to every transparent edge of a sprite.

Required sprite assets in `src/assets/`:

- `player_idle.png`
- `player_flap_1.png`
- `player_flap_2.png`
- `enemy_grunt.png`
- `enemy_hunter.png`
- `enemy_champion.png`
- `egg.png`
- `egg_hatching.png`
- `platform.png`
- `lava.png`
- `spark.png` or particle sprite

Keep source SVGs next to generated PNGs when sprites are created through the SVG-to-PNG workflow.

Texture loading:

- Missing textures must not crash the game.
- Track texture availability in a small asset struct.
- Use primitive fallback drawing for any missing texture.
- Unload every loaded texture during shutdown.

## Audio

Use short generated WAV sounds. Loading should remain resilient through `sounds.c`.

Required sounds:

- `flap.wav`
- `joust_win.wav`
- `joust_bounce.wav`
- `egg_collect.wav`
- `egg_hatch.wav`
- `player_die.wav`
- `wave_clear.wav`
- `start.wav`

Optional sounds:

- `lava.wav`
- `predator.wav`
- `pause.wav`

Sound IDs:

```c
typedef enum SoundID {
    SOUND_FLAP,
    SOUND_JOUST_WIN,
    SOUND_JOUST_BOUNCE,
    SOUND_EGG_COLLECT,
    SOUND_EGG_HATCH,
    SOUND_PLAYER_DIE,
    SOUND_WAVE_CLEAR,
    SOUND_START,
    SOUND_LAVA,
    SOUND_PREDATOR,
    SOUND_PAUSE,
    SOUND_COUNT
} SoundID;
```

All sound files should be runtime-tolerant during development. If a file is missing, mark it unloaded and skip playback.

## Data Model

Keep fixed-size arrays in `Game` for predictable arcade gameplay.

```c
#define MAX_ENEMIES    16
#define MAX_EGGS       16
#define MAX_PARTICLES  256
#define MAX_PLATFORMS  12

typedef struct Actor {
    Vector2 previous_position;
    Vector2 position;
    Vector2 velocity;
    float radius;
    float flap_cooldown;
    float invuln_timer;
    int facing;
    bool alive;
    bool grounded;
} Actor;

typedef enum EnemyType {
    ENEMY_GRUNT,
    ENEMY_HUNTER,
    ENEMY_CHAMPION
} EnemyType;

typedef struct Enemy {
    Actor actor;
    EnemyType type;
    float think_timer;
    float target_y;
    float spawn_grace_timer;
} Enemy;

typedef enum EggState {
    EGG_FALLING,
    EGG_RESTING,
    EGG_HATCHING
} EggState;

typedef struct Egg {
    Vector2 position;
    Vector2 velocity;
    EggState state;
    float timer;
    EnemyType hatch_type;
    bool active;
} Egg;
```

Texture data:

```c
typedef enum TextureID {
    TEXTURE_PLAYER_IDLE,
    TEXTURE_PLAYER_FLAP_1,
    TEXTURE_PLAYER_FLAP_2,
    TEXTURE_ENEMY_GRUNT,
    TEXTURE_ENEMY_HUNTER,
    TEXTURE_ENEMY_CHAMPION,
    TEXTURE_EGG,
    TEXTURE_EGG_HATCHING,
    TEXTURE_PLATFORM,
    TEXTURE_LAVA,
    TEXTURE_SPARK,
    TEXTURE_COUNT
} TextureID;

typedef struct GameTexture {
    Texture2D texture;
    bool loaded;
} GameTexture;
```

Core `Game` fields:

```c
typedef struct Game {
    GameMode mode;
    float mode_timer;
    Actor player;
    Enemy enemies[MAX_ENEMIES];
    Egg eggs[MAX_EGGS];
    Platform platforms[MAX_PLATFORMS];
    int platform_count;
    GameTexture textures[TEXTURE_COUNT];
    Sound sounds[SOUND_COUNT];
    int score;
    int high_score;
    int lives;
    int wave;
    int combo;
    bool debug_draw;
    bool sounds_loaded;
} Game;
```

The final structs can differ, but should stay simple, contiguous, and easy to scan.

## Suggested File Organization

Keep the first implementation compact. Split only when the code is clearly getting large.

- `src/main.c`: window/audio lifecycle and main loop.
- `src/game.h`: constants, enums, structs, public game functions.
- `src/game.c`: state transitions, update, collision, spawning, drawing.
- `src/sounds.h` / `src/sounds.c`: sound IDs, loading, playback, unload.
- `src/assets.h` / `src/assets.c`: texture IDs, loading, fallback helpers, unload.

Possible later split:

- `src/particles.h` / `src/particles.c`: particles if the effects code grows.

## Rendering Details

- Use `Texture2D` for all gameplay sprites.
- Draw sprites centered on actor positions.
- Flip sprites horizontally based on `facing`.
- Use simple frame timers for flap animation.
- Set loaded sprite textures to nearest-neighbor filtering.
- Use circles/rectangles only as debug fallback when textures are missing.
- Keep the camera fixed; no scrolling.
- Use particles for collisions, egg collection, lava contact, and wave clear.

## Collision Details

Per-frame order:

1. Read input.
2. Handle state transitions and pause/title/game-over inputs.
3. Store previous actor and egg positions.
4. Update player physics.
5. Update enemies.
6. Update eggs.
7. Resolve platform landings.
8. Resolve rider-vs-rider collisions.
9. Resolve player-vs-egg collection.
10. Resolve hazards.
11. Check wave clear/death transitions.

Collision shapes:

- Actors: circles.
- Eggs: small circles.
- Platforms: rectangles.
- Lava: rectangle band.

For platform landing:

- Track previous position.
- If actor was above platform top last frame and is falling this frame, snap to platform top.
- Do not block from below.
- Add a small landing tolerance, around `6px`, to reduce missed landings at high speed.

For wrapping:

- Wrap actors and eggs horizontally when their center moves beyond `-radius` or `WINDOW_WIDTH + radius`.
- Preserve velocity during wrapping.

## Wave Design

Wave progression should be deterministic and easy to tune.

Example:

- Wave 1: 3 grunts.
- Wave 2: 4 grunts.
- Wave 3: 3 grunts, 1 hunter.
- Wave 4: 4 grunts, 2 hunters.
- Wave 5+: increase count up to `MAX_ENEMIES`, then replace some grunts with hunters/champions.

Spawn rules:

- Avoid spawning directly on the player.
- Prefer upper and side spawn positions.
- Give newly spawned enemies a short non-colliding grace period if needed.
- If an egg hatches, spawned enemy type should match `hatch_type`.
- Wave clear requires no active enemies and no active eggs.

Suggested spawn points:

```c
static const Vector2 ENEMY_SPAWN_POINTS[] = {
    { 160, 150 },
    { 1040, 150 },
    { 260, 520 },
    { 940, 520 },
    { 600, 320 }
};
```

## Debug Tools

Debug keys can be compiled in always for now:

- `F1`: toggle debug collision shapes.
- `F2`: spawn grunt.
- `F3`: clear wave.
- `F4`: kill player.

Debug drawing:

- Actor circles.
- Platform bounds.
- Egg timers.
- Current wave and active counts.

Debug actions should be ignored on title and game-over screens unless they make sense for setup testing.

## Milestones

### Milestone 1: Playable Movement

- Change window constants to `1200x900`.
- Replace template circle movement with flap physics.
- Add platform collision.
- Add horizontal wrapping.
- Add placeholder sprite loading/fallback drawing.

### Milestone 2: Basic Jousting

- Add enemies.
- Add enemy AI.
- Add rider collision win/bounce/death rules.
- Add score and lives.

### Milestone 3: Eggs And Waves

- Add egg drops, collection, hatch timers.
- Add wave setup and clear transitions.
- Add title and game-over screens.

### Milestone 4: Presentation

- Add original sprites.
- Add generated sounds.
- Add particles and simple animations.
- Add HUD polish and pause overlay.
- Tune constants for game feel.

### Milestone 5: Polish

- Add gamepad support.
- Add high score persistence if desired.
- Balance waves and hazards.

## Acceptance Criteria

The first complete version is done when:

- The game builds with `cmake --build build`.
- The executable runs from `build/template/Debug/`.
- The game window is `1200x900`.
- Player can start, fly, land, wrap, defeat enemies, collect eggs, lose lives, clear waves, and restart after game over.
- Missing assets or sounds use fallback rendering or skipped playback rather than crashing during development.
- No external project code, assets, or layouts are copied.
- Gameplay is understandable without instructions beyond the visible title/start prompts and standard controls.
