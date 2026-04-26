# IMPLEMENTATION.md

High-level code architecture for the game described in SPEC.md.

## Game Loop

The main loop runs at 60 FPS. Each frame has four phases:

1. **Input**: read keyboard/gamepad, update input state.
2. **Update**: advance game logic — player, enemies, projectiles, timers,
   collision, transitions. Drain the event queue at the end of update.
3. **Draw**: render the play area, entities, effects, then the HUD.
4. **Audio**: process queued sound events (jingles, effects).

Screen transitions and pause screen replace the normal update/draw with
their own logic. The game uses a top-level state machine:

- `STATE_TITLE` — file select screen.
- `STATE_PLAY` — normal gameplay.
- `STATE_PAUSE` — inventory/pause screen. Game logic frozen, draw dimmed.
- `STATE_TRANSITION` — screen scroll or fade. Input locked, entities frozen,
  scroll interpolation runs until complete.
- `STATE_DEATH` — death animation, then continue prompt.
- `STATE_ITEM_GET` — brief item display with jingle. Entities frozen.

## Module Structure

```
src/
  main.c            entry point, raylib init, main loop
  game.h/c          Game struct, top-level state machine, frame dispatch
  game_config.h     compile-time constants
  input.h/c         input abstraction (keyboard + gamepad)
  player.h/c        player state, movement, combat, items
  enemy.h/c         enemy definitions, AI, spawning
  projectile.h/c    player and enemy projectiles
  tilemap.h/c       tile data, screen/room loading, tile collision
  camera.h/c        screen transitions, scroll animation
  hud.h/c           HUD rendering
  inventory.h/c     pause screen logic and rendering
  event.h/c         event queue
  collision.h/c     hitbox overlap checks, collision dispatch
  save.h/c          save/load serialization
  sounds.h/c        sound loading and playback
  textures.h/c      sprite/texture loading and management
  assets/            game assets (PNG, WAV, map data)
```

Each module is a header/source pair. Headers declare the public interface.
No module includes another module's `.c` file. Shared types go in the header
that owns them (e.g., `Direction` in `player.h`, `TileType` in `tilemap.h`).

## Core Data Structures

### Game

Single top-level struct owned by `main.c`, passed by pointer to all systems.
Contains or points to all game state:

- Game state enum (title, play, pause, transition, death, item-get).
- Player struct.
- Current screen/room enemies array.
- Active projectiles array.
- Active pickups array.
- Event queue.
- World state (persistent flags, save data).
- Camera/transition state.
- Asset handles (textures, sounds).

### Player

Struct with:

- Pixel position (continuous, not tile-locked).
- Facing direction (N/S/E/W enum).
- State machine: IDLE, MOVING, ATTACKING, USING_ITEM, KNOCKBACK, INVULNERABLE.
- State timer (frames remaining in current state).
- Health (current, max) in half-hearts.
- Equipment tiers (sword, shield, armor).
- Collected items bitfield.
- Inventory (rupees, bombs, keys, equipped item).
- Invulnerability timer.

### Enemy

Data-driven design with function pointers. Each enemy type has a static
definition table:

```c
typedef struct EnemyDef {
    int health;
    int contact_damage;
    int projectile_damage;
    int speed;
    int tier;
    bool ignores_walls;
    void (*update)(Enemy *self, Game *game);
    void (*draw)(Enemy *self, Game *game);
} EnemyDef;
```

The `EnemyDef` table is a const array indexed by `EnemyType` enum. Adding a
new enemy means writing its update/draw functions and adding a row to the
table. The orchestrator calls `enemy->def->update(enemy, game)` — no switch
on enemy type.

Runtime enemy instances hold a pointer to their def plus per-instance state:

- Pixel position and facing direction.
- Current health.
- AI state and timer (meaning varies per enemy type).
- Active/inactive flag.

### Projectile

Same function-pointer pattern as enemies. Each projectile type has a def
with `update` and `draw` pointers:

- Position, velocity (direction + speed).
- Owner (player or enemy, determines collision layer).
- Lifetime or range remaining.
- Active flag.

Boomerangs are special: they reverse direction after reaching max range and
return to the owner. This is handled in the boomerang's own `update`
function, not a special case in the orchestrator.

### Dispatch strategy

Use function pointers (vtable-style) for type sets that are large and
expected to grow: enemies (26 types) and projectiles (8+ types). Adding a
new type means writing its functions and a def table row — no other files
change.

Use switch statements for state machines with few, stable states: game
state (6 states), player state (6 states). These are small enough that a
switch is clearer than indirection.

### Tilemap

Each screen/room is a 16x11 grid of tile IDs:

```c
uint8_t tiles[SCREEN_TILES_Y][SCREEN_TILES_X];
```

Tile IDs map to a tile property table that stores: passability, type
(floor, wall, water, pit, door, stairs, etc.), and sprite index. This keeps
tile collision as a simple table lookup.

Screen data also includes: enemy spawn list, item placements, door
positions, and special triggers (shutter room flag, pushable block
positions).

## Event System

A simple queue drained once per frame at the end of update. Events are
fire-and-forget notifications that decouple systems.

```c
typedef enum EventType {
    EVENT_ENEMY_KILLED,
    EVENT_ITEM_PICKUP,
    EVENT_SECRET_REVEALED,
    EVENT_DOOR_OPENED,
    EVENT_BOSS_DEFEATED,
    EVENT_PLAYER_DAMAGED,
    EVENT_PLAYER_DEATH,
    EVENT_SHUTTER_CLEAR,
    EVENT_COUNT
} EventType;

typedef struct Event {
    EventType type;
    int param;
} Event;
```

Producers push events during update (e.g., collision detects a kill and
pushes `EVENT_ENEMY_KILLED`). Consumers read them at drain time:

- `EVENT_ENEMY_KILLED` → increment kill counter (drop table), check if room
  is clear (shutter doors), play sound.
- `EVENT_ITEM_PICKUP` → update inventory, play jingle, enter ITEM_GET state
  if major item.
- `EVENT_SECRET_REVEALED` → play secret jingle, mark in world state.
- `EVENT_PLAYER_DAMAGED` → flash effect, start invulnerability timer.
- `EVENT_SHUTTER_CLEAR` → open shutter doors in current room.

The queue is a fixed-size ring buffer. Events that overflow are dropped
(indicates a bug — the buffer should be sized generously).

## Collision Pipeline

Runs once per frame during update, after all positions have been advanced:

1. **Tile collision**: for each moving entity, check destination tile in the
   tile property table. Block if impassable. This uses logical grid
   positions, not pixel positions.

2. **Hitbox collision**: build hitboxes from visual positions for all active
   entities, the sword (if swinging), and all projectiles. Check overlaps
   using axis-aligned rectangle intersection:
   - Player sword hitbox vs. enemy hitboxes.
   - Player hitbox vs. enemy hitboxes (contact damage).
   - Player hitbox vs. enemy projectile hitboxes.
   - Player projectile hitboxes vs. enemy hitboxes.
   - Player hitbox vs. pickup hitboxes.
   - All projectile hitboxes vs. terrain (stop on wall tile).

3. **Response**: on hit, push the appropriate event, apply damage, start
   knockback, consume projectile. Shield check: if player is idle
   (not attacking) and the enemy projectile comes from the facing direction,
   block instead of damage.

Collision layers are implicit in the check order above — there is no
general-purpose layer mask. Player-side and enemy-side never need to check
against their own team.

## Screen and Room Management

The overworld is a 16x8 grid of screen data. Dungeons are separate grids
(up to 8x8 rooms). The game tracks the current screen coordinates.

On screen transition:

1. Enter `STATE_TRANSITION`.
2. Freeze all entities.
3. Load the adjacent screen's tile and spawn data.
4. Scroll the camera over ~30 frames (half second at 60 FPS).
5. Spawn enemies for the new screen. Check world state for persistent
   changes (opened doors, revealed walls, collected items).
6. Place the player at the entry edge of the new screen.
7. Return to `STATE_PLAY`.

Cave/dungeon entry uses a fade-to-black instead of a scroll. The fade takes
~15 frames out, loads the new room, ~15 frames in.

## Save/Load

Save data is a flat binary struct written to disk. Three save slots.
The struct mirrors the Save Data Contract in SPEC.md:

- Player stats (health, rupees, bombs, keys, equipment tiers).
- Collected items bitfield.
- Per-dungeon flags (map, compass, boss defeated, fragment).
- Persistent world state: bitfields for opened doors, revealed walls,
  collected heart containers, visited caves, spent shops, triggered NPCs.
- Current location (overworld or dungeon + coordinates).

On save: serialize the struct to a file. On load: deserialize and
reconstruct game state. On death/continue: reset health to three hearts and
position to start/dungeon entrance, keep everything else.

The save file is small (under 1 KB) and written atomically (write to temp
file, then rename) to prevent corruption.

## Asset Organization

Runtime assets are loaded from the `assets/` directory relative to the
executable (copied there by CMake at build time).

- **Sprites**: loaded as raylib `Texture2D`. Spritesheets where possible
  (one texture for all player frames, one for all enemy types, one for
  tiles). Individual sprites are regions within the sheet.
- **Sounds**: loaded as raylib `Sound`. One WAV per effect. Loaded at
  startup, held for the game's lifetime.
- **Music**: loaded as raylib `Music` (streamed). One track per
  biome/dungeon/boss.
- **Map data**: screen/room layouts stored as binary or simple text files.
  Loaded on demand when entering a new screen.

## Memory Model

- **Static/stack**: Game struct, player, fixed-size entity arrays (enemies,
  projectiles, pickups), event queue, tile data for the current screen.
  These are allocated once and reused.
- **Heap**: only for variable-size data that cannot be bounded at compile
  time — map data loaded from files, asset handles managed by raylib.

Fixed array sizes:

- Max enemies per screen: 16.
- Max projectiles: 32.
- Max pickups: 16.
- Event queue: 64 entries.
- Max overworld screens: 128 (16x8).
- Max dungeon rooms per dungeon: 64 (8x8).
