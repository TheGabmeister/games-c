# IMPLEMENTATION.md

High-level code architecture for the game described in SPEC.md.

Window: 1024x960 pixels. Logical resolution: 1024x960 at 1:1 scale (no
scaling). Tile size: 64x64 pixels (defined as `TILE_SIZE` in
`game_config.h`). The screen is divided vertically: HUD (1024x224, 3.5
tiles) at top, 32px divider, play area (1024x704, 11 tiles) below.

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
  enemy/
    enemy.h/c       shared types, def table, dispatch, spawning
    slime.c         slime update/draw
    bat.c           bat update/draw
    ghost.c         ghost update/draw
    ...             one file per enemy type
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

### Inventory

Player inventory is split by data shape:

```c
typedef struct Inventory {
    // Collected items — one bit each, never lost
    uint32_t items;           // bitfield indexed by ItemID enum

    // Equipment tiers — higher value = better
    int sword_tier;           // 0=none, 1=basic, 2=strong, 3=master
    int shield_tier;          // 0=none, 1=small, 2=large
    int armor_tier;           // 0=none, 1=blue, 2=red

    // Consumables — counts
    int rupees;
    int bombs;
    int bomb_capacity;
    int keys;

    // Active item slot
    ItemID equipped;          // which item the use button activates
} Inventory;
```

Checking if the player has an item: `inventory.items & (1 << ITEM_RAFT)`.
The conditional tile passability system uses the same check.

Equipping: the pause screen sets `inventory.equipped` to the selected
`ItemID`. Only items flagged as active (boomerang, bombs, bow, candle,
recorder, food, magic rod, potion) can be equipped. Passive items (raft,
ladder, bracelet) take effect automatically and are never equipped.

Using items: when the player presses the item button, the player enters the
USING_ITEM state and a switch on `inventory.equipped` runs the effect:

```c
switch (player->inventory.equipped) {
    case ITEM_BOOMERANG:  spawn_boomerang(player, projectiles); break;
    case ITEM_BOMB:       place_bomb(player, &inventory); break;
    case ITEM_BOW:        try_fire_arrow(player, &inventory, projectiles); break;
    case ITEM_CANDLE:     use_candle(player, tilemap, events); break;
    case ITEM_RECORDER:   use_recorder(player, events); break;
    case ITEM_POTION:     use_potion(player, &inventory); break;
    // ...
}
```

A switch is appropriate here — the item set is small (~12 equippable items)
and stable. Each case calls a function that owns the item's full behavior:
consuming ammo, spawning projectiles, modifying state, or pushing events.

Pause screen flow:

1. Player presses pause → game enters `STATE_PAUSE`.
2. Pause screen reads `inventory` to display collected items and equipment.
3. Player navigates the item grid and selects an item → sets
   `inventory.equipped`.
4. Player presses pause again → game returns to `STATE_PLAY`.
5. No game logic runs during pause. The pause screen only writes to
   `inventory.equipped`.

### Enemy

Data-driven design with function pointers. Each enemy type has a static
definition table:

```c
typedef struct EnemyContext {
    Vector2 player_pos;
    const TileMap *tilemap;
    EventQueue *events;
    ProjectileList *projectiles;
} EnemyContext;

typedef struct EnemyDef {
    int health;
    int contact_damage;
    int projectile_damage;
    int speed;
    int tier;
    bool ignores_walls;
    void (*update)(Enemy *self, const EnemyContext *ctx);
    void (*draw)(const Enemy *self);
} EnemyDef;
```

`EnemyContext` is built once per frame from the current game state and
passed to all enemy updates. It exposes only what enemies need: where the
player is, what tiles block movement, where to push events, and where to
spawn projectiles. Enemies cannot reach the full game state.

`draw` takes only the enemy itself — rendering should not have side effects
or need game-wide access.

The `EnemyDef` table is a const array indexed by `EnemyType` enum. Adding a
new enemy means writing its update/draw functions and adding a row to the
table. The orchestrator calls `enemy->def->update(enemy, &ctx)` — no switch
on enemy type.

Runtime enemy instances hold a pointer to their def plus per-instance state:

- Pixel position and facing direction.
- Current health.
- AI state and timer (meaning varies per enemy type).
- Active/inactive flag.

Enemy movement:

- Enemies move in continuous pixel space, same as the player.
- Enemies that respect walls (most ground enemies) check their pixel hitbox
  against impassable tiles the same way the player does, via the tilemap
  passed in `EnemyContext`.
- Enemies with `ignores_walls = true` (bats, ghosts, flying enemies) skip
  tile collision entirely.
- Enemy speed is in pixels per second. Each enemy's update function advances
  position by `speed * dt` in its chosen direction.

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

### Animation

Sprites are driven by an `Anim` struct shared by all entities:

```c
typedef struct AnimDef {
    int first_frame;
    int frame_count;
    int frame_duration;   // frames per sprite frame
    bool loops;
} AnimDef;

typedef struct Anim {
    const AnimDef *def;
    int timer;            // counts down each game frame
    int current_frame;    // index within the def's frame range
    bool finished;
} Anim;
```

Each entity type defines its animations as a static array of `AnimDef`s
indexed by state and facing direction. For example, the player has:

- `ANIM_IDLE_N`, `ANIM_IDLE_S`, `ANIM_IDLE_E`, `ANIM_IDLE_W` (1 frame each).
- `ANIM_WALK_N`, `ANIM_WALK_S`, `ANIM_WALK_E`, `ANIM_WALK_W` (2-4 frames, looping).
- `ANIM_ATTACK_N`, ... (2-3 frames, non-looping).

When an entity changes state or facing, it sets `anim.def` to the
appropriate `AnimDef` and resets the timer. Each frame, `anim_tick(&anim)`
decrements the timer and advances `current_frame` when it hits zero.

At draw time, the sprite frame index is `anim.def->first_frame +
anim.current_frame`. This indexes into the spritesheet by row/column to
produce the source rectangle.

Enemy animations follow the same pattern — each enemy's file defines its
own `AnimDef` array. Simple enemies (slime, bat) may have just idle and
move. Complex enemies (shield knight) may have idle, move, block, and
stagger.

### Tilemap

Each screen/room is a 16x11 grid of tile IDs:

```c
uint8_t tiles[SCREEN_TILES_Y][SCREEN_TILES_X];
```

Tile IDs map to a tile property table:

```c
typedef struct TileDef {
    TileType type;        // FLOOR, WALL, WATER, PIT, DOCK, GAP, etc.
    int sprite_index;
    bool passable;        // default passability
    ItemID pass_requires; // ITEM_NONE, ITEM_RAFT, ITEM_LADDER, etc.
} TileDef;
```

Tile collision checks `passable` first. If false and `pass_requires` is set,
the collision system checks the player's inventory — if the required item is
collected, the tile is treated as passable. This handles:

- Water tiles: `passable = false`, `pass_requires = ITEM_RAFT`. Only
  passable at dock tiles (which are always passable and trigger raft
  launch).
- Gap tiles: `passable = false`, `pass_requires = ITEM_LADDER`. The ladder
  activates automatically when the player steps onto a gap.
- Heavy rocks: `passable = false`, `pass_requires = ITEM_BRACELET`. The
  rock becomes pushable with the bracelet.

Enemies ignore `pass_requires` — they use only the base `passable` flag
(unless `ignores_walls` is set, in which case they skip tile collision).

### Screen file format

One plain text file per screen, stored in `assets/screens/` (overworld) and
`assets/dungeons/<n>/` (per dungeon). Human-editable, one character per
tile.

```
# metadata
shutter: true
enemy: slime 4 3
enemy: bat 10 7
item: key 8 5
door: north 8 locked

# tilemap (16 wide, 11 tall)
WWWWWWWWWWWWWWWW
W..............W
W..............W
W....WWWW......W
W..............W
D..............D
W......PP......W
W..............W
W..............W
W..............W
WWWWWWWWWWWWWWWW
```

Header lines before the tilemap declare metadata: enemy spawns (type,
tile x, tile y), item placements, door positions and lock state, and room
flags (shutter, dark). Lines starting with `#` are comments.

The tile character map is defined once in code (e.g., `W` = wall, `.` =
floor, `~` = water, `D` = door, `P` = pushable block, `S` = stairs). Adding
a new tile type means adding a character mapping and a row in the tile
property table.

Overworld screens are named by grid position: `assets/screens/03_05.txt`
for column 3, row 5. Dungeon rooms: `assets/dungeons/1/02_03.txt`. Caves:
`assets/caves/cave_12.txt`.

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
    union {
        struct { Vector2 pos; int enemy_type; } enemy_killed;
        struct { int item_id; bool is_major; } item_pickup;
        struct { Vector2 pos; } secret_revealed;
        struct { int damage; Direction from; } player_damaged;
    };
} Event;
```

Each event type has its own payload struct in the union. Producers fill the
relevant fields when pushing. Consumers read them at drain time:

- `EVENT_ENEMY_KILLED` → use `enemy_type` and `pos` for drop table lookup,
  play death sound, check if room is clear (shutter doors).
- `EVENT_ITEM_PICKUP` → use `item_id` to update inventory, `is_major` to
  decide whether to enter ITEM_GET state and play the big jingle.
- `EVENT_SECRET_REVEALED` → play secret jingle, mark location in world
  state.
- `EVENT_PLAYER_DAMAGED` → use `damage` for health reduction, `from` for
  knockback direction, start invulnerability timer.
- `EVENT_SHUTTER_CLEAR` → open shutter doors in current room.

The queue is a fixed-size ring buffer. Events that overflow are dropped
(indicates a bug — the buffer should be sized generously).

## Collision Pipeline

Runs once per frame during update, after all positions have been advanced:

1. **Tile collision**: for each moving entity, check the player's pixel
   hitbox against impassable tile rectangles. Block if overlapping. The
   player moves in pixel space but the tile grid determines passability.

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

Connectivity:

- Overworld screens connect implicitly by grid adjacency. Walking off the
  north edge of screen (3, 5) enters screen (3, 4). If the adjacent cell
  has no screen file, the edge is blocked (impassable border tiles).
- Dungeon rooms also connect by grid adjacency by default. A door on the
  north wall leads to the room one row up in the grid.
- Non-adjacent connections (warp stairs, one-way passages) are declared in
  the screen file metadata with a `warp` line:
  `warp: stairs 8 5 -> 02_07` — stairs tile at (8,5) warps to room 02_07.
- Cave entrances from the overworld use a similar warp:
  `warp: cave 7 3 -> cave_12` — entering the cave tile warps to cave room
  file `assets/caves/cave_12.txt`.

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

- **Sprites**: one spritesheet PNG per category (player, each enemy type,
  tiles, items, HUD, projectiles). Each sheet is drawn as a single SVG
  with frames on a 64px grid, exported via Inkscape. Loaded as raylib
  `Texture2D`. Individual frames are accessed by source rectangle using
  row/column indices. Stored in `assets/sprites/`.
- **Sounds**: loaded as raylib `Sound`. One WAV per effect. Loaded at
  startup, held for the game's lifetime.
- **Music**: loaded as raylib `Music` (streamed). One track per
  biome/dungeon/boss.
- **Map data**: plain text screen files (see Screen file format above).
  Stored in `assets/screens/` (overworld), `assets/dungeons/<n>/` (per
  dungeon), and `assets/caves/` (cave rooms). Loaded on demand when
  entering a new screen.

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

## Implementation Phases

Each phase produces a playable build that can be tested before moving on.
Later phases build on earlier ones — do not skip ahead.

### Phase 1 — Player and Tiles

Deliverable: player sprite moves on a single screen with tile collision.

- Animation system (`Anim`/`AnimDef` structs, `anim_tick`).
- Player sprite with walk animations (4 directions) and idle frames.
- Tilemap loading from a single hardcoded screen file.
- Tile animations (water, grass).
- Tile collision (wall blocking, impassable tiles).
- HUD layout (hearts, rupees, bombs, keys, equipped item, minimap area).
- `game_config.h` constants for tile size, screen dimensions.

### Phase 2 — World Navigation

Deliverable: player walks between connected overworld screens.

- Screen file parser (metadata + tilemap from plain text).
- Overworld grid (load screens by grid coordinates).
- Screen transitions (scroll animation, ~30 frames).
- Cave/warp transitions (fade-to-black).
- Camera module.
- Impassable screen edges when no adjacent screen exists.
- Overworld music (one track, switches on screen transition).

### Phase 3 — Combat

Deliverable: player can fight enemies, take damage, and die.

- Sword attack (hitbox in facing direction, active frames, cooldown).
- Enemy system (EnemyDef table, EnemyContext, spawn from screen metadata).
- 3 starter enemies: slime, bat, charging snake.
- Collision pipeline (sword vs enemy, player vs enemy contact).
- Contact damage and knockback.
- Invulnerability frames with flash.
- Health system (half-heart granularity, heart display in HUD).
- Death state and respawn at starting screen.
- Event system (enemy killed, player damaged, player death).
- Combat sounds (sword swing, hit, enemy death, player damage, low health).
- Enemy attack animations and tells.

### Phase 4 — Projectiles and Items

Deliverable: ranged combat works, basic inventory functional.

- Projectile system (player and enemy projectiles, lifetime, wall collision).
- Enemy projectiles: rock spitter, spear thrower.
- Player projectiles: boomerang (stun + return), bow/arrows (rupee cost).
- Bombs (placement, timed blast, area damage, bombable wall reveal).
- Pickup and drop system (rupees, hearts, bombs from defeated enemies).
- Inventory struct and equipped item slot.
- Shield blocking (projectile from facing direction).
- Item icons in HUD and inventory.
- Pause screen (equipment display, item grid, equipped item switching).
- Projectile and pickup sounds (arrow, bomb, boomerang, rupee, heart).
- Fire and magic animations.

### Phase 5 — Dungeons

Deliverable: one complete dungeon playable start to finish.

- Dungeon room grid loading (separate from overworld).
- Locked doors and key consumption.
- Shutter rooms (clear all enemies to open doors).
- Push blocks revealing stairs.
- Dark rooms (candle/fire tool to light).
- Boss AI (dragon guardian — first boss).
- Heart container and relic fragment rewards.
- Dungeon map and compass items.
- Dungeon map view on pause screen.
- `STATE_ITEM_GET` with jingle.
- Dungeon and boss music.
- Secret and door sounds.

### Phase 6 — World Systems

Deliverable: overworld is navigable with shops, NPCs, and saves.

- Shop system (buy items, rupee transaction).
- NPC dialogue (short text display, hint caves, gift caves).
- Cave room loading (`assets/caves/`).
- Item gates (raft, ladder, bracelet, fire, bombs — conditional tile
  passability).
- Sword upgrades (strong sword, master sword, health gate check).
- Save/load (binary struct, 3 slots, atomic write).
- Title screen with file select.
- Continue-after-death flow (reset health, keep progress).
- Shop and NPC sounds.

### Phase 7 — Content

Deliverable: full game content in place.

- All enemy types and red/blue variants.
- All 9 dungeons with bosses and mini-bosses.
- Full overworld (128 screens, all biome regions).
- All items and equipment upgrades.
- All NPCs, shops, secrets (bombable walls, burnable shrubs, pushable
  stones).
- Drop tables and economy balancing.
- Progression gate testing (no softlocks).
- Per-biome overworld music tracks.
- Per-dungeon music tracks.

### Phase 8 — Polish

Deliverable: game feels complete and ready for playtesting.

- Screen transition polish (timing, easing).
- Game balance tuning from playtesting.
- Edge case and regression fixes.
