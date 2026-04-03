# Galaxian — Modernized Clone Specification

## Overview

A modernized Galaxian clone built with SDL3 primitive rendering (no sprites or textures). Same core mechanics as the 1979 arcade original — fixed-formation enemies, dive-bombing attacks, single-bullet shooting — but with a **neon-geometry aesthetic** inspired by games like Geometry Wars. All visuals are rendered using `draw_rect`, `draw_circle`, `draw_line`, `draw_rect_outline`, and direct SDL render calls for triangles/polygons.

**Resolution:** 600 x 800 (portrait, matching the existing window config)
**No audio.** Audio module is initialized but unused.

---

## Game States

Using the existing `game_state` system:

| ID | State | Purpose |
|----|-------|---------|
| `STATE_MENU` | Title Screen | Animated logo, "PRESS ENTER TO START", high score display |
| `STATE_GAMEPLAY` | Main Game | The Galaxian gameplay loop |
| `STATE_GAME_OVER` | Game Over | Final score, high score update, "PRESS ENTER TO RESTART" |

Add `STATE_GAME_OVER` to the `GameStateID` enum.

---

## Gameplay Mechanics

### Player
- Positioned near the bottom of the screen (y ≈ 720)
- Moves left/right with **Arrow Keys** or **A/D**
- Fires with **Space** (single bullet on screen at a time — faithful to original)
- 3 lives at start; lost when hit by an enemy or enemy bullet
- Brief invincibility flash (~2 seconds) after respawning
- Clamped to screen bounds with a small margin

### Enemies

Three types, arranged in a grid formation near the top:

| Type | Rows | Points | Color | Shape |
|------|------|--------|-------|-------|
| **Drone** | 2 bottom rows (rows 4-5) | 30 | Blue | Small diamond |
| **Escort** | 2 middle rows (rows 2-3) | 60 | Red | Hexagon |
| **Flagship** | 1 top row (row 1) | 150 (solo) / 300 (with escorts) | Yellow/Gold | Large circle with inner ring |

**Formation:** 10 columns x 5 rows = 50 enemies per stage. The entire formation drifts left and right in a slow sinusoidal sway.

**Dive attacks:**
- Enemies periodically peel off from the formation and dive toward the player in **curved Bézier paths**
- Drones dive solo
- Escorts dive solo or in pairs
- Flagships dive with 0-2 escorts flanking them (bonus points if flagship killed while escorted)
- Diving enemies fire 1-2 bullets during their run
- After a dive, surviving enemies loop back up and rejoin the formation
- Dive frequency increases as fewer enemies remain and as stages progress

### Bullets
- **Player bullet:** Travels straight up. Only 1 on screen at a time.
- **Enemy bullets:** Travel downward with slight tracking toward the player's x-position. Max 4-6 on screen at once.

### Collision
- Rectangle vs. rectangle AABB for simplicity (hitboxes slightly smaller than visual shapes for fairness)

### Scoring & Progression
- Points per enemy kill (see table above)
- Flagship bonus: 300 if killed while diving with escorts, 150 if solo
- Stage clears when all 50 enemies are destroyed
- Next stage: enemies dive sooner, fire more often, bullets slightly faster
- **High score** persisted in memory for the session (no file I/O needed)

### Lives & Game Over
- Start with 3 lives, displayed as small ship icons in the HUD
- Extra life at 10,000 points (one-time)
- Game over when lives reach 0 → transition to `STATE_GAME_OVER`

---

## Visual Design

### Aesthetic: Neon Geometry

Everything is rendered with primitive shapes, lines, and circles. The look is clean vector-style graphics with glow effects achieved through layered semi-transparent shapes.

### Color Palette

| Element | Primary | Glow/Trail |
|---------|---------|------------|
| Background | Near-black (#0A0A1A) | — |
| Player | Bright cyan (#00FFFF) | Cyan at 30% alpha |
| Drones | Electric blue (#4488FF) | Blue at 30% alpha |
| Escorts | Hot red (#FF3344) | Red at 30% alpha |
| Flagships | Gold (#FFDD00) | Yellow at 30% alpha |
| Player bullet | White (#FFFFFF) | Cyan at 40% alpha |
| Enemy bullets | Orange-red (#FF6633) | Red at 40% alpha |
| Explosions | White → Yellow → Orange → Red (fading) | — |
| Stars | White, dim gray, pale blue (randomized) | — |
| UI Text | White | — |

### Entity Rendering (All Primitive Shapes)

**Player ship:**
- Main body: upward-pointing triangle drawn with 3 `draw_line` calls
- Two small "wing" rectangles on the sides
- A "thruster glow": 2-3 small circles at the base with decreasing alpha (animated flicker)
- Glow outline: same triangle shape drawn slightly larger at low alpha

**Drones:**
- Small rotated square (diamond) drawn with 4 `draw_line` calls
- Faint glow outline at low alpha

**Escorts:**
- Hexagon drawn with 6 `draw_line` calls
- Inner filled circle
- Glow outline

**Flagships:**
- Large filled circle (`draw_circle`)
- Inner ring (`draw_circle` slightly smaller, or `draw_rect_outline` circle approximation)
- Small radiating lines around the circumference (crown/star motif)
- Pulsing glow: outer circle at low alpha that scales up and down

**Player bullet:**
- Tall thin rectangle with a bright core
- Trailing glow: 2-3 fading rectangles behind it

**Enemy bullets:**
- Small filled circle
- Short trailing line behind it

### Visual Effects

**Starfield background:**
- 3 layers of stars (far, mid, near) scrolling downward at different speeds for parallax
- Far: tiny 1px dots, dim. Mid: 2px, medium brightness. Near: 3px, bright.
- ~80 stars total, wrapping around when they scroll off-screen

**Explosions (particle system):**
- On enemy death: burst of 12-20 particles radiating outward from the center
- Each particle: small filled circle or short line, colored based on the enemy type
- Particles have velocity, fade out over 0.4-0.8 seconds, and shrink
- Flagships get a bigger, more dramatic explosion (more particles, wider spread)

**Player death:**
- Larger explosion with white/cyan particles
- Brief screen flash (full-screen white rect at low alpha, fading over 0.2s)

**Thruster trail (player):**
- Small particles emitted from the bottom of the ship each frame
- Drift downward slightly, fade quickly (0.2s lifetime)
- Color: orange → yellow → transparent

**Formation shimmer:**
- Enemies in formation have a subtle brightness oscillation (sine wave offset per row) to make the grid feel alive

**Dive trail:**
- Diving enemies leave a faint trail of dots/small circles along their path
- Trail fades over ~0.5 seconds

**Screen shake:**
- On player death: brief 0.3s screen shake (random x/y offset on the camera/render offset, decaying)

**UI pulse:**
- "PRESS ENTER" text blinks via alpha oscillation (sine wave)

---

## HUD Layout

```
┌──────────────────────────────────────┐
│  SCORE: 003250        HI: 012400     │  ← top bar, white text
│                                      │
│         [enemy formation]            │
│                                      │
│                                      │
│                                      │
│              [play area]             │
│                                      │
│                                      │
│            [player ship]             │
│  ♦ ♦ ♦                  STAGE 3      │  ← bottom bar: lives + stage
└──────────────────────────────────────┘
```

- Score and high score at the top (left and right aligned)
- Lives as small ship silhouettes (miniature triangles) at bottom-left
- Current stage number at bottom-right

---

## File Structure

New source files to create:

| File | Purpose |
|------|---------|
| `src/galaxian.h` | Shared constants, structs (Player, Enemy, Bullet, Particle), enums, formation config |
| `src/galaxian.c` | Core game data: formation layout, dive path definitions, difficulty curves |
| `src/state_menu.c` | Title screen state |
| `src/state_gameplay.c` | Main gameplay state (update + draw for all entities) |
| `src/state_gameover.c` | Game over screen state |
| `src/particles.h` | Particle system interface |
| `src/particles.c` | Particle spawning, updating, and rendering |

All new `.c` files get added to `CMakeLists.txt`'s `add_executable()`.

---

## Key Data Structures

```c
// In galaxian.h

#define MAX_ENEMIES       50
#define MAX_PARTICLES     512
#define MAX_ENEMY_BULLETS 8
#define FORMATION_COLS    10
#define FORMATION_ROWS    5

typedef enum { ENEMY_DRONE, ENEMY_ESCORT, ENEMY_FLAGSHIP } EnemyType;
typedef enum { ENEMY_IN_FORMATION, ENEMY_DIVING, ENEMY_RETURNING, ENEMY_DEAD } EnemyState;

typedef struct {
    float x, y;
    float vel_x;
    bool alive;
    bool invincible;        // respawn grace period
    float invincible_timer;
    int lives;
    int score;
    int high_score;
    int stage;
} Player;

typedef struct {
    EnemyType type;
    EnemyState state;
    float x, y;              // current position
    float form_x, form_y;    // home position in formation
    float dive_t;            // parametric t along dive Bézier curve (0..1)
    float dive_speed;
    bool alive;
    float shimmer_offset;    // per-enemy brightness oscillation phase
} Enemy;

typedef struct {
    float x, y;
    float vel_y;
    bool active;
    bool is_player;          // true = player bullet, false = enemy bullet
    float vel_x;             // slight tracking for enemy bullets
} Bullet;

typedef struct {
    float x, y;
    float vx, vy;
    float life;              // remaining life in seconds
    float max_life;
    float size;
    SDL_Color color;
    bool active;
} Particle;
```

---

## Dive Path System

Diving enemies follow cubic Bézier curves with 4 control points:

1. **Start:** Enemy's current formation position
2. **Control 1:** Offset to one side (creates the initial swoop)
3. **Control 2:** Near the player's x-position, below mid-screen
4. **End:** Off-screen bottom (or loop-back point)

If the enemy survives the dive (goes off-screen bottom), it wraps around and returns to its formation slot from the top via a gentler curve.

Multiple pre-defined path templates are mirrored and varied randomly:
- **Swoop Left:** Curves left then cuts across toward the player
- **Swoop Right:** Mirror of swoop left
- **Center Dive:** Drops nearly straight down with a slight S-curve
- **Wide Loop:** Sweeps out wide, then loops back — used by flagships

---

## Difficulty Progression

| Stage | Dive Interval | Enemy Bullet Speed | Max Simultaneous Divers | Enemy Bullet Count |
|-------|--------------|-------------------|------------------------|--------------------|
| 1 | 3.0s | 150 px/s | 2 | 1 per dive |
| 2 | 2.5s | 170 px/s | 2 | 1-2 per dive |
| 3 | 2.0s | 190 px/s | 3 | 1-2 per dive |
| 4 | 1.7s | 210 px/s | 3 | 2 per dive |
| 5+ | 1.5s | 230 px/s | 4 | 2 per dive |

Values are clamped at stage 5 levels.

---

## Controls Summary

| Key | Action |
|-----|--------|
| Left Arrow / A | Move left |
| Right Arrow / D | Move right |
| Space | Fire |
| Enter | Start game / Restart |
| Escape | Quit to menu (from gameplay) / Quit application (from menu) |
