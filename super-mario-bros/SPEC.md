# Super Mario Bros — Game Specification

## Overview

A modernized Super Mario Bros clone focusing on the core mechanics of the original NES game. Written in C with raylib. Levels are tile-based, side-scrolling left-to-right. Graphics use PNG sprites (created from SVGs via Inkscape) with modern visual touches like particle effects and smooth animations.

This is not a 1:1 recreation — we're capturing the feel and mechanics, not matching frame data or sub-pixel quirks.

---

## Window & Rendering

- **Resolution:** 1200x900
- **Tile size:** 16x16 pixels
- **FPS:** 60
- **Camera:** follows Mario horizontally, does not scroll backward (classic SMB behavior). No vertical scrolling — the level fits vertically within the screen.
- **Sprites:** PNG textures loaded via raylib. Source SVGs stored separately for editing.

---

## Game States

```
TITLE -> PLAYING -> DYING -> PLAYING (respawn) or GAME_OVER -> TITLE
                     |
                     +-> LEVEL_COMPLETE -> PLAYING (next level)
                     +-> CASTLE_COMPLETE -> PLAYING (next world)
                     +-> PAUSED
                     +-> WIN (after 8-4)
```

- **TITLE:** logo, "Press Enter to Start"
- **PLAYING:** main gameplay loop
- **DYING:** death animation, lose a life, respawn at level start or game over
- **LEVEL_COMPLETE:** flagpole slide animation, walk into castle, timer bonus tally, transition to next level
- **CASTLE_COMPLETE:** axe reached, bridge collapses, Bowser falls, Toad/Princess message, transition to next world
- **GAME_OVER:** display "Game Over", return to title
- **PAUSED:** freeze gameplay, resume on unpause
- **WIN:** Princess rescued in 8-4, ending message

---

## Mario

### States

| State | Description |
|-------|-------------|
| Small | Default. One hit = death. |
| Big | After mushroom. One hit = shrink to Small. Can break brick blocks. |
| Fire | After fire flower. Can throw fireballs. One hit = shrink to Small. |

After taking damage as Big or Fire, Mario has a brief invincibility window (flashing animation, ~2 seconds) during which he cannot be hurt again.

### Movement

- **Run:** accelerate to max speed; higher max speed when holding run button
- **Walk:** lower max speed when not holding run
- **Deceleration/friction:** Mario decelerates when no input or changing direction (skid)
- **Jump:** variable-height based on how long jump button is held. Higher jump when running faster. Mario can only jump when on the ground (no double jump).
- **Gravity:** constant downward acceleration, capped fall speed
- **No backward scrolling:** Mario cannot move left past the camera's left edge
- **Swimming:** in underwater levels, jump button gives upward impulse instead. Mario sinks slowly under gravity. No running. Fire Mario can still throw fireballs underwater.

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
| Ground | Solid. Standard terrain block. |
| Hard block | Solid. Indestructible (used in castles and underground). |
| Brick | Solid. Small Mario bumps from below (enemies on top are killed); Big/Fire Mario breaks. May contain coins or multi-coin blocks (hit repeatedly). |
| Question block | Solid. Hit from below to release item (coin, mushroom, fire flower, star). Becomes inactive (empty) after hit. |
| Used block | Solid. A question block or brick that has been emptied. Cannot be interacted with. |
| Pipe (top/body) | Solid. Some are entry points to underground/bonus areas (down input to enter). Pipe tops have a distinct visual from pipe bodies. |
| Flagpole | End-of-level trigger. Score based on contact height. |
| Flagpole base | Solid ground block at the flagpole base. |
| Castle door | End-of-castle trigger after defeating Bowser (reaching the axe). |
| Invisible block | Hidden until hit from below. Appears as a used block after hit. |
| Coral | Solid. Underwater terrain. |
| Bridge | Solid. Collapses when the axe is reached in Bowser fights. |
| Axe | End-of-castle trigger. Touching it collapses the bridge and defeats Bowser. |

### Tile Scaling

The original NES rendered 16x15 visible tiles (256x240). At 1200x900, we render tiles at 3.75x scale (16px * 3.75 = 60px per tile on screen), giving us approximately 20 tiles wide x 15 tiles tall visible area. The internal tile grid remains 16px; rendering scales up to fill the window.

### Level Format

Levels are stored as 2D tile arrays in C source code (static const). Each level has:
- A tile grid (width varies per level, height is 15 tiles — matching the original NES visible area)
- Entity spawn list: `{type, tile_x, tile_y}` entries loaded into the entity array as they scroll into view
- Background color / theme
- Level type flag (overworld, underground, underwater, castle, athletic) to determine physics and palette

---

## Enemies

### Summary Table

| Enemy | Stompable | Fireproof | Shell-killable | Star-killable | Notes |
|-------|-----------|-----------|----------------|---------------|-------|
| Goomba | Yes (dies) | No | Yes | Yes | Basic enemy |
| Green Koopa Troopa | Yes (shell) | No | Yes | Yes | Walks off ledges |
| Red Koopa Troopa | Yes (shell) | No | Yes | Yes | Turns at ledges |
| Green Paratroopa (bounce) | Yes (loses wings) | No | Yes | Yes | Bouncing |
| Green Paratroopa (fly) | Yes (loses wings) | No | Yes | Yes | Horizontal flight |
| Red Paratroopa | Yes (loses wings) | No | Yes | Yes | Vertical flight |
| Buzzy Beetle | Yes (shell) | **Yes** | Yes | Yes | Fireproof |
| Spiny | **No** (hurts Mario) | No | Yes | Yes | Thrown by Lakitu |
| Spiny Egg | **No** | No | Yes | Yes | Becomes Spiny on landing |
| Lakitu | Yes (dies) | No | Yes | Yes | Respawns, throws Spiny Eggs |
| Hammer Bro | Yes (dies) | No | Yes | Yes | Throws hammers, jumps between platforms |
| Bullet Bill | Yes (dies) | **Yes** | Yes | Yes | Fired from Bill Blasters |
| Bill Blaster | N/A (terrain) | N/A | No | No | Indestructible cannon, spawns Bullet Bills |
| Piranha Plant | **No** | No | Yes | Yes | Hides when Mario is near pipe |
| Blooper | N/A (underwater) | No | Yes | Yes | Pulsing diagonal swim toward Mario |
| Cheep-Cheep (swim) | N/A (underwater) | No | Yes | Yes | Horizontal swimmer |
| Cheep-Cheep (leap) | Yes (in air) | No | Yes | Yes | Leaps from water on bridge levels |
| Firebar | **No** (obstacle) | Immune | No | No | Indestructible, rotating fire chain |
| Podoboo | **No** (obstacle) | Immune | No | No | Indestructible, jumps from lava |
| Bowser | **No** | No (5 hits kill) | No | No | Bridge axe or 5 fireballs |

### Detailed Behaviors

**Goomba** — Walks forward, reverses on wall collision. Stomped = squished, dies. Side contact = damages Mario. Palette variants in underground/castle levels behave identically.

**Green Koopa Troopa** — Walks forward, walks off ledges. Stomped = retreats into shell. Shell can be kicked (slides, kills other enemies, bounces off walls). Kicked shell damages Mario on side contact. Fireball kills outright (no shell).

**Red Koopa Troopa** — Same as Green, but turns around at ledges instead of walking off.

**Green Koopa Paratroopa (bouncing)** — Bounces along the ground in an arc. First stomp removes wings, becomes Green Koopa Troopa. Second stomp = shell.

**Green Koopa Paratroopa (flying)** — Flies horizontally in a straight line (often over pits). Stomp removes wings, becomes Green Koopa Troopa and falls.

**Red Koopa Paratroopa** — Flies vertically up and down in a fixed column. Stomp removes wings, becomes Red Koopa Troopa.

**Buzzy Beetle** — Walks forward like Goomba. Fireproof — fireballs bounce off. Stomped = retreats into shell (behaves like Koopa shell when kicked).

**Spiny** — Walks forward. Cannot be stomped (spikes on top damage Mario). Killed by fireball, shell, or star. Only spawned by Lakitu.

**Spiny Egg** — Thrown by Lakitu in an arc. Becomes a Spiny on ground contact. Damages Mario on any contact before and after hatching.

**Lakitu** — Flies on a cloud at top of screen, follows Mario horizontally, throws Spiny Eggs at intervals. Can be stomped from high platforms/blocks. Respawns after Mario moves far enough.

**Hammer Bro** — Stands on platforms, jumps between two platform levels, throws hammers in arcs. Moves toward Mario if Mario lingers. Often appears in pairs. Can also be killed by hitting the block it's standing on from below.

**Bullet Bill** — Fired horizontally from Bill Blasters at constant speed. Fireproof. Stompable (Mario bounces off). Bill Blasters don't fire if Mario is standing on them or immediately adjacent.

**Bill Blaster** — Stationary solid terrain object (cannon). Cannot be destroyed. Mario can stand on it. Spawns Bullet Bills at intervals.

**Piranha Plant** — Emerges from pipes vertically on a timer, pauses, retreats. Cannot be stomped (any contact damages Mario). Does NOT emerge if Mario is standing on or adjacent to its pipe. Killed by fireball, shell, or star.

**Blooper** — Underwater only. Swims in pulsing diagonal pattern toward Mario. Cannot be stomped underwater. Killed by star.

**Cheep-Cheep (swimming)** — Underwater. Swims horizontally in a gentle sine-wave path. Red variant is faster than gray/green. Cannot be stomped underwater.

**Cheep-Cheep (leaping)** — Bridge levels. Leaps from water below in arcs, crossing the bridge. Stompable while airborne. Comes in large numbers.

**Firebar** — Rotating chain of fireballs anchored to a block. Clockwise or counterclockwise. Indestructible. Any contact damages Mario. Varies in length.

**Podoboo** — Jumps from lava in castle levels in a fixed vertical arc, falls back. Indestructible. Any contact damages Mario.

**Bowser** — End of each castle (worlds 1-4 through 8-4). Jumps, moves back and forth, breathes fire (horizontal fireballs). From World 6 onward, also throws hammers in arcs. Cannot be stomped. Defeated by: reaching the axe behind him (collapses bridge), or 5 fireballs. Worlds 1-7 Bowsers are disguised regular enemies (revealed when killed by fireballs). World 8 is the real Bowser.

**Bowser's Fireballs** — Horizontal projectiles launched by Bowser. Travel in a straight line. Indestructible. Any contact damages Mario.

**Hammer (projectile)** — Thrown by Hammer Bros and Bowser (worlds 6+). Travels in an arc. Indestructible. Any contact damages Mario.

### Additional Enemy Interactions

- **Block bump kill:** hitting a block from below kills any enemy standing on top of it. Works on all enemy types including Buzzy Beetles and Spinys.
- **Shell interactions:** a kicked shell kills all enemy types it contacts. Two shells colliding destroy both. A shell bouncing off a wall can come back and damage Mario.
- **Starman:** kills all enemies on contact except Bowser (Mario passes through him harmlessly). Environmental hazards (Firebar, Podoboo, lava, pits) still kill Star Mario.

---

## Items & Power-ups

| Item | Source | Effect |
|------|--------|--------|
| Coin | Floating in air, or from blocks | +1 coin, score. 100 coins = extra life. |
| Super Mushroom | Question block (when Small) | Small -> Big |
| Fire Flower | Question block (when Big) | Big -> Fire Mario |
| Starman | Question block (rare) | Temporary invincibility, kills enemies on contact |
| 1-Up Mushroom | Hidden blocks, specific locations | +1 life |

- Mushrooms/stars move: slide along ground, bounce off walls, fall off edges.
- Fire flower stays in place.
- Question blocks give a coin if Mario is already Big/Fire (instead of another mushroom).

### Fireballs

- Fire Mario can throw fireballs (up to 2 active at once) using the run/fire button.
- Fireballs travel forward in an arc, bouncing off the ground.
- They disappear on hitting a wall, an enemy, or after traveling offscreen.
- Fireballs do NOT work underwater in the original SMB (but Fire Mario can still throw them underwater — they just behave the same as on land with swimming physics).

---

## Entity System

All dynamic objects (Mario, enemies, items, projectiles, debris) are stored in a single flat array of `Entity` structs with a `type` tag.

```c
Entity entities[MAX_ENTITIES];  // MAX_ENTITIES = 128
int mario;                      // index of Mario in the array
```

Each entity has: type, position, velocity, size, facing direction, ground flag, active flag, collision property flags (see Collision Detection), and a small set of flat fields for type-specific state (animation frame, state timer, health/power level).

**Spawning:** find a free slot (`type == ENT_NONE`), set fields, set collision flags. One spawn helper per entity type.

**Activation:** enemies and items from the level spawn list are activated when they scroll into view (camera reaches their x position). They are not active before that. Once activated, they remain active until killed or scrolled far enough offscreen to be despawned.

**Update:** `entity_update()` switches on type to call the right behavior (mario input/physics, goomba walk/reverse, shell slide, mushroom move, fireball bounce, hammer arc, Bowser AI, etc.).

**Draw:** `entity_draw()` switches on type to draw the right sprite/animation.

**Why a tagged array:** variable entity counts per level, simple spawning/despawning, one collision loop covers all interactions, adding new types is just a new enum value + case. The entity array is fixed-size on the stack; level tile grids and other variable-size data use heap allocation.

---

## Collision Detection

### Tile Collision

AABB check against solid tiles in the grid. Resolve by pushing the entity out of the solid tile along the smallest overlap axis. Head bump detection: when Mario hits a solid tile from below, trigger block behavior (break brick, release item from question block, kill enemy standing on top).

### Entity Collision — Property-Based

Entities carry collision property flags set at spawn time:

```c
bool stompable;       // can Mario stomp it? (false for Spiny, Piranha Plant, Firebar, etc.)
bool damages_mario;   // does contact hurt Mario? (true for all enemies/hazards)
bool fire_immune;     // survives fireballs? (Buzzy Beetle, Bullet Bill, Firebar, Podoboo)
bool shell_killable;  // dies to kicked shell?
bool star_killable;   // dies to Starman Mario?
bool destructible;    // can be destroyed at all? (false for Firebar, Podoboo, Bill Blaster)
```

The collision phase in `game.c` checks properties instead of switching on every entity type:

- **Stomp check:** if Mario is falling and overlaps enemy's top half — if `stompable`, kill/transform the enemy; otherwise damage Mario.
- **Side contact:** if `damages_mario`, hurt Mario.
- **Fireball hit:** if `!fire_immune && destructible`, kill enemy.
- **Shell hit:** if `shell_killable`, kill enemy.
- **Star contact:** if `star_killable`, kill enemy.
- **Item collection:** items (coins, mushrooms, etc.) don't use these flags — they have their own collect logic by type.

Type-specific behavior (Koopa becoming a shell, Paratroopa losing wings, Lakitu respawning) still switches on type in the interaction handlers, but the core collision logic is generic.

### Stomp Detection

Mario is falling (vy > 0) and his bottom overlaps the enemy's top half -> stomp. Otherwise -> side contact (damage).

---

## Camera

- Scrolls right when Mario moves past a fixed horizontal threshold (~40% of screen width from the left edge)
- Never scrolls left (`camera_x` only increases, even if Mario walks left)
- Mario cannot move left past the camera's left edge (acts as a wall)
- Clamps at level start (`camera_x >= 0`) and level end (`camera_x <= level_width_px - screen_width`)
- No vertical scrolling — levels are designed to fit vertically within the screen

---

## Scoring

| Action | Points |
|--------|--------|
| Stomp enemy | 100 |
| Consecutive stomps (no landing) | 100, 200, 400, 800, 1000, 2000, 4000, 8000, then 1-Up |
| Fireball kill | 200 |
| Shell kill (per enemy) | 100, 200, 400, 800, 1000 (escalating for multi-kill chains) |
| Coin | 200 |
| Mushroom / Fire flower | 1000 |
| Starman | 1000 |
| 1-Up | No points (extra life) |
| Flagpole (height-based) | 100, 400, 800, 2000, 5000 |
| End-of-level timer bonus | Remaining time x 50 |

---

## HUD

Displayed at top of screen:

```
SCORE: 000000    COINS: x00    WORLD 1-1    TIME: 400    LIVES: x3
```

- **Timer:** counts down from 400 (decrements ~2.5 per real second in the original). Running out kills Mario. Remaining time converts to points at level end.
- **Coins:** 100 coins = 1-Up, counter resets to 0.
- **World display:** shows current world and sub-level (e.g. "1-1", "4-2").

---

## Audio

WAV sound effects generated with rfxgen:

| Sound | rfxgen Preset | Usage |
|-------|---------------|-------|
| Jump (small) | jump | Small Mario jumps |
| Jump (big) | jump (tweaked) | Big/Fire Mario jumps (lower pitch) |
| Coin | coin | Collect coin |
| Stomp | hit | Stomp enemy |
| Power-up | powerup | Collect mushroom/flower/star |
| Power-down | hit (tweaked) | Mario takes damage, shrinks |
| Brick break | explosion | Break brick block |
| Bump | blip (tweaked) | Hit solid block from below (no break) |
| Fireball | laser | Throw fireball |
| Death | hit (tweaked) | Mario dies |
| 1-Up | powerup (tweaked) | Extra life |
| Flagpole | blip | Grab flagpole |
| Kick | hit (tweaked) | Kick shell |
| Pipe | blip (tweaked) | Enter pipe |
| Bowser fall | explosion (tweaked) | Bowser falls into lava |

---

## Environmental Hazards

| Hazard | Description |
|--------|-------------|
| Bottomless pits | Gaps in the ground. Falling in = instant death (even with Star). |
| Lava | Bottom of castle levels. Instant death on contact. |
| Falling off screen | Below screen boundary = instant death in all level types. |
| Timer expiration | Timer reaches zero = instant death. |
| Balance lifts | Two platforms on a pulley. One lowers as Mario stands on it. Falling off over a pit = death. |
| Moving platforms | Small platforms that move horizontally or vertically. Missing one over a pit = death. |

---

## Levels

All 32 levels from the original game: worlds 1-1 through 8-4.

### Level Themes

| Theme | Description | Worlds |
|-------|-------------|--------|
| Overworld | Blue sky, green ground, pipes, blocks | 1-1, 1-3, 2-1, 2-3, 3-1, 3-3, etc. |
| Underground | Dark background, blue/gray bricks | 1-2, 4-2, etc. |
| Athletic/Bridge | Sky platforms, Cheep-Cheeps leaping from below | 2-3, 7-3, etc. |
| Castle | Dark, Firebars, Podoboos, lava, Bowser at end | x-4 levels |
| Underwater | Swimming physics, Bloopers, Cheep-Cheeps | 2-2, 7-2 |

### Level Features

- **Pipe warping:** enter certain pipes to access underground bonus areas or skip sections (warp zones)
- **Warp zones:** hidden areas that let the player skip to later worlds
- **Looping mazes:** World 8-4 requires taking the correct path or the level loops
- **Coin rooms:** underground bonus rooms filled with coins, accessed via pipe

### World 1-1 (first implementation)

The classic first level — used as the testbed during development:
- Flat ground with gaps (pits)
- Brick and question blocks at various heights
- Goombas and Koopa Troopas as enemies
- Coins floating and in blocks
- Pipes (some warpable)
- Flagpole at the end

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
6. **Items** — coins, mushroom (Small -> Big), fire flower, starman, 1-Up
7. **Basic enemies** — Goomba, Koopa Troopa (green/red), shell mechanics, stomping
8. **HUD & scoring** — score, coins, timer, lives display
9. **Game states** — death, respawn, game over, level complete (flagpole)
10. **Advanced enemies** — Paratroopas, Buzzy Beetle, Piranha Plant, Hammer Bro, Lakitu + Spiny, Bullet Bill
11. **Castle levels** — Firebars, Podoboos, lava, Bowser boss fights
12. **Underwater levels** — swimming physics, Blooper, Cheep-Cheep
13. **Level features** — pipe warping, warp zones, underground bonus rooms, looping mazes (8-4)
14. **All 32 levels** — build out worlds 1-1 through 8-4
15. **Audio** — generate and integrate all sound effects
16. **Polish** — particles, animations, screen transitions, title screen
