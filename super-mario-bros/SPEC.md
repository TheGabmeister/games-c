# Super Mario Bros — Game Specification

## Overview

A modernized Super Mario Bros clone focusing on the core mechanics of the original NES game. Written in C with raylib. Levels are tile-based, side-scrolling left-to-right. Graphics use PNG sprites (created from SVGs via Inkscape) with modern visual touches like particle effects and smooth animations.

This is not a 1:1 recreation — we're capturing the feel and mechanics, not matching frame data or sub-pixel quirks.

---

## Window & Rendering

- **Resolution:** 1200x900
- **Tile size:** 16x16 pixels
- **FPS:** 60
- **Camera:** follows Mario horizontally, does not scroll backward (classic SMB behavior). Vertical scrolling for underground/sky areas.
- **Sprites:** PNG textures loaded via raylib. Source SVGs stored separately for editing.

---

## Game States

```
TITLE -> PLAYING -> DYING -> PLAYING (respawn) or GAME_OVER -> TITLE
                     |
                     +-> LEVEL_COMPLETE -> PLAYING (next level)
                     +-> PAUSED
```

- **TITLE:** logo, "Press Enter to Start"
- **PLAYING:** main gameplay loop
- **DYING:** death animation, lose a life, respawn or game over
- **LEVEL_COMPLETE:** flagpole animation, score tally, transition to next level
- **GAME_OVER:** display "Game Over", return to title
- **PAUSED:** freeze gameplay, resume on unpause

---

## Mario

### States

| State | Description |
|-------|-------------|
| Small | Default. One hit = death. |
| Big | After mushroom. One hit = shrink to Small. Can break brick blocks. |
| Fire | After fire flower. Can throw fireballs. One hit = shrink to Small. |

### Movement

- **Run:** accelerate to max speed; higher max speed when holding run button
- **Walk:** lower max speed when not holding run
- **Deceleration/friction:** Mario decelerates when no input or changing direction (skid)
- **Jump:** variable-height based on how long jump button is held. Higher jump when running faster.
- **Gravity:** constant downward acceleration, capped fall speed
- **No backward scrolling:** Mario cannot move left past the camera's left edge

### Physics Constants (tunable)

| Parameter | Approximate Value |
|-----------|-------------------|
| Walk max speed | 90 px/s |
| Run max speed | 150 px/s |
| Acceleration | 450 px/s² |
| Deceleration (friction) | 400 px/s² |
| Skid deceleration | 700 px/s² |
| Jump initial velocity | -280 px/s |
| Jump sustained (hold) | -50 px/s² (reduces gravity while held) |
| Gravity | 980 px/s² |
| Max fall speed | 600 px/s |

These are starting points — we'll tune by feel.

---

## Tiles & Level Structure

### Tile Types

| Tile | Behavior |
|------|----------|
| Empty/Air | Passable |
| Ground | Solid. Brown brick. |
| Brick | Solid. Small Mario bumps from below; Big Mario breaks. May contain coins/items. |
| Question block | Solid. Hit from below to release item (coin, mushroom, fire flower, star). Goes inactive after hit. |
| Pipe (top/body) | Solid. Decorative in most cases. Entry point for underground areas (stretch goal). |
| Flagpole | End-of-level trigger. |
| Invisible block | Hidden until hit from below (stretch goal). |

### Level Format

Levels are stored as 2D tile arrays in C source code (static const). Each level has:
- A tile grid (width varies, height fixed ~15 tiles visible)
- Entity spawn list: `{type, tile_x, tile_y}` entries loaded into the entity array at level start
- Background color / theme

---

## Enemies

### Goomba

- Walks in one direction, reverses on wall collision
- Stomped from above: squished, dies
- Contact from side: damages Mario
- Dies from fireball, star, or shell hit

### Koopa Troopa

- Walks in one direction, reverses on wall collision
- Stomped: retreats into shell
- Shell can be kicked (slides, kills enemies it hits, bounces off walls)
- Kicked shell damages Mario on contact
- Dies from fireball, star

### Piranha Plant (stretch goal)

- Emerges from pipes on a timer
- Does not emerge if Mario is adjacent to the pipe
- Cannot be stomped; killed by fireball or star

---

## Items & Power-ups

| Item | Source | Effect |
|------|--------|--------|
| Coin | Floating in air, or from blocks | +1 coin, score. 100 coins = extra life. |
| Super Mushroom | Question block (when Small) | Small -> Big |
| Fire Flower | Question block (when Big) | Big -> Fire Mario |
| Starman | Question block (rare) | Temporary invincibility, kills enemies on contact |
| 1-Up Mushroom | Hidden blocks (stretch goal) | +1 life |

- Mushrooms/stars move: slide along ground, bounce off walls, fall off edges.
- Fire flower stays in place.
- Question blocks give a coin if Mario is already Big/Fire (instead of another mushroom).

---

## Entity System

All dynamic objects (Mario, enemies, items, projectiles, debris) are stored in a single flat array of `Entity` structs with a `type` tag. Fixed-size, no heap allocation.

```c
Entity entities[MAX_ENTITIES];  // MAX_ENTITIES = 128
int mario;                      // index of Mario in the array
```

Each entity has: type, position, velocity, size, facing direction, ground flag, active flag, and a small set of flat fields for type-specific state (animation frame, state timer, health/power level).

**Spawning:** find a free slot (`type == ENT_NONE`), set fields. One spawn helper handles all types.

**Update:** `entity_update()` switches on type to call the right behavior (mario input/physics, goomba walk/reverse, shell slide, mushroom move, etc.).

**Draw:** `entity_draw()` switches on type to draw the right sprite/animation.

**Why a tagged array:** variable entity counts per level, simple spawning/despawning, one collision loop covers all interactions, adding new types is just a new enum value + case. The entity array is fixed-size on the stack; level tile grids and other variable-size data use heap allocation.

---

## Collision Detection

- **Tile collision:** AABB check against solid tiles in the grid. Resolve by pushing the entity out of the solid tile along the smallest overlap axis.
- **Entity collision:** AABB overlap between Mario and other entities. `game.c` iterates the entity array and switches on the other entity's type to determine the interaction (stomp, damage, collect, etc.).
- **Head bump:** when Mario hits a solid tile from below, trigger block behavior (break brick, release item from question block).
- **Stomp detection:** Mario is falling (vy > 0) and his bottom overlaps the enemy's top half -> stomp. Otherwise -> damage.

---

## Camera

- Follows Mario's x position with a dead zone (Mario stays in the left ~40% of screen)
- Never scrolls left (camera_x only increases)
- Clamps at level start (0) and level end (level_width - screen_width)

---

## Scoring

| Action | Points |
|--------|--------|
| Stomp enemy | 100 |
| Consecutive stomps (no landing) | 100, 200, 400, 800, 1000 |
| Fireball kill | 200 |
| Coin | 200 |
| Mushroom / Fire flower | 1000 |
| Starman | 1000 |
| 1-Up | No points (extra life) |
| Flagpole (height-based) | 100–5000 |

---

## HUD

Displayed at top of screen:

```
SCORE: 000000    COINS: x00    WORLD 1-1    TIME: 400    LIVES: x3
```

- **Timer:** counts down from 400. Running out kills Mario.
- **Coins:** 100 coins = 1-Up, counter resets.

---

## Audio

WAV sound effects generated with rfxgen:

| Sound | rfxgen Preset | Usage |
|-------|---------------|-------|
| Jump | jump | Mario jumps |
| Coin | coin | Collect coin |
| Stomp | hit | Stomp enemy |
| Power-up | powerup | Collect mushroom/flower/star |
| Brick break | explosion | Break brick block |
| Fireball | laser | Throw fireball |
| Death | hit (tweaked) | Mario dies |
| 1-Up | powerup (tweaked) | Extra life |
| Flagpole | blip | Level complete |
| Bump | blip (tweaked) | Hit block from below (no break) |

---

## Levels

### World 1-1 (first implementation)

The classic first level:
- Flat ground with gaps (pits)
- Brick and question blocks at various heights
- Goombas and Koopa Troopas as enemies
- Coins floating and in blocks
- Pipes (decorative initially)
- Flagpole at the end

### Future worlds (stretch goals)

- Underground sections (enter via pipe)
- Sky/cloud bonus areas
- Castle levels with different tileset
- Multiple worlds (1-1 through 8-4)

---

## Source Layout

```
src/
  common.h        Constants, enums, shared types (EntityType, GameState, Direction)
  entity.h/c      Entity struct, update/draw dispatch, spawn helpers
  mario.h/c       Mario-specific input, physics, state transitions (operates on Entity*)
  level.h/c       Tile grid, tile types, level data, tile collision helpers
  camera.h/c      Camera follow logic, dead zone, clamping
  particles.h/c   Particle effects
  game.h/c        Game struct (entity array, level, camera), state machine, orchestration, HUD
  main.c          Entry point
  resources/      PNGs (runtime), WAVs (runtime)
  resources/svg/  Source SVGs (not loaded at runtime)
```

### Update Flow (STATE_PLAYING)

1. Mario input & physics (`mario_update`)
2. Tile collision for Mario (`level_collide_entity`)
3. Block hit logic (break bricks, spawn items from question blocks)
4. Entity update loop — move enemies, items, projectiles, debris (`entity_update` per active entity)
5. Tile collision for non-Mario entities
6. Entity-vs-Mario collision — stomp, damage, collect, etc. (`game.c` handles interactions)
7. Entity-vs-entity collision — shell kills enemies, fireball kills enemies
8. Particles update
9. Camera update

### Draw Flow

1. Background
2. Level tiles (`level_draw`)
3. Entity draw loop — all active entities including Mario (`entity_draw`)
4. Particles
5. HUD

---

## Implementation Order

1. **Mario movement & physics** — running, jumping, gravity, friction on flat ground
2. **Tile rendering & camera** — render a static level, scrolling camera
3. **Tile collision** — solid ground, walls, pits (falling = death)
4. **Sprites** — create SVGs, convert to PNGs, render Mario and tiles as textures
5. **Blocks** — question blocks (coins), brick breaking
6. **Items** — coins, mushroom (Small -> Big), fire flower
7. **Enemies** — Goomba AI, stomping, Koopa + shell mechanics
8. **HUD & scoring** — score, coins, timer, lives display
9. **Game states** — death, respawn, game over, level complete (flagpole)
10. **Audio** — generate and integrate sound effects
11. **Polish** — particles, animations, screen transitions, title screen

---

## Stretch Goals

- Pipe entry (warp to underground)
- Piranha Plants
- Moving platforms
- Underwater levels
- Multiple worlds
- High score persistence
- Warp zones
