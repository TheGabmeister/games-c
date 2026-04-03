# Space Invaders - Design and Implementation Specification

## Goal

Recreate the core 1978 Space Invaders loop in raylib, but present it as a cleaner, more modern arcade game built entirely from primitive shapes. The game should feel immediately familiar: one screen, marching invaders, protective shields, a mystery ship, one player shot at a time, escalating pressure, and score chasing.

This project is desktop-first, keyboard-controlled, and intentionally lightweight:

- No sprites
- No textures
- No custom fonts
- No audio
- No save data

The modernization comes from motion, color, polish, readability, and feel, not from changing the core rules.

---

## Project Pillars

1. **Classic mechanics first**
   Same player constraints, same wave pressure, same shield attrition, same one-shot rhythm.
2. **Modern arcade presentation**
   Neon colors, glow, particles, subtle camera shake, and cleaner UI.
3. **Simple implementation**
   Keep logic deterministic and readable. Prefer small fixed-size pools over dynamic allocation.
4. **Primitive-only rendering**
   All gameplay visuals must be made with raylib draw primitives and the default font.

---

## Target Runtime

- **Window**: `800x600`
- **Frame rate target**: `60 FPS`
- **Window title**: `"Space Invaders"`
- **Background color**: `(10, 10, 30)`
- **Platform target**: desktop first
- **Primary input**: keyboard

The game should still compile in the existing raylib project structure, but the design is not required to support touch controls or audio.

---

## Scope and Non-Goals

### In Scope

- Title screen
- Gameplay screen
- Ending / game over screen
- Wave progression
- Score + high score for current app session
- Shield destruction
- UFO / mystery ship
- Primitive-based effects

### Out of Scope

- Options menu
- Audio or music
- Texture assets
- Persistent save files
- Power-ups
- Alternate weapons
- Multiplayer
- Pause menu

---

## Core Gameplay Summary

- Player moves left/right along the bottom of the screen.
- Player may have only **one active bullet** at a time.
- 55 aliens descend in a `5 x 11` formation.
- Aliens march horizontally, drop when they hit a boundary, and speed up as their count decreases.
- Aliens fire downward from the bottom-most live alien in a chosen column.
- 4 destructible shields absorb fire from both sides.
- A mystery ship occasionally crosses the top of the playfield for bonus points.
- Player starts with `3` lives.
- Game ends when:
  - the player loses the final life, or
  - any live alien reaches the shield line.

---

## Visual Direction

The game should feel like a clean neon cabinet version of Space Invaders.

- **Palette**
  - Background: deep navy `(10, 10, 30)`
  - Player: neon cyan
  - Aliens: magenta / cyan / green by type
  - Enemy fire and explosions: orange / red
  - UI text: white with cyan accent glow
- **Glow**
  Draw key objects twice:
  - solid base shape
  - slightly larger low-alpha pass for bloom-like glow
- **Animation**
  - alien two-frame stepping
  - bullet trails
  - blinking invulnerability
  - pulsing UFO glow
  - floating score text
- **Background**
  Layered starfield with subtle motion
- **Camera feel**
  Light screen shake on impactful events only

Readability matters more than visual excess. The player, bullets, and shield damage must remain easy to parse at a glance.

---

## Screen Flow

The project keeps the existing screen-based architecture, but the flow is simplified:

`LOGO -> TITLE -> GAMEPLAY -> ENDING`

### LOGO

- Keep the existing logo screen behavior unless it breaks due to the other refactors.
- No gameplay-specific changes required.

### TITLE

- Dark background matching gameplay
- Large glowing title: `SPACE INVADERS`
- Decorative row showing the 3 alien types and their point values
- Blinking prompt: `PRESS ENTER TO START`
- High score displayed near the bottom

### GAMEPLAY

- Main game loop
- HUD at top
- Playfield below

### ENDING

- Dark background
- Large glowing `GAME OVER`
- Final score
- High score
- Blinking prompt: `PRESS ENTER TO PLAY AGAIN`

### Removed Screen

- `OPTIONS` is removed entirely
- No code should reference the `OPTIONS` enum or its transition paths after the refactor

---

## Layout

### Global Coordinates

- Window size: `800x600`
- Horizontal play bounds: `x = 20` to `x = 780`
- Decorative ground line: `y = 580`
- HUD band: roughly `y = 8` to `y = 28`
- UFO lane center: `y = 40`

### Entity Anchors

- Player baseline area: `y = 550`
- Shield top: `y = 460`
- Initial alien formation origin: `x = 80`, `y = 70`
- Alien slot spacing: `40px` horizontal, `36px` vertical

### HUD Layout

- Left: `SCORE`
- Center: `WAVE`
- Right: `LIVES`

Lives are shown as small ship icons, not plain numbers.

---

## Gameplay Rules

### Player

- **Shape**: stylized chevron ship from primitives
- **Approx size**: `30px` wide, `20px` tall
- **Movement**: `Left/Right` or `A/D`
- **Speed**: `300 px/s`
- **Bounds**: clamp to the horizontal play area
- **Fire**: `Space`
- **Shot limit**: exactly 1 active player bullet

If the player presses fire while their shot is still active, input is ignored.

### Player Hit / Respawn

- On hit:
  - lose 1 life
  - spawn player explosion particles
  - trigger screen shake
  - clear all active bullets
- If lives remain:
  - player respawns at the same `x` position
  - player becomes invulnerable for `2.0s`
  - ship blinks during invulnerability
  - movement and firing remain allowed during invulnerability
- If lives reach `0`:
  - enter a short death delay of `1.5s`
  - then transition to `ENDING`

Invulnerability only applies after a hit. It does not carry between waves.

### Aliens

- Formation size: `5 rows x 11 columns = 55 aliens`
- Each alien occupies a logical slot in the formation grid
- The formation moves as a single unit

#### Alien Types

- **Rows 0-1**
  - diamond/rhombus variant
  - color: magenta `(255, 0, 180)`
  - score: `30`
- **Rows 2-3**
  - circle with antennae variant
  - color: cyan `(0, 220, 255)`
  - score: `20`
- **Row 4**
  - winged rectangle variant
  - color: green `(0, 255, 100)`
  - score: `10`

#### Alien Animation

- Aliens have 2 visual frames
- The frame toggles each time the formation completes a horizontal step
- The animation should be subtle but clearly visible

#### Alien Movement

- Horizontal step size: `4px`
- Downward drop on edge reversal: `12px`
- Initial direction: right

Base step interval:

`stepInterval = 0.02 + (aliveCount / 55.0) * 0.48`

This yields:

- about `0.50s` per step when all 55 are alive
- about `0.03s` per step near the end of the wave

Wave scaling is applied after that:

`waveSpeedMultiplier = max(0.35, 1.0 - (wave - 1) * 0.10)`

`finalStepInterval = stepInterval * waveSpeedMultiplier`

This fixes an issue in the earlier draft where wave 1 was unintentionally already sped up.

#### Edge Detection

Before each horizontal step, test whether the next step would push any live alien beyond the play bounds.

If yes:

- reverse horizontal direction
- move formation down by `12px`
- do not also apply the blocked horizontal step on that same tick

### Alien Firing

Enemy firing must be **time-based**, not frame-dependent.

Use a continuous fire rate:

`enemyShotRate = 0.30 + (1.0 - aliveCount / 55.0) * 0.90`

This yields roughly `0.3` to `1.2` shots per second over the course of a wave.

Rules:

- max `3` active enemy bullets at once
- when an enemy shot is spawned:
  - choose a random column that still contains at least 1 live alien
  - fire from the bottom-most live alien in that column
- no enemy shots are spawned during wave-complete pause or game-over delay

### Bullets

#### Player Bullet

- Size: `3x10`
- Color: white core with cyan glow
- Speed: `500 px/s`
- Destroyed when:
  - hitting an alien
  - hitting a shield block
  - hitting the mystery ship
  - leaving the top of the screen

#### Enemy Bullet

- Size: `3x10`
- Color: orange/red glow
- Speed: `250 px/s`
- Destroyed when:
  - hitting the player
  - hitting a shield block
  - leaving the bottom of the screen

#### Shared Bullet Rules

- player bullets and enemy bullets do **not** collide with each other
- each bullet stores its previous `3` positions for trail rendering

### Shields

- Count: `4`
- Built from destructible block cells
- Cell size: `6x6`
- Color: green `(0, 200, 80)`

Shield pattern:

```
Row 0:   . # # # # # # .
Row 1:   # # # # # # # #
Row 2:   # # # # # # # #
Row 3:   # # # . . # # #
Row 4:   # # . . . . # #
```

- Grid size: `8 columns x 5 rows`
- Approx shield size: `48 x 30`

Placement:

- evenly distributed across the play area
- all shield tops aligned at `y = 460`

Block behavior:

- a bullet destroys exactly one shield cell on contact
- the hit cell flashes white briefly, then disappears
- the bullet is destroyed on impact
- a destroyed cell never regenerates during the wave
- all 4 shields reset to full when a new wave begins

Implementation note:

Store a small per-cell state:

- intact
- flashing
- destroyed

This avoids ambiguity in the original draft, which required a flash but did not define how that state is represented.

### Mystery Ship

- Max `1` active at a time
- Spawns at a random interval between `15s` and `25s`
- Spawns just off-screen
- Moves horizontally at `150 px/s`
- Alternates direction each appearance:
  - first spawn: left to right
  - second: right to left
  - continue alternating
- Despawns after fully exiting the screen

Score value is chosen at spawn time from:

`{50, 100, 150, 200, 300}`

On hit:

- award the selected score
- destroy the UFO immediately
- spawn floating score text at its position

The mystery ship timer pauses during wave-complete and game-over delays, and resets when a new game begins.

---

## Wave Flow

### New Game

Starting a new run resets:

- score to `0`
- wave to `1`
- lives to `3`
- alien formation
- shields
- UFO timer / direction state
- all bullets, particles, and floating texts

High score persists only for the current app session.

### Start of Wave

Each wave starts with:

- 55 live aliens
- 4 rebuilt shields
- no active bullets
- player active and controllable

Wave start vertical offset:

- base alien origin `y = 70`
- each new wave starts `36px` lower than the previous wave
- cap the formation origin at `y = 250`

The cap prevents impossible or unfair starts while preserving the intended rising pressure.

### Wave Complete

When the last alien is destroyed:

- stop spawning enemy bullets
- despawn any remaining bullets
- despawn active UFO if present
- display `WAVE COMPLETE`
- keep particles and floating texts updating
- wait `2.0s`
- then start the next wave

Score and remaining lives carry over.

### Game Over by Invasion

If any live alien's **bottom edge** reaches the shield line, the run ends immediately.

Use this condition:

`alienBottom >= 460`

This fixes the earlier draft, which mixed `y >= 450` with shields placed at `y = 460`.

---

## Scoring

- Bottom-row alien: `10`
- Middle-row aliens: `20`
- Top-row aliens: `30`
- Mystery ship: random from `{50, 100, 150, 200, 300}`

Rules:

- score updates immediately on kill
- high score updates immediately when exceeded
- final score shown on the ending screen is the run score at time of defeat

---

## Effects

### Particle System

Use a fixed pool of `512` particles.

Each particle stores:

- position
- velocity
- color
- lifetime
- maxLifetime
- size

Per-frame update:

- `position += velocity * dt`
- `lifetime -= dt`
- alpha fades based on remaining lifetime
- size may shrink linearly over lifetime

If the particle pool is full, skip spawning excess particles rather than allocating memory dynamically.

#### Particle Presets

- **Alien death**
  - `15-25` particles
  - alien color
  - speed `80-200 px/s`
  - lifetime `0.3-0.6s`
  - size `2-4`
- **Player death**
  - `30-40` particles
  - white/cyan
  - speed `100-300 px/s`
  - lifetime `0.4-0.8s`
  - size `3-5`
- **Shield block hit**
  - `5-8` particles
  - shield green
  - speed `40-100 px/s`
  - lifetime `0.2-0.4s`
  - size `2-3`
- **Bullet impact**
  - `8-12` particles
  - bullet color
  - speed `60-150 px/s`
  - lifetime `0.15-0.3s`
  - size `1-2`

### Bullet Trails

- store the last `3` positions per bullet
- draw progressively dimmer and smaller copies behind the live bullet
- suggested alpha values: `0.50`, `0.25`, `0.10`

### Screen Shake

Use a gameplay-only shake offset applied during world rendering.

- Player death: amplitude `4px`, duration `0.30s`
- Shield hit: amplitude `2px`, duration `0.10s`

Shake decays linearly over time.

Applying shake to gameplay only keeps the HUD readable while still selling impact.

### Floating Score Text

- Max `8` active texts
- Spawn on alien or UFO kill
- Rise upward at `40 px/s`
- Fade out over `1.0s`
- Color matches defeated target

### Starfield

Use about `100` stars across `3` depth layers:

- near: faster, brighter
- mid: medium speed
- far: slower, dimmer

Suggested speeds:

- far: `10 px/s`
- mid: `20 px/s`
- near: `35 px/s`

Stars wrap from bottom to top.

---

## Controls

| Input | Action |
|---|---|
| `Left` / `A` | Move left |
| `Right` / `D` | Move right |
| `Space` | Fire |
| `Enter` | Start / Restart |
| `Esc` | Quit |

Touch and gesture input are not required for this version.

---

## Update Order

To avoid ambiguous collision behavior, the gameplay update should follow a stable order:

1. Read input
2. Update player movement / firing
3. Advance alien step timer and formation movement
4. Advance enemy fire timer and spawn enemy bullets
5. Move bullets
6. Resolve collisions
7. Update particles, floating texts, starfield, shake
8. Evaluate wave clear / game over transitions

Collision priority for the player bullet:

1. mystery ship
2. nearest alien hit
3. shield block

This keeps scoring interactions predictable when objects are vertically close together.

---

## Rendering Order

Recommended gameplay draw order:

1. background fill
2. starfield
3. shields
4. aliens
5. mystery ship
6. bullets + trails
7. player
8. particles
9. floating score text
10. ground line
11. HUD
12. wave-complete / death overlay text if needed

---

## Technical Constraints

- Use `GetFrameTime()` for movement and timers
- Use `CheckCollisionRecs()` for AABB collision checks
- Use `GetRandomValue()` for random selection
- Use fixed-size arrays or pools for gameplay entities
- Do not rely on heap allocation for core gameplay entities
- Draw using raylib primitives only

Allowed draw APIs include:

- `DrawRectangle`
- `DrawRectangleRec`
- `DrawTriangle`
- `DrawCircle`
- `DrawCircleV`
- `DrawLine`
- `DrawLineEx`
- `DrawPoly`
- `DrawText`
- `Fade`

Do not use:

- `LoadTexture`
- `LoadImage`
- `LoadFont`
- any audio API

---

## Code Structure

All gameplay code remains under `src/`.

### New Files

| File | Purpose |
|---|---|
| `src/game_types.h` | Shared constants, enums, structs, compile-time limits |
| `src/gameplay.h` | Public gameplay API used by `screen_gameplay.c` |
| `src/gameplay.c` | Game state init, update loop, collisions, wave flow |
| `src/particles.h` | Particle system API |
| `src/particles.c` | Particle pool, spawn helpers, update/draw support |
| `src/drawing.h` | Drawing API for ships, aliens, shields, HUD, starfield |
| `src/drawing.c` | Primitive rendering helpers for all gameplay visuals |

The earlier draft listed only `.c` files for these modules, which would leave their interfaces undefined in C. Header files are required.

### Existing Files to Modify

| File | Changes |
|---|---|
| `src/game.c` | Set window to `800x600`, title to `Space Invaders`, remove audio init/cleanup, remove font loading, remove `OPTIONS` transitions, clear background to the new palette |
| `src/screens.h` | Remove `OPTIONS`, `font`, `music`, and `fxCoin`; add shared app/session state for score and high score |
| `src/screen_gameplay.c` | Thin screen wrapper that delegates to `gameplay.*` |
| `src/screen_title.c` | Replace template screen with stylized title implementation using default font only |
| `src/screen_ending.c` | Replace template screen with game-over / score display using default font only |
| `src/screen_logo.c` | Keep mostly unchanged unless minor compatibility cleanup is needed |

### Files to Delete

| File | Reason |
|---|---|
| `src/screen_options.c` | Options screen removed from the design |

### Resources Cleanup

These files are no longer needed:

- `src/resources/coin.wav`
- `src/resources/mecha.png`

If no runtime assets remain, remove the resource-copy step from `CMakeLists.txt` as part of cleanup. Leaving a dead asset pipeline in place is unnecessary.

---

## Shared App State

Use a small shared struct for cross-screen data, exposed through `screens.h` or a dedicated shared header.

Minimum shared data:

- current score
- high score
- last run final score
- current wave if title / ending should display it later

Do not spread these values across unrelated globals.

---

## Acceptance Criteria

The spec is satisfied when:

1. The game boots into `LOGO`, then reaches a polished `TITLE` screen.
2. Pressing `Enter` starts gameplay directly.
3. The player can move, fire, and clear waves with only one shot active.
4. Aliens march, accelerate, descend, and fire correctly.
5. Shields degrade cell by cell from both player and enemy fire.
6. The mystery ship spawns and awards random bonus points.
7. Score, wave, lives, and high score display correctly.
8. The game transitions to `ENDING` on player defeat or invasion.
9. No textures, custom fonts, or audio are used anywhere.
10. The codebase no longer contains `OPTIONS` screen dependencies.

---

## Summary of Corrections Made During Review

This revised spec fixes several issues in the original draft:

- removed ambiguity around `OPTIONS` removal
- corrected wave-speed scaling so wave 1 is not accidentally accelerated
- changed enemy shooting from frame-based chance to time-based logic
- fixed invasion loss condition to use the shield line consistently
- added missing header files for the proposed C module split
- clarified wave reset, death reset, and bullet cleanup rules
- clarified shield cell state handling
- clarified what resets on new game versus what persists for the session
- removed leftover touch/audio/template assumptions from the design
