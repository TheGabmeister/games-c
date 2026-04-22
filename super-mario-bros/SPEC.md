# Super Mario Bros — Game Specification

## Overview

A modernized Super Mario Bros clone focusing on the core mechanics of the original NES game. Written in C with raylib. Levels are tile-based, side-scrolling left-to-right. Graphics use PNG sprites (created from SVGs via Inkscape) with modern visual touches like particle effects and smooth animations.

This is not a 1:1 recreation — we're capturing the feel and mechanics, not matching frame data or sub-pixel quirks.

---

## Window & Rendering

- **Window:** 1280x960 (4:3)
- **Tile size:** 64x64 pixels
- **Visible area:** 20x15 tiles (matches original NES proportions)
- **FPS:** 60
- **Camera:** follows Mario horizontally, does not scroll backward (classic SMB behavior). No vertical scrolling — the level fits vertically within the window.
- **Sprites:** 64x64 PNG textures loaded via raylib. Source SVGs stored separately for editing. No framebuffer scaling — rendering is done directly at window resolution.

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

Values are in pixels at our 64px tile size. For reference, one tile = 64px.

| Parameter | Approximate Value | In tiles/s |
|-----------|-------------------|------------|
| Walk max speed | 360 px/s | ~5.6 |
| Run max speed | 600 px/s | ~9.4 |
| Acceleration | 1800 px/s² | |
| Deceleration (friction) | 1600 px/s² | |
| Skid deceleration | 2800 px/s² | |
| Jump initial velocity | -1120 px/s | |
| Jump sustained (hold) | -200 px/s² (reduces gravity while held) | |
| Gravity | 3920 px/s² | |
| Max fall speed | 2400 px/s | |

These are starting points (scaled 4x from 16px-tile equivalents) — we'll tune by feel.

---

## Tiles & Level Structure

### Tile Types

| Tile | Behavior |
|------|----------|
| Empty/Air | Passable |
| Ground | Solid. Standard terrain block. |
| Hard block | Solid. Indestructible (used in castles and underground). |
| Brick | Solid. Small Mario bumps from below (enemies on top are killed); Big/Fire Mario breaks. May contain coins, items, or be a multi-coin brick (gives a coin per hit for ~4 seconds, then becomes used block). |
| Question block | Solid. Hit from below to release item (coin, mushroom, fire flower, star). Becomes inactive (empty) after hit. |
| Used block | Solid. A question block or brick that has been emptied. Cannot be interacted with. |
| Pipe (top/body) | Solid. Some are entry points to underground/bonus areas (down input to enter). Pipe tops have a distinct visual from pipe bodies. |
| Flagpole | End-of-level trigger. Score based on contact height. |
| Flagpole base | Solid ground block at the flagpole base. |
| Castle door | End-of-castle trigger after defeating Bowser (reaching the axe). |
| Invisible block | Hidden until hit from below. Appears as a used block after hit. |
| Coral | Solid. Underwater terrain. |
| Bridge | Solid. Collapses tile-by-tile when the axe is reached in Bowser fights. |
| Axe | End-of-castle trigger. Touching it collapses the bridge and defeats Bowser. |
| Vine block | A ? block that spawns a climbable vine. Vine grows upward, Mario can grab and climb it to reach sky bonus areas. |
| Bill Blaster | Solid. Cannon that spawns Bullet Bill entities at intervals. Does not fire if Mario is on or adjacent to it. |

### Level Format

Levels are stored as 2D tile arrays in C source code (static const). Each level has:
- A tile grid (width varies per level, height is 15 tiles)
- Entity spawn list: `{type, tile_x, tile_y}` entries loaded into the entity array as they scroll into view
- Pipe warp table: which pipes are enterable and where they lead `{pipe_tile_x, dest_level, dest_x, dest_y}`
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

**Bill Blaster** — A solid tile (not an entity). Cannot be destroyed. Mario can stand on it. The level update logic spawns Bullet Bill entities at intervals from Bill Blaster tile positions. Does not fire if Mario is standing on or adjacent to it.

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
- Fireballs work underwater in SMB1 — Fire Mario can throw them and they behave normally.

### Other Dynamic Objects

These are entities (live in the entity array) but are not enemies or collectible items:

| Object | Description |
|--------|-------------|
| Springboard | Green trampoline. Mario bounces high when landing on it (higher if holding jump). Stationary. |
| Vine | Grows upward from a vine block. Mario can grab it and climb vertically to reach sky bonus areas. |
| Score popup | Floating text (+100, +200, etc.) that drifts upward and fades. Spawned on enemy kill, coin collect, etc. |
| Brick debris | 4 fragments spawned when Big Mario breaks a brick. Fly outward in arcs, despawn after a short time. |
| Coin from block | A coin that pops up briefly from a ? block or brick when hit. Visual only — despawns after the pop animation. The actual coin count is incremented by the tile handler. |

---

## Entity System

All dynamic objects (Mario, enemies, items, projectiles, debris) are stored in a single flat array of `Entity` structs with a `type` tag.

```c
Entity entities[MAX_ENTITIES];
int mario;  // index of Mario in the array
```

### Entity struct

Each entity has:
- **Common fields:** type, position, velocity, size, facing direction, on_ground flag, active flag
- **Type-specific state:** animation frame/timer, state timer, health/power level (flat fields, no union)
- **Collision flags:** `stompable`, `damages_mario`, `fire_immune`, `shell_killable`, `star_killable`, `destructible` — set at spawn time, used by the engine's collision phase (see Collision Detection)
- **Vtable pointer:** points to a `static const EntityVtab` that defines the entity's behavior callbacks

### Vtable (function pointers per entity type)

Each entity type defines a `static const EntityVtab` with callback functions. The engine calls these — the entity decides what to do. This is how interaction logic stays with the entity type instead of accumulating in `game.c`.

```c
typedef struct {
    void (*update)(Entity *self, Game *game);
    void (*draw)(Entity *self, float camera_x);
    void (*touch)(Entity *self, Entity *other, Game *game);
    void (*stomped)(Entity *self, Entity *mario, Game *game);
    void (*hit_by_fire)(Entity *self, Game *game);
    void (*hit_by_shell)(Entity *self, Game *game);
    void (*hit_by_star)(Entity *self, Game *game);
    void (*bumped)(Entity *self, Game *game);
    void (*kill)(Entity *self, Game *game);
} EntityVtab;
```

Unimplemented callbacks are NULL — the engine skips them. Each entity type sets its vtable at spawn time. Examples:

- **Goomba's `stomped`:** play squish sound, spawn score popup, set type to ENT_NONE.
- **Koopa's `stomped`:** transform into ENT_SHELL, set vx = 0 (stationary until kicked).
- **Coin's `touch`:** increment coins/score, play coin sound, despawn self.
- **Mushroom's `touch`:** power up Mario, play power-up sound, despawn self.
- **Paratroopa's `stomped`:** remove wings (transform to Koopa Troopa), reassign vtable.

### Spawning

Each entity type has a spawn function (e.g. `spawn_goomba`, `spawn_coin`) that finds a free slot (`type == ENT_NONE`), sets all fields, collision flags, and the vtable pointer.

### Activation

Enemies and items from the level spawn list are activated when they scroll into view (camera reaches their x position). They are not active before that. Once activated, they remain active until killed or scrolled far enough offscreen to be despawned.

### Why this pattern

- **Engine stays generic:** `game.c` detects overlaps and calls callbacks. It does not contain Goomba logic, Koopa logic, coin logic, etc.
- **Entity owns its behavior:** each entity type defines what happens when it's stomped, hit by fire, touched, etc. Adding a new enemy means writing a new vtable + spawn function, not editing `game.c`.
- **Flat array for data:** the tagged array gives us simple iteration, cache-friendly layout, and easy spawning/despawning. Level tile grids and other variable-size data use heap allocation.
- **This is the standard pattern** in shipped C platformers (high_impact, Cave Story, waterCloset). It's the C equivalent of virtual methods — proven at this scale.

---

## Collision Detection

### Tile Collision

AABB check against solid tiles in the grid. Resolve by pushing the entity out of the solid tile along the smallest overlap axis.

When Mario hits a tile from below, the engine calls a **tile handler function** looked up from a handler table indexed by tile type. Each tile type defines its own response:

```c
// Example tile handlers
static void brick_hit(Game *game, int tx, int ty, Entity *mario) {
    if (mario->state >= MARIO_BIG) {
        destroy_brick(game, tx, ty);      // remove tile, spawn debris entities
        kill_enemy_on_tile(game, tx, ty);  // scan entities standing on this tile
        play_sound(game, SND_BRICK_BREAK);
    } else {
        bump_tile(game, tx, ty);           // bump animation
        kill_enemy_on_tile(game, tx, ty);
        play_sound(game, SND_BUMP);
    }
}

static void question_block_hit(Game *game, int tx, int ty, Entity *mario) {
    set_tile(game, tx, ty, TILE_USED);
    spawn_item_from_block(game, tx, ty);  // coin, mushroom, star, etc.
    kill_enemy_on_tile(game, tx, ty);
    play_sound(game, SND_BUMP);
}
```

This keeps tile interaction logic organized by tile type, not spread across a monolithic function.

### Entity Collision — Property-Based with Vtable Callbacks

The collision phase in `game.c` uses a two-step approach:

**Step 1 — Collision flags for the generic decision:**

```c
bool stompable;       // can Mario stomp it? (false for Spiny, Piranha Plant, Firebar, etc.)
bool damages_mario;   // does contact hurt Mario? (true for enemies/hazards, false for items)
bool fire_immune;     // survives fireballs? (Buzzy Beetle, Bullet Bill, Firebar, Podoboo)
bool shell_killable;  // dies to kicked shell?
bool star_killable;   // dies to Starman Mario?
bool destructible;    // can be destroyed at all? (false for Firebar, Podoboo, Bill Blaster)
```

The engine checks these flags to decide *what kind* of interaction occurs (stomp? damage? ignore?).

**Step 2 — Vtable callback for the type-specific response:**

Once the engine decides "this is a stomp," it calls `entity->vtab->stomped(entity, mario, game)`. The entity handles its own response (Goomba dies, Koopa becomes shell, Paratroopa loses wings, etc.).

```c
// game.c collision phase — generic, no per-type knowledge
for (int i = 0; i < MAX_ENTITIES; i++) {
    Entity *e = &game->entities[i];
    if (e->type == ENT_NONE || i == game->mario) continue;
    Entity *mario = &game->entities[game->mario];
    if (!aabb_overlap(mario, e)) continue;

    // Starman: kill enemies on contact
    if (mario->star_active && e->star_killable && e->vtab->hit_by_star) {
        e->vtab->hit_by_star(e, game);
        continue;
    }

    if (mario_is_falling(mario) && overlaps_top_half(mario, e)) {
        if (e->stompable && e->vtab->stomped)
            e->vtab->stomped(e, mario, game);
        else if (e->damages_mario && !mario->star_active)
            mario_take_damage(mario, game);
    } else if (e->vtab->touch) {
        e->vtab->touch(e, mario, game);
    }
}
```

- Items (coins, mushrooms, etc.) have `damages_mario = false` and handle collection in their `touch` callback.
- Enemies have `damages_mario = true` and handle side-contact damage in their `touch` callback.
- The engine just iterates and calls — it doesn't know what any entity type does.

### Stomp Detection

Mario is falling (vy > 0) and his bottom overlaps the enemy's top half -> stomp. Otherwise -> side contact (routed to `touch` callback).

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
- **Looping mazes:** castles 4-4, 7-4, and 8-4 have path puzzles — taking the wrong route loops the level back. 8-4 is the most complex (multiple sections with pipe transitions).
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
  common.h          Constants, enums, shared types (EntityType, GameState, Direction)
  entity.h/c        Entity/EntityVtab structs, collision flag helpers, generic spawn/kill
  mario.h/c         Mario vtable, input, physics, state transitions, spawn
  enemies/
    goomba.h/c      Goomba vtable + spawn
    koopa.h/c       Koopa Troopa, Paratroopa, Shell vtables + spawn
    buzzy_beetle.h/c
    lakitu.h/c      Lakitu + Spiny + Spiny Egg
    hammer_bro.h/c  Hammer Bro + Hammer projectile
    bullet_bill.h/c Bullet Bill (Bill Blaster is a tile, not an entity)
    piranha.h/c     Piranha Plant
    blooper.h/c     Blooper (underwater)
    cheep_cheep.h/c Cheep-Cheep (swimming + leaping)
    bowser.h/c      Bowser + Bowser fireballs
    firebar.h/c     Firebar (rotating obstacle)
    podoboo.h/c     Podoboo (lava jump)
  items.h/c         Coin, Mushroom, Fire Flower, Starman, 1-Up vtables + spawn
  blocks.h/c        Tile handler table (brick, question block, invisible block, etc.)
  level.h/c         Tile grid, tile types, level data, tile collision, entity activation
  camera.h/c        Camera follow logic, threshold, clamping
  particles.h/c     Particle effects
  game.h/c          Game struct, state machine, collision loops, HUD, orchestration
  main.c            Entry point
  resources/        PNGs (runtime), WAVs (runtime)
  resources/svg/    Source SVGs (not loaded at runtime)
```

### Update Flow (STATE_PLAYING)

1. Activate entities that have scrolled into view (`level_activate_entities`)
2. Mario input & physics (`mario->vtab->update`)
3. Tile collision for Mario — calls tile handler on head bump (`level_collide_entity`)
4. Entity update loop — call `entity->vtab->update` per active non-Mario entity
5. Tile collision for non-Mario entities
6. Entity-vs-Mario collision — engine checks flags, calls `stomped`/`touch` callbacks
7. Entity-vs-entity collision — shell/fireball vs enemies, calls `hit_by_shell`/`hit_by_fire` callbacks
8. Despawn entities that have scrolled far offscreen
9. Particles update
10. Camera update

### Draw Flow

1. Background
2. Level tiles (`level_draw`)
3. Entity draw loop — call `entity->vtab->draw` per active entity (including Mario)
4. Particles
5. HUD

---

## Implementation Order

Build the game world-by-world, introducing mechanics and enemies as they first appear in the original game. Each phase should be playable before moving to the next.

### Phase 1 — Core Engine

1. **Mario movement & physics** — running, jumping, gravity, friction, skid on flat ground
2. **Tile rendering & camera** — render a static level, scrolling camera, tile grid
3. **Tile collision** — solid ground, walls, pits (falling = death)
4. **Sprites** — create SVGs, convert to PNGs, render Mario and tiles as textures
5. **HUD & scoring** — score, coins, timer, lives display
6. **Game states** — title screen, death, respawn, game over, pause

### Phase 2 — World 1

7. **1-1:** Goomba, Green Koopa Troopa, shell kicking, ? blocks, brick blocks, coins, mushroom (Small -> Big), fire flower, starman, 1-Up, pipes (decorative), pits, flagpole + level complete sequence
8. **1-2:** Underground theme, Piranha Plant, enterable pipes, warp zone, coin rooms
9. **1-3:** Red Koopa Troopa, balance lifts, athletic/treetop level
10. **1-4:** Castle theme, Firebar, Podoboo, lava, Bowser boss fight, bridge + axe, Toad rescue message

### Phase 3 — World 2

11. **2-1:** Green Koopa Paratroopa (bouncing), springboard
12. **2-2:** Underwater level, swimming physics, Blooper, Cheep-Cheep (swimming)
13. **2-3:** Bridge level, Cheep-Cheep (leaping)
14. **2-4:** Castle (new Firebar configurations)

### Phase 4 — World 3

15. **3-1:** Hammer Bro
16. **3-2 through 3-4:** Build remaining levels with existing enemies (increased difficulty)

### Phase 5 — World 4

17. **4-1:** Lakitu, Spiny, Spiny Egg
18. **4-2:** Buzzy Beetle, vine/beanstalk (sky bonus area), second warp zone
19. **4-3 through 4-4:** Looping maze castle mechanic

### Phase 6 — Worlds 5–8

20. **5-1 through 5-4:** Bullet Bill, Bill Blaster
21. **6-1 through 6-4:** Higher density of existing enemies, Bowser throws hammers (6-4+)
22. **7-1 through 7-4:** Complex looping maze castle (7-4)
23. **8-1 through 8-4:** Gauntlet levels, underwater section inside castle, real Bowser (hammers + fire), game ending/win state

### Phase 7 — Audio & Polish

24. **Audio** — generate and integrate all sound effects
25. **Polish** — particles, death/transition animations, screen transitions, title screen art
