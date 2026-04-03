# Galaxian - Modernized Clone Specification

## Overview

This project is a modernized Galaxian remake built on the existing SDL3 template.
The goal is to preserve the classic fixed-shooter loop and enemy attack patterns
while presenting them with a cleaner neon-vector look, stronger screen effects,
and tighter presentation than the 1979 original.

Version 1 scope:

- Single-player keyboard controls only
- Primitive-rendered gameplay entities and effects
- Menu, gameplay, and game-over flow
- Session-only high score tracking
- No authored audio in the first milestone

Version 1 non-goals:

- Online leaderboards
- Save files or persistent profile data
- Power-ups, weapon upgrades, or alternate ships
- Co-op / versus play
- Texture-based ship or enemy art

Technical assumptions:

- Window resolution stays `600 x 800` in portrait orientation
- Gameplay visuals use primitive drawing only; UI text may use `SDL_ttf`
- Rendering helpers should live in `drawing.c/.h`; gameplay code should not
  scatter direct SDL rendering calls across state files
- The audio module may still be initialized by the app shell, but gameplay
  should not depend on audio assets in this milestone

## Design Pillars

- Keep the Galaxian risk/reward loop: enemies hold formation, then peel off
  into dangerous dives that are worth more points while in flight
- Readability first: bullets, hit reactions, convoy attacks, and score events
  must remain easy to parse even with glow-heavy visuals
- Modern presentation, classic rules: update the art direction and transitions
  without changing the one-shot weapon, grounded movement, or stage flow
- Small-engine-friendly implementation: data-oriented gameplay code, minimal
  global state, and clean integration with the existing wrapper modules

## Game States

Use the existing `game_state` system.

| ID | State | Purpose |
|----|-------|---------|
| `STATE_MENU` | Title Screen | Animated logo, attract background, controls, session high score |
| `STATE_GAMEPLAY` | Main Game | Stage intro, active play, respawn handling, stage clear flow |
| `STATE_GAME_OVER` | Game Over | Final score, new high score callout, restart prompt |

Notes:

- Add `STATE_GAME_OVER` to `GameStateID`
- Keep `STATE_PAUSE` reserved in the enum for later, but do not implement it in v1
- `main.c` should register all three active states and start in `STATE_MENU`

## Controls

| Key | Action |
|-----|--------|
| Left Arrow / A | Move left |
| Right Arrow / D | Move right |
| Space | Fire |
| Enter | Start game / Restart from game over |
| Escape | Return to title from gameplay, quit app from menu/game over |

Control notes:

- The player moves only on the X axis
- Firing is edge-triggered; holding `Space` should not autofire
- If the player returns to title from gameplay, the current run is abandoned

## Playfield Layout

Screen layout constants:

- Window: `600 x 800`
- Top HUD band: `48 px`
- Bottom HUD band: `48 px`
- Playfield bounds: `x = 24..576`, `y = 72..752`
- Player spawn position: centered horizontally, `y = 724`
- Player horizontal margin from walls: `24 px`

Formation layout:

- Logical formation grid: `10 columns x 6 rows`
- Horizontal slot spacing: `40 px`
- Vertical slot spacing: `34 px`
- Formation anchor: centered on screen, top row starting near `y = 120`
- The formation itself sways horizontally by about `+/- 28 px`
- Sway is smooth and periodic, not stepped like Space Invaders

Classic-inspired row occupancy for each new stage:

| Row | Occupied Columns | Enemy Type | Count |
|-----|------------------|------------|-------|
| 0 | 4-5 | Flagship | 2 |
| 1 | 2-7 | Escort | 6 |
| 2 | 1-8 | Raider | 8 |
| 3 | 0-9 | Drone | 10 |
| 4 | 0-9 | Drone | 10 |
| 5 | 0-9 | Drone | 10 |

Total starting enemies per stage: `46`

## Core Gameplay

### Player

- Starts with `3` lives
- Moves at `320 px/s`
- Can have only one player bullet active at a time
- Respawns after a `0.9s` delay if lives remain
- Gains `2.0s` of invulnerability after respawn
- Flashes while invulnerable and cannot fire during the respawn delay
- Collision uses a forgiving hitbox smaller than the rendered ship silhouette

### Enemy Types

Use four enemy classes so the remake preserves the original scoring ladder and
formation structure.

| Type | Formation Rows | Score In Formation | Score In Flight | Visual Direction |
|------|----------------|--------------------|-----------------|------------------|
| `Drone` | Rows 3-5 | 30 | 60 | Small cyan-blue diamond |
| `Raider` | Row 2 | 40 | 80 | Angular violet kite |
| `Escort` | Row 1 | 50 | 100 | Red hexagon with bright core |
| `Flagship` | Row 0 | 60 | 150 / convoy bonus | Gold ring with crown lines |

Flagship convoy scoring:

- `200` if the flagship is destroyed while diving with one escort
- `300` if the flagship is destroyed while diving with two escorts before both escorts die
- `800` if the flagship is destroyed after both escorts in a two-escort convoy have already been destroyed

### Enemy Formation Rules

- Enemies return to their original home slot after a completed dive
- Solo dives should prefer enemies on the outer edges of currently occupied rows
- Flagships may launch convoy attacks with up to two living escorts taken from the nearest valid escort slots
- Only one flagship-led convoy may be active at a time
- Enemies in formation continue their idle shimmer while the whole formation sways
- If a row has gaps, home slots remain fixed; surviving enemies do not collapse inward

### Dive Rules

- First attack begins about `1.25s` after the stage intro ends
- Diving enemies follow authored cubic Bezier paths
- Paths are mirrored left/right variants with small deterministic offsets so they
  feel alive without becoming random noise
- Drones and Raiders mostly dive solo
- Escorts may dive solo or as a synchronized pair
- Flagships strongly prefer convoy dives when escorts are available
- Diving enemies may fire only while above a soft lower firing line near `y = 680`
- Surviving divers exit off-screen and return from above to rejoin formation

Suggested path families:

- `HookLeft`: peel left, arc down, cross inward
- `HookRight`: mirrored version of `HookLeft`
- `CenterS`: shallow S-curve through center lane
- `WideLoop`: broader path for flagship convoy attacks
- `ReturnArc`: re-entry curve from above the top HUD into the enemy's home slot

Recommended dive durations:

- Drone: `1.8s`
- Raider: `2.0s`
- Escort: `2.0s`
- Flagship convoy: `2.3s`

### Swarm / Endgame Behavior

Late-stage cleanup should feel more aggressive.

- Trigger swarm mode when `3` or fewer enemies remain, or when all Drones and Raiders are gone
- In swarm mode, attack cadence becomes much faster and enemies stop lingering in formation
- Swarm mode ends when the stage ends or the player loses a life
- After a player death, surviving enemies should return to a stable formation state before play resumes

### Bullets

Player bullet:

- Speed: `620 px/s` straight upward
- Width/height target: roughly `4 x 16 px`
- Despawns on enemy hit or when leaving the top of the screen

Enemy bullets:

- Max simultaneous enemy bullets: `6`
- Speed is stage-dependent
- Fired with slight X velocity based on player position at fire time
- No homing after launch
- Cleared when the player loses a life or when a stage ends

### Collision Rules

- Use AABB collision for all gameplay interactions
- Player ship hitbox should be about `70%` of the rendered ship width
- Enemy hitboxes should be slightly inset from their glow layers
- Player bullets can hit diving or in-formation enemies
- Enemy bullets never collide with other enemies
- The player dies on contact with an enemy body or an enemy bullet

### Lives, Extra Life, and Game Over

- One bonus life at `7000` points
- Losing a life does not reset the stage; remaining enemies stay dead
- On player death, clear active enemy bullets, trigger explosion/screen shake,
  then begin respawn if lives remain
- If no lives remain, transition to `STATE_GAME_OVER` after the death sequence
- Session high score updates immediately when the score exceeds it

### Stage Flow

Per-stage loop:

1. Show `STAGE N` overlay for about `1.0s`
2. Spawn the full formation in its home slots
3. Run attack/formation gameplay
4. Trigger a short clear delay when all enemies are destroyed
5. Advance to the next stage with harder parameters

Run reset rules:

- Starting a new run resets score, lives, stage number, formation, bullets, and particles
- Session high score survives until the application closes

## Difficulty Progression

| Stage | Attack Interval | Enemy Bullet Speed | Max Active Divers | Bullet Pattern |
|-------|-----------------|--------------------|-------------------|----------------|
| 1 | 3.00s | 170 px/s | 2 | Mostly single shots |
| 2 | 2.55s | 185 px/s | 2 | Some double-shot dives |
| 3 | 2.20s | 200 px/s | 3 | Double shots common |
| 4 | 1.90s | 220 px/s | 3 | Faster escorts and convoys |
| 5+ | 1.65s | 240 px/s | 4 | Aggressive swarm cleanup |

Scaling rules:

- Clamp the table at stage 5 values
- Also shorten the delay before the first attack by `0.05s` per stage down to a floor of `0.8s`
- Slightly increase formation sway speed after stage 3

## Visual Direction

### Aesthetic

The remake should look like a neon-arcade cabinet interpretation of Galaxian:
sharp line work, bold silhouettes, layered glow, and restrained particle use.
The screen must remain readable at gameplay speed.

### Palette

| Element | Primary | Secondary / Glow |
|---------|---------|------------------|
| Background | `#090B16` | `#11162A` |
| Player | `#4DF6FF` | `#00C8FF` |
| Drone | `#4A8DFF` | `#8CC7FF` |
| Raider | `#9B6BFF` | `#C8A7FF` |
| Escort | `#FF4A5E` | `#FF9A8A` |
| Flagship | `#FFD44D` | `#FFF1A8` |
| Player Bullet | `#FFFFFF` | `#7DF9FF` |
| Enemy Bullet | `#FF7A3D` | `#FFC16E` |
| HUD Text | `#F5F7FF` | `#7AA2FF` |

### Rendering Rules

Gameplay state code should rely on helper functions in `drawing.c/.h`. If the
current helpers are not enough, extend that module with primitives such as:

- `draw_triangle_outline(...)`
- `draw_polygon_outline(...)`
- `draw_ring(...)`
- `draw_glow_circle(...)`

Avoid calling raw SDL render APIs directly from `state_gameplay.c` except for
temporary debugging.

### Entity Readability

Player ship:

- Main silhouette is a triangle with two short wing bars
- Thruster glow flickers subtly while alive
- Invulnerability flash alternates visible/hidden at about `12 Hz`

Enemy silhouettes:

- Drone: diamond outline plus faint inner core
- Raider: narrow four-point kite
- Escort: thicker hex outline with a center dot
- Flagship: circular core, inner ring, short crown rays

Bullets:

- Player bullet should be brighter and slimmer than enemy bullets
- Enemy bullets should have a small tail or spark so their travel direction is obvious

### Effects

Background:

- Three-layer starfield with slow downward drift
- About `80` total stars across all layers
- Stars wrap vertically

Particles:

- Enemy death: `12-20` particles based on enemy type
- Flagship death: `24-32` particles and a larger glow burst
- Player death: strongest particle burst plus white-cyan flash

Camera / screen feedback:

- `0.25s` decaying screen shake on player death
- Tiny `0.08s` nudge on flagship destruction
- Menu and game-over prompts pulse by alpha, not scale

## HUD And Screen Flow

Top HUD:

- Left: `SCORE`
- Right: `HI SCORE`
- Use zero-padded numeric formatting for score display

Bottom HUD:

- Left: remaining lives as miniature ship icons
- Right: current stage number

Menu screen:

- Title logo centered in upper third
- Small attract-mode formation or drifting enemies behind the title
- Session high score shown prominently
- One-line controls hint near the bottom

Game-over screen:

- `GAME OVER` headline
- Final score
- High score or `NEW HIGH SCORE` callout when relevant
- `PRESS ENTER TO PLAY AGAIN`

## Implementation Structure

Recommended new source files:

| File | Purpose |
|------|---------|
| `src/galaxian.h` | Shared constants, enums, and gameplay structs |
| `src/galaxian.c` | Formation setup, attack selection, path generation, difficulty helpers |
| `src/state_menu.c` | Title screen logic and rendering |
| `src/state_gameplay.c` | Main gameplay state |
| `src/state_gameover.c` | Game-over screen logic |
| `src/particles.h` | Particle system interface |
| `src/particles.c` | Particle storage, update, and draw |

Also update:

- `src/main.c` to register the new states and start at the menu
- `src/game_state.h` to add `STATE_GAME_OVER`
- `src/drawing.h` and `src/drawing.c` if extra primitive helpers are needed
- `CMakeLists.txt` to compile every new `.c` file

## Data Model

Keep score/session state separate from the player entity. The original draft put
score, stage, and high score inside `Player`, which makes ownership muddy.

```c
// galaxian.h

#define MAX_ENEMIES       46
#define MAX_PARTICLES     768
#define MAX_ENEMY_BULLETS 6
#define FORMATION_COLS    10
#define FORMATION_ROWS    6

typedef enum {
    ENEMY_DRONE,
    ENEMY_RAIDER,
    ENEMY_ESCORT,
    ENEMY_FLAGSHIP
} EnemyType;

typedef enum {
    ENEMY_IN_FORMATION,
    ENEMY_DIVING,
    ENEMY_RETURNING,
    ENEMY_SWARMING,
    ENEMY_DEAD
} EnemyState;

typedef enum {
    BULLET_NONE,
    BULLET_PLAYER,
    BULLET_ENEMY
} BulletOwner;

typedef struct {
    vector2 position;
    float speed;
    bool alive;
    bool invulnerable;
    float invulnerable_timer;
    float respawn_timer;
} Player;

typedef struct {
    EnemyType type;
    EnemyState state;
    int slot_row;
    int slot_col;
    vector2 home_position;
    vector2 position;
    float path_t;
    float path_duration;
    float shimmer_phase;
    int shots_remaining;
    bool alive;
    bool reserved_for_convoy;
} Enemy;

typedef struct {
    vector2 position;
    vector2 velocity;
    BulletOwner owner;
    bool active;
} Bullet;

typedef struct {
    vector2 position;
    vector2 velocity;
    float life;
    float max_life;
    float size;
    SDL_Color color;
    bool active;
} Particle;

typedef struct {
    Player player;
    Enemy enemies[MAX_ENEMIES];
    Bullet player_bullet;
    Bullet enemy_bullets[MAX_ENEMY_BULLETS];
    Particle particles[MAX_PARTICLES];
    int score;
    int high_score;
    int stage;
    int lives;
    bool extra_life_awarded;
    bool swarm_mode;
    float stage_intro_timer;
    float stage_clear_timer;
    float attack_timer;
    float first_attack_delay;
    int active_divers;
} GameplayStateData;
```

## Acceptance Criteria

- The game boots into a menu state instead of immediately entering gameplay
- A full run can progress through multiple stages and reach game over without crashes
- Stage setup creates a `46`-enemy formation with the row occupancy described above
- The player can never have more than one bullet active at a time
- Convoy dives, swarm behavior, scoring, and extra-life rules match this spec
- All gameplay ships, enemies, bullets, particles, and HUD embellishments render
  without texture assets, except for font rendering
- Session high score persists between runs until the executable closes
