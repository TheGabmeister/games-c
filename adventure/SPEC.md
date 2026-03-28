# Adventure - Game Specification

A modernized reimagining of Adventure (Atari 2600, 1980) built in C11 with Flecs ECS, SDL3, SDL3_mixer, and SDL3_ttf.

This spec replaces the current Pong gameplay with a single-screen action-adventure game while keeping the existing engine style: ECS-driven simulation, explicit pipeline phases, and simple SDL renderer primitives.

## 1. Vision

Recreate the core loop of the original Adventure:

- Explore a network of single-screen rooms
- Collect and carry one object at a time
- Avoid or defeat dragons
- Recover the chalice and return it to the gold castle

Modernization goals:

- 8-direction movement
- Cleaner rectangular visual language
- Dark-room fog of war
- Slightly clearer room readability than the Atari original
- Stable controls that remain faithful to the one-item tension

Non-goals for the first playable version:

- Full shader-based bloom
- Procedural room generation
- Save/load support
- Networked or local co-op

## 2. Presentation

| Property | Value |
|----------|-------|
| Window size | 1280 x 720 |
| Playfield | 960 x 720 |
| Framing | Centered 4:3 playfield with black pillarbox bars on the left and right |
| Background | Black |
| Art style | Stylized rectangles with bright cores, outline accents, and lightweight particle effects |

### 2.1 Visual style

- Walls, rooms, items, and creatures are rendered primarily as rectangles and simple composed shapes.
- Glow is simulated with layered translucent geometry, not a post-process bloom pass.
- Dark rooms use a circular visibility mask centered on the player.
- Maze rooms should be visually distinguishable through small tint and layout differences, not heavy decoration.

### 2.2 Palette

| Entity | Color direction |
|--------|-----------------|
| Player | Bright yellow / gold |
| Yorgle | Yellow-green |
| Grundle | Green |
| Rhindle | Red |
| Bat | Purple / magenta |
| Gold key / castle | Gold / amber |
| Black key / castle | Dark gray with pale outline |
| White key / castle | White / silver |
| Sword | Cyan |
| Magnet | Orange |
| Bridge | Warm brown / amber |
| Chalice | Bright gold with strongest shimmer |
| Walls | Dark gray with lighter edge accents |

## 3. Scenes And Core Loop

Game scenes:

- `SCENE_MENU`
- `SCENE_PLAYING`
- `SCENE_DEATH_FREEZE`
- `SCENE_VICTORY`

Primary loop during play:

1. Read input
2. Simulate player, AI, carry state, movement, and collisions
3. Resolve interactions and room transitions
4. Render current room
5. Render visible entities and effects
6. Render HUD or menu overlay

Winning condition:

- The chalice enters the gold castle throne room while carried by the player.

Fail state:

- There is no permanent fail state. Dragon bites cause a temporary death freeze and respawn.

## 4. World Structure

### 4.1 Rooms

- Full game world contains about 30 single-screen rooms.
- Game 1 uses a curated subset of those rooms.
- Rooms are connected as a graph with up to four exits per room.
- Exit links are not required to be symmetric. The maze may intentionally contain confusing navigation.

### 4.2 Coordinate space

- Room-local coordinates use the playfield space: `0..959` in X and `0..719` in Y.
- Entity transforms remain room-local.
- Each dynamic entity tracks its current room id separately from its position.

### 4.3 Room authoring model

Room geometry uses a hybrid model:

- Boundary walls are generated from the room perimeter plus authored exit gaps.
- Interior obstacles are authored as axis-aligned wall rectangles.
- Optional bridge sockets mark wall spans that the bridge may open.

This is preferred over a tile grid because it matches the desired rectangle-heavy look, works with the current rectangle collision code, and keeps room data compact.

### 4.4 Data source

- Room definitions are compiled into the game as const data.
- Use header declarations plus a single C definition file to avoid duplicated large arrays across translation units.
- No JSON or runtime parsing is required for the first version.

Recommended layout:

- `src/data/rooms.h` for declarations and shared constants
- `src/data/rooms.c` for room table definitions
- `src/data/game_modes.c` for spawn tables and mode-specific room subsets

### 4.5 Room categories

| Category | Approx. count | Notes |
|----------|----------------|------|
| Overworld | 10-12 | Open fields and connective paths |
| Gold castle | 3-4 | Includes start room and throne room |
| Black castle | 3-4 | Includes locked interior |
| White castle | 3-4 | Includes locked interior |
| Maze / catacombs | 6-8 | Some dark, some visually similar |
| Secret room | 1 | Easter egg area |

## 5. Room Geometry And Transition Rules

### 5.1 Directions

Use one canonical direction order everywhere:

```c
typedef enum Direction {
    DIR_NORTH = 0,
    DIR_EAST  = 1,
    DIR_SOUTH = 2,
    DIR_WEST  = 3,
    DIR_COUNT = 4
} Direction;
```

### 5.2 Exits

Each exit definition stores:

- Destination room id
- Gap span along the relevant edge
- Player spawn offset inside the destination room

Transition behavior:

- Transition occurs when the player crosses an open exit span.
- On arrival, the player is moved just inside the destination room by a fixed inset, such as 12 px.
- The perpendicular coordinate is preserved and clamped to the destination exit span.
- Dragons and the bat use the same room transition rules.

### 5.3 Walls

Boundary walls:

- Generated automatically from room size and exit gaps.
- Should not need to be hand-authored per room except for special gates.

Interior walls:

- Authored as axis-aligned rectangles.
- Must not overlap exits.
- Should leave enough clearance for the player, bat, and dragons.

### 5.4 Gates

Castle gates are special wall spans at castle entrances.

- A gate is either open or closed.
- Closed gates are solid collision.
- Open gates remove collision only for the gate span, not for the whole border wall.
- If a gate closes while an entity overlaps it, push that entity to the nearest valid side and keep the gate closed.

### 5.5 Bridge sockets

The bridge does not arbitrarily disable collision along any wall pixel it touches. That would make collision handling and authoring brittle.

Instead, rooms may define bridge sockets:

- A bridge socket references a specific wall span.
- When the bridge overlaps a socket closely enough, that socket becomes passable.
- Each room should define only the intentional wall spans that can be bridged.

This keeps the original bridge fantasy while making room data and collision behavior deterministic.

## 6. Player

| Property | Value |
|----------|-------|
| Shape | Small square, about 16 x 16 px |
| Color | Bright yellow / gold |
| Move speed | 200 px/s, tunable |
| Movement | 8-direction, normalized diagonals |
| Carry limit | 1 player-carryable object |

### 6.1 Controls

Gameplay:

- `WASD` or arrow keys: move
- `Space` or `Enter`: interact, pick up, swap, or drop
- `Escape`: return to menu from menu/victory scenes; during play it may quit to menu if implemented

Title menu:

- Up/down to change selection
- Enter or Space to confirm

An explicit interact button is preferred over auto-pickup because it:

- Matches the original Adventure more closely
- Prevents accidental swaps in cluttered rooms
- Makes bridge placement controllable
- Avoids frustrating magnet-related forced pickups

### 6.2 Interaction rules

If the player presses interact:

- While empty-handed and overlapping a player-carryable object: pick it up
- While carrying an object and overlapping another player-carryable object: swap them
- While carrying an object and not overlapping another player-carryable object: drop current object at a valid nearby location

Dropped object placement:

- Prefer directly in front of the player
- Fall back to the nearest non-blocked spot within a small search radius
- Never place inside a wall, gate, or another solid collider

### 6.3 Death and respawn

When a dragon bites the player:

1. Switch to `SCENE_DEATH_FREEZE`
2. Freeze world simulation and timer for about 1.5 seconds
3. Drop any carried object at the death location if possible
4. Respawn the player in the gold castle start room
5. Resume `SCENE_PLAYING`

World persistence rules:

- Items, dragons, gates, and the bat keep their current state and positions
- Only the player respawns

## 7. Carryable Objects

### 7.1 Carry model

There are two separate concepts:

- `Carryable`: an entity is allowed to be carried
- `Carried`: an entity is currently attached to a carrier

Carrier rules:

- The player may carry items and the secret dot
- The bat may carry items and dragons
- The player does not carry dragons
- A carried entity inherits the carrier's room id and follows a local offset

### 7.2 Object list

| Object | Role |
|--------|------|
| Sword | Kills dragons on contact while carried by the player |
| Gold key | Opens gold castle gate while in the same room |
| Black key | Opens black castle gate while in the same room |
| White key | Opens white castle gate while in the same room |
| Magnet | Pulls nearby items in the same room |
| Bridge | Opens authored bridge sockets |
| Chalice | Win condition |
| Secret dot | Hidden trigger for Easter egg route |

### 7.3 Sword

- If the player carries the sword and overlaps a live dragon, the dragon dies.
- Sword-on-dragon kill should have a small contact grace window so simultaneous player death does not feel random.
- Dead dragons become inert and non-hostile. They may either remain as corpses or be removed; choose one approach and use it consistently.

Recommended first version:

- Remove dead dragons from active simulation after a brief death effect.

### 7.4 Keys and gates

- A castle gate opens whenever its matching key is in the same room, regardless of who carries the key.
- Gates update immediately when the key enters or leaves the room.
- Gates should emit only one open sound on state change, not every frame while open.

### 7.5 Magnet

- The magnet only affects items in the same room.
- It does not affect dragons, the bat, or the player.
- It does not pull through solid walls or closed gates.
- Pulled items move slowly, such as 30 px/s, and stop at a small orbit distance instead of stacking at one exact point.
- The magnet has no effect while carried.

### 7.6 Bridge

- The bridge is a horizontal rectangle, about 40 x 8 px.
- It only activates authored bridge sockets in the same room.
- When carried, it may activate a socket if its carried position overlaps that socket.
- When dropped, it remains active while resting on a valid socket.
- Removing the bridge deactivates that socket immediately.

### 7.7 Chalice

- The chalice has the strongest glow and shimmer effect.
- The game is won only if the player is carrying the chalice into the gold castle throne room.
- The chalice resting on the floor inside the throne room should not trigger victory by itself.

### 7.8 Secret dot

- The secret dot is not rendered and has no glow.
- It should still have a small pickup radius, larger than 1 px, so the Easter egg is achievable without pixel-perfect movement.
- The dot is never randomized in Game 3.

## 8. Enemies

### 8.1 Dragons

| Dragon | Speed | Aggression | Notes |
|--------|-------|------------|------|
| Yorgle | 80 px/s | Low | Flees the gold key, loosely guards the chalice |
| Grundle | 120 px/s | Medium | Guards the magnet, bridge, and black key area |
| Rhindle | 160 px/s | High | Most aggressive and relentless |

Dragon state machine:

- `DRAGON_IDLE`
- `DRAGON_CHASE`
- `DRAGON_FLEE`
- `DRAGON_BITE`
- `DRAGON_DEAD`

Behavior rules:

- Dragons steer directly toward their target within a room. No full pathfinding is required.
- If the player leaves through an exit while being chased, the dragon may follow through that exit.
- Idle dragons drift around a home room or nearby linked rooms.
- Yorgle flees when the gold key is in the same room and within a defined influence range.

Recommended combat fairness rule:

- After room transition, give the player and chasing dragon a very short re-entry grace window, such as 0.2 seconds, before a bite can occur.

### 8.2 Bat

| Property | Value |
|----------|-------|
| Speed | 100-200 px/s, tunable |
| Movement | Erratic wandering with periodic heading changes |
| Carry limit | 1 bat-carryable entity |

Bat behavior:

- Roams globally from room to room
- May pick up items or dragons
- If already carrying something and it overlaps another valid target, it swaps
- May steal the player's currently carried item
- Never carries the player

Implementation guardrails:

- The bat should have a swap cooldown so it does not thrash every frame while overlapping several objects.
- While a dragon is carried by the bat, that dragon's movement and bite logic are suspended.

## 9. Game Modes

### 9.1 Game 1 - Beginner

- Reduced room subset, about 15 rooms
- No maze or dark-room complexity
- Reduced entity set
- No bat
- Intended as onboarding

### 9.2 Game 2 - Standard

- Full room set
- Full item set
- All three dragons plus bat
- Fixed spawn locations
- Canonical ruleset

### 9.3 Game 3 - Randomized

- Same room set and entity types as Game 2
- Uses curated valid spawn tables, not naive random placement

Randomization constraints:

- No spawn inside walls or closed gates
- Chalice cannot spawn in the gold castle throne room
- A key cannot spawn inside its own locked castle interior unless the design explicitly guarantees escape
- Secret dot is never randomized
- Player always starts in the gold castle start room

## 10. Audio

- No background music in the first version
- Use short retro-styled sound effects
- Prefer asset-backed `.wav` or `.ogg` files loaded through SDL3_mixer
- Procedural synthesis is optional later, not required for the first milestone

| Event | Sound direction |
|-------|-----------------|
| Pickup | Short ascending bleep |
| Drop | Short descending bleep |
| Dragon bite | Low buzz / growl |
| Dragon death | Sharp high tone plus crunch |
| Gate open | Rising chime |
| Gate close | Falling chime |
| Bat swap | Quick chirp / warble |
| Win | Short victory arpeggio |
| Wall bump | Soft thud |
| Bridge engage | Mechanical clunk |

## 11. UI

### 11.1 Title screen

- Rendered with SDL_ttf, not `SDL_RenderDebugText`, because title text needs controlled sizing
- Shows title plus three mode options
- Shows simple control hint at the bottom

Recommended text:

- `ADVENTURE`
- `1 - BEGINNER`
- `2 - STANDARD`
- `3 - RANDOMIZED`
- `MOVE: WASD OR ARROWS`
- `INTERACT: SPACE / ENTER`

### 11.2 In-game HUD

- Show elapsed time in the top-right of the playfield
- No score display
- No inventory panel in the first version

### 11.3 Victory screen

- Centered victory message
- Completion time
- Game mode label
- Prompt to return to the menu

### 11.4 Death freeze

- Freeze on the last gameplay frame
- No extra overlay required
- Optional subtle red flash or brief audio stinger is acceptable

## 12. Technical Architecture

### 12.1 Reused components

Keep and reuse these existing components where they still fit:

- `Transform`
- `Velocity`
- `Collider`
- `Collision`
- `Shape`
- `Input`

### 12.2 New components

| Component | Fields | Purpose |
|-----------|--------|---------|
| `RoomOccupant` | `int room_id` | Current room membership |
| `Player` | `ecs_entity_t carried_entity` | Player marker and carry state |
| `Dragon` | `type`, `state`, `home_room`, `target_room`, `speed`, `aggression_range`, `flee_range`, timers | Dragon AI |
| `Bat` | `ecs_entity_t carried_entity`, `heading`, `retarget_timer`, `swap_cooldown` | Bat AI |
| `Item` | `type` | Item classification |
| `Carryable` | flags for `player_can_carry`, `bat_can_carry` | Carry permissions |
| `Carried` | `ecs_entity_t carrier`, `vector2 local_offset` | Active attachment |
| `Gate` | `room_id`, `direction`, `key_type`, `open` | Gate state |
| `Glow` | `SDL_Color base_color`, `int layers`, `float radius` | Multi-layer glow rendering |
| `ParticleEmitter` | `type`, `rate`, `lifetime`, `burst_count` | FX source |

Tags:

- `Dead`
- `Invisible`
- `PlayerOnlyCarry`

### 12.3 Non-ECS game state

Use a small central `GameState` struct for scene-level state:

```c
typedef enum GameScene {
    SCENE_MENU,
    SCENE_PLAYING,
    SCENE_DEATH_FREEZE,
    SCENE_VICTORY
} GameScene;

typedef struct GameState {
    GameScene scene;
    int mode;
    float elapsed_time;
    float death_timer;
    int current_room;
    bool debug_show_colliders;
} GameState;
```

Notes:

- `current_room` is the player's room, cached for rendering
- Timer only advances during `SCENE_PLAYING`

### 12.4 Event bus updates

The current event bus is Pong-specific. Replace the event list with gameplay events such as:

- `EVENT_PLAY_SOUND`
- `EVENT_PLAYER_DIED`
- `EVENT_DRAGON_KILLED`
- `EVENT_GATE_STATE_CHANGED`
- `EVENT_GAME_WON`

Audio remains event-driven. Gameplay state transitions may be handled directly in systems or via lightweight events.

### 12.5 System pipeline

Recommended system flow:

| Phase | System | Purpose |
|-------|--------|---------|
| `EcsOnLoad` | `process_input` | Read keyboard state into singleton input |
| `EcsPreUpdate` | `collision_clear` | Remove transient collision data |
| `EcsPreUpdate` | `scene_update` | Advance timers and handle death-freeze countdown |
| `EcsOnUpdate` | `player_intent` | Convert input into player movement and interaction intent |
| `EcsOnUpdate` | `dragon_ai` | Update dragon states and desired velocity |
| `EcsOnUpdate` | `bat_ai` | Update bat movement and swap decisions |
| `EcsOnUpdate` | `magnet_pull` | Move affected items toward active magnet |
| `EcsOnUpdate` | `carry_sync` | Attach carried entities to carriers |
| `EcsOnUpdate` | `move_entities` | Apply velocity to movable entities |
| `EcsPostUpdate` | `collision_detect` | Populate `Collision` components |
| `EcsOnValidate` | `resolve_player_interactions` | Pickups, drops, swaps, sword hits |
| `EcsOnValidate` | `resolve_gate_state` | Open or close gates on state change |
| `EcsOnValidate` | `room_transition` | Move entities across room links |
| `EcsOnValidate` | `check_victory` | Detect chalice delivery |
| `EcsPreStore` | `render_clear` | Clear window and draw pillarbox bars |
| `EcsOnStore` | `render_room` | Draw walls, gates, sockets, and room tint |
| `EcsOnStore` | `render_entities` | Draw visible entities in current room |
| `EcsOnStore` | `render_fog` | Apply dark-room mask if needed |
| `EcsOnStore` | `render_particles` | Draw active effects |

UI rendering remains outside ECS in `ui_render()` after `ecs_progress()`.

### 12.6 Room data structures

Recommended room types:

```c
#define ROOM_NONE (-1)
#define MAX_ROOM_WALLS 16
#define MAX_ROOM_BRIDGE_SOCKETS 4

typedef struct WallRect {
    float x;
    float y;
    float w;
    float h;
} WallRect;

typedef struct RoomExit {
    int destination_room;
    float span_min;
    float span_max;
} RoomExit;

typedef struct BridgeSocket {
    Direction wall;
    float span_min;
    float span_max;
} BridgeSocket;

typedef enum RoomFlags {
    ROOM_FLAG_NONE = 0,
    ROOM_FLAG_DARK = 1 << 0,
    ROOM_FLAG_CASTLE = 1 << 1,
    ROOM_FLAG_SECRET = 1 << 2
} RoomFlags;

typedef struct RoomDef {
    int id;
    const char *name;
    RoomExit exits[DIR_COUNT];
    WallRect interior_walls[MAX_ROOM_WALLS];
    int interior_wall_count;
    BridgeSocket bridge_sockets[MAX_ROOM_BRIDGE_SOCKETS];
    int bridge_socket_count;
    RoomFlags flags;
    SDL_Color ambient_tint;
} RoomDef;
```

Authoring rules:

- Interior walls should be stored separately from generated boundary walls
- Exit spans are defined in room-local coordinates
- Room ids must be stable across all game modes

### 12.7 Rendering notes

- Current rectangle rendering paths can be extended instead of replaced
- Glow should be integrated as a rendering pass over visible entities, not as separate gameplay entities
- Fog of war should be clipped to the playfield only, not the whole 1280 x 720 window

## 13. Files To Create Or Modify

### New files

- `src/components/room_occupant.h`
- `src/components/player.h`
- `src/components/dragon.h`
- `src/components/bat.h`
- `src/components/item.h`
- `src/components/carryable.h`
- `src/components/carried.h`
- `src/components/gate.h`
- `src/components/glow.h`
- `src/components/particle.h`
- `src/data/rooms.h`
- `src/data/rooms.c`
- `src/data/game_modes.h`
- `src/data/game_modes.c`
- `src/systems/player.c`
- `src/systems/player.h`
- `src/systems/dragon.c`
- `src/systems/dragon.h`
- `src/systems/bat.c`
- `src/systems/bat.h`
- `src/systems/magnet.c`
- `src/systems/magnet.h`
- `src/systems/interaction.c`
- `src/systems/interaction.h`
- `src/systems/room_transition.c`
- `src/systems/room_transition.h`
- `src/systems/win.c`
- `src/systems/win.h`
- `src/systems/fog.c`
- `src/systems/fog.h`
- `src/systems/particle.c`
- `src/systems/particle.h`
- `src/menu.c`
- `src/menu.h`

### Files to modify

- `src/game.c`
- `src/game.h`
- `src/defines.h`
- `src/event_bus.h`
- `src/event_bus.c`
- `src/managers/audio.c`
- `src/managers/audio.h`
- `src/managers/component.c`
- `src/managers/component.h`
- `src/managers/entity.c`
- `src/managers/entity.h`
- `src/managers/system.c`
- `src/managers/system.h`
- `src/ui.c`
- `src/ui.h`

### Files to remove after migration

- `src/components/paddle.h`
- `src/components/ball.h`
- `src/systems/paddle.c`
- `src/systems/paddle.h`
- `src/systems/ball.c`
- `src/systems/ball.h`
- `src/score.c`
- `src/score.h`

## 14. Verification

### 14.1 Build and run

```powershell
cmake --preset default
cmake --build build
.\build\Debug\adventure.exe
```

### 14.2 Manual checklist

- Title menu appears and all three modes are selectable
- Playfield renders centered in 4:3 with black side bars
- Player moves in 8 directions with normalized diagonals
- Player cannot walk through solid walls or closed gates
- Room transitions preserve the correct edge and destination link
- Player can pick up, drop, and swap objects intentionally
- Carried objects follow the player cleanly
- Sword kills dragons reliably
- Dragon bites trigger death freeze and respawn
- Bat roams and swaps objects without rapid overlap thrashing
- Keys open and close the correct gates on state change
- Magnet pulls only valid items in the same room and respects walls
- Bridge activates only authored bridge sockets
- Dark rooms apply fog of war only inside the playfield
- Chalice carried into the throne room triggers victory
- Victory screen shows elapsed time and selected mode
- Game 3 randomization respects spawn constraints
- Secret room remains reachable
- No removed Pong systems or score UI remain active

## 15. Implementation Priorities

Suggested delivery order:

1. Core room data, player movement, room transitions, and static walls
2. Player carry system, keys, gates, chalice, and victory
3. Dragons plus sword combat
4. Bat and magnet
5. Bridge sockets and Easter egg route
6. Dark-room fog, glow polish, and particles

This sequence gets a playable vertical slice early while leaving higher-chaos systems for later integration.
