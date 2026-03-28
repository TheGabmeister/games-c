# Adventure - Game Specification

A modernized reimagining of Adventure (Atari 2600, 1980) built in C11 with Flecs ECS, SDL3, SDL3_mixer, and SDL3_ttf.

This spec replaces the current Pong gameplay with a single-screen action-adventure game while preserving the repo's general style:

- ECS-driven simulation
- Explicit system phases
- SDL primitive rendering
- Simple, deterministic game rules

## 1. Vision

Recreate the core loop of the original Adventure:

- Explore a network of single-screen rooms
- Collect and carry one object at a time
- Avoid or defeat dragons
- Recover the chalice and return it to the gold castle

Modernization goals:

- 8-direction movement
- Clear rectangular visual language
- Dark-room fog of war
- More readable maze differentiation than the Atari original
- Consistent controls that preserve the tension of one-item carry

Non-goals for the first playable version:

- Shader-based bloom
- Procedural world generation
- Save/load support
- Multiplayer
- In-game room editor

## 2. Presentation

| Property | Value |
|----------|-------|
| Window size | 960 x 720 |
| Playfield | 960 x 720 |
| Playfield origin | `(0, 0)` |
| Framing | Playfield fills the full window |
| Background | Black |
| Window title | `Adventure` |
| Art style | Stylized rectangles with bright cores, outline accents, and lightweight particles |

### 2.1 Visual style

- Walls, items, creatures, and room features are rendered as rectangles or small compositions of rectangles.
- Glow is faked with layered translucent geometry, not post-processing.
- Dark rooms are rendered normally first, then covered by a black overlay with a circular transparent cutout centered on the player.
- Maze rooms should be distinguishable through small tint and layout changes, not by adding noisy decoration.

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

### 2.3 Entity dimensions

Reference sizes for collision and rendering:

| Entity | Size (px) | Notes |
|--------|-----------|-------|
| Player | 16 x 16 | Square |
| Yorgle | 28 x 20 | Wider body |
| Grundle | 24 x 18 | Slightly smaller body |
| Rhindle | 24 x 18 | Same silhouette as Grundle |
| Bat body | 12 x 10 | Core rectangle |
| Bat wings | 24 x 6 each side | Flap via vertical offset |
| Key | 10 x 18 | Vertical body plus small top crossbar |
| Sword | 6 x 28 | Tall narrow rectangle |
| Magnet | 16 x 16 | U-shape from 3 rectangles |
| Bridge | 40 x 8 | Horizontal plank |
| Chalice | 12 x 20 | Body plus wider base |
| Secret dot | 6 x 6 | Invisible pickup footprint |

These are starting values only. Tune during play so they feel correct relative to 16 px wall thickness and exit widths.

### 2.4 Particles

Particles are tiny rectangles with position, velocity, lifetime, and alpha fade.

| Trigger | Count | Lifetime | Behavior |
|---------|-------|----------|----------|
| Item pickup | 6-8 | 0.3 s | Small burst outward |
| Item drop | 4-6 | 0.2 s | Small downward burst |
| Dragon death | 12-16 | 0.5 s | Larger burst in dragon color |
| Player death | 8-10 | 0.4 s | Red burst |
| Gate open | 6 | 0.3 s | Rise from gate span |
| Gate close | 4 | 0.2 s | Fall from gate span |
| Chalice shimmer | 1-2/frame | 0.6 s | Slow golden drift |
| Player trail | 1 every 3 frames | 0.15 s | Quick fade |
| Bat swap | 6 | 0.3 s | Purple burst |

Particle size:

- 2 x 2 or 3 x 3 px
- No rotation
- Typical speed 40-120 px/s

## 3. Scenes And Core Loop

Game scenes:

- `SCENE_MENU`
- `SCENE_PLAYING`
- `SCENE_PAUSED`
- `SCENE_DEATH_FREEZE`
- `SCENE_VICTORY`

Core gameplay loop:

1. Poll SDL events
2. Update input singleton
3. Simulate gameplay if the active scene allows it
4. Resolve interactions and transitions
5. Render current room and visible entities
6. Render HUD or scene overlay
7. Present the frame

Win condition:

- The player carries the chalice into the gold castle throne room.

Failure model:

- There is no permanent fail state.
- Dragon bites cause a temporary death freeze, item drop, and respawn.

## 4. World Structure

### 4.1 Rooms

- Full world contains about 30 single-screen rooms.
- Game 1 uses a curated subset of those rooms.
- Each room has up to four exits.
- Exit links do not need to be symmetric.
- Maze sections may intentionally use confusing topology.

### 4.2 Coordinates

- Room-local coordinates use playfield space: `0..959` on X and `0..719` on Y.
- Entities store room-local positions in `Transform`.
- Each dynamic entity also stores a room id separately.
- Rendering translates room-local coordinates into window coordinates by adding the playfield origin.

### 4.3 Room authoring model

Use a hybrid geometry model:

- Boundary walls generated from room perimeter plus exit gaps
- Interior walls authored as axis-aligned rectangles
- Optional bridge sockets authored as special passable wall spans

This is preferred over a tile grid because it:

- Matches the intended rectangular art style
- Fits the repo's current rectangle collision system
- Keeps room data compact and readable
- Supports bridge and gate mechanics cleanly

### 4.4 Data source

- Room data is compiled into the game as const arrays.
- Use declarations in headers and definitions in `.c` files to avoid duplicated storage.
- No JSON or runtime parsing is required for the first version.

Recommended data files:

- `src/data/rooms.h`
- `src/data/rooms.c`
- `src/data/game_modes.h`
- `src/data/game_modes.c`

### 4.5 Room categories

| Category | Approx. count | Notes |
|----------|----------------|------|
| Overworld | 10-12 | Open connective rooms |
| Gold castle | 3-4 | Includes start room and throne room |
| Black castle | 3-4 | Includes locked interior |
| White castle | 3-4 | Includes locked interior |
| Maze / catacombs | 6-8 | Some dark, some visually similar |
| Secret room | 1 | Easter egg room |

## 5. Room Geometry And Transitions

### 5.1 Direction order

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

Each exit stores:

- Destination room id
- Gap span along the relevant edge

Transition rules:

- Transition occurs when an entity crosses an open exit span.
- Arrival uses a shared inset constant such as `ROOM_TRANSITION_INSET = 12.0f`.
- The perpendicular coordinate is preserved and clamped to the destination exit span.
- Player, dragons, and bat all use the same transition logic.

### 5.3 Walls

Boundary walls:

- Generated automatically from room size and exit gaps
- Wall thickness is 16 px
- Walls occupy the inside edge of the playfield
- Exit gaps should usually be at least 32 px wide

Interior walls:

- Axis-aligned rectangles
- Must not overlap exit gaps
- Must leave enough clearance for the player, dragons, and bat

### 5.4 Gates

Castle gates are special wall spans at room entrances.

Rules:

- A gate is either open or closed
- Closed gates are solid
- Open gates remove collision only for the gate span
- If a gate closes while an entity overlaps it, push that entity to the nearest valid side
- Gate state changes only on state transition, not continuously every frame

Rendering:

- Closed gate uses the matching key color with a subtle pulse
- Open gate is not drawn
- Gate state changes are instant, with sound and particles

### 5.5 Bridge sockets

The bridge should not disable arbitrary wall pixels on contact. That would make room authoring and collision brittle.

Instead:

- A room may define one or more bridge sockets
- A bridge socket references a specific wall span
- If the bridge overlaps a valid socket closely enough, that socket becomes passable
- Only authored bridge sockets may be opened

This keeps the mechanic deterministic and testable.

## 6. Player

| Property | Value |
|----------|-------|
| Shape | 16 x 16 square |
| Color | Bright yellow / gold |
| Move speed | 200 px/s, tunable |
| Movement | 8-direction, normalized diagonals |
| Carry limit | 1 player-carryable object |

### 6.1 Controls

Gameplay:

- `WASD` or arrow keys: move
- `Space` or `Enter`: interact, pick up, swap, or drop
- `Escape` or `P`: pause / resume
- `Q` on pause overlay: quit to menu

Menus:

- Up/down: selection
- Enter or Space: confirm
- Escape: back where applicable

Explicit interact is preferred over auto-pickup because it:

- Better matches original Adventure
- Prevents accidental swaps
- Makes bridge placement reliable
- Avoids magnet-related frustration

### 6.2 Interaction rules

If the player presses interact:

- While empty-handed and overlapping a carryable object: pick it up
- While carrying an object and overlapping another carryable object: swap them
- While carrying an object and not overlapping another carryable object: drop the current object

Drop placement:

- Prefer in front of the player
- Fall back to nearest valid nearby position
- Never place inside walls, gates, or solid colliders

### 6.3 Death and respawn

On dragon bite:

1. Switch to `SCENE_DEATH_FREEZE`
2. Freeze simulation and timer for about 1.5 s
3. Drop carried object if possible
4. Respawn player in the gold castle start room
5. Resume `SCENE_PLAYING`

Persistent world rules:

- Items remain where they are
- Dragons remain where they are
- Bat remains where it is
- Gates retain their current open/closed state

## 7. Carryable Objects

### 7.1 Carry model

Separate the concepts:

- `Carryable`: an entity may be carried
- `Carried`: an entity is currently attached to a carrier

Carrier rules:

- Player may carry items and the secret dot
- Bat may carry items and dragons
- Player may not carry dragons
- A carried entity inherits the carrier's room id

Rendering offsets:

- Player-carried items render 20 px below player center
- Bat-carried entities render 12 px below bat center

### 7.2 Object list

| Object | Role |
|--------|------|
| Sword | Kills dragons while carried by the player |
| Gold key | Opens gold castle gate when in same room |
| Black key | Opens black castle gate when in same room |
| White key | Opens white castle gate when in same room |
| Magnet | Pulls nearby items in same room |
| Bridge | Opens authored bridge sockets |
| Chalice | Win condition |
| Secret dot | Hidden Easter egg trigger |

### 7.3 Sword

- If the player carries the sword and overlaps a live dragon, the dragon dies.
- Sword kill has priority over bite in the same frame.
- Dead dragons should not remain active threats.

Recommended first version:

- Remove the dragon after a brief death effect.

### 7.4 Keys and gates

- A gate opens whenever its matching key is in the same room
- It does not matter whether the key is carried, dropped, or held by the bat
- Gate sound plays only on state transition

### 7.5 Magnet

- Affects only items in the same room
- Does not affect dragons, bat, or player
- Does not pull through solid walls or closed gates
- Has no effect while carried
- Pulled items should settle into a small orbit distance instead of stacking at one point

### 7.6 Bridge

- Size is about 40 x 8
- Activates only authored bridge sockets in the same room
- May activate while carried if the carried position overlaps the socket
- Remains active while dropped on a valid socket
- Deactivates immediately when moved away

### 7.7 Chalice

- Strongest glow and shimmer
- Only wins the game when carried by the player into the throne room
- Resting on the floor in the throne room does not count

### 7.8 Secret dot

- Not rendered
- No glow
- Pickup area should be larger than a literal 1 px point
- Never randomized in Game 3
- Room data should include author comments identifying the dot location and target wall

## 8. Enemies

### 8.1 Dragons

| Dragon | Speed | Aggression | Notes |
|--------|-------|------------|------|
| Yorgle | 80 px/s | Low | Flees gold key, loosely guards chalice |
| Grundle | 120 px/s | Medium | Guards black-key-area valuables |
| Rhindle | 160 px/s | High | Fastest and most relentless |

Dragon states:

- `DRAGON_IDLE`
- `DRAGON_CHASE`
- `DRAGON_FLEE`
- `DRAGON_BITE`
- `DRAGON_DEAD`

Behavior rules:

- No full pathfinding inside a room; use direct steering
- Chasing dragons may follow through exits
- Idle dragons wander around home room or nearby rooms
- Yorgle flees if gold key is nearby in the same room

Cross-room navigation:

- Precompute a simple room-distance table at startup
- A dragon chasing a player in another room heads toward the exit whose destination minimizes distance to the player's room
- Ties are broken by choosing the nearest exit in local space

Fairness rule:

- After room transition, use a short bite grace period such as 0.2 s

### 8.2 Bat

| Property | Value |
|----------|-------|
| Speed | 100-200 px/s, tunable |
| Movement | Erratic wandering with periodic heading changes |
| Carry limit | 1 bat-carryable entity |

Bat behavior:

- Roams from room to room
- May pick up items or dragons
- If already carrying something and overlapping another valid target, swaps
- May steal the player's carried item
- Never carries the player

Guardrails:

- Must have a swap cooldown so it does not thrash every frame
- A carried dragon has movement and bite logic suspended
- A dropped dragon re-enters `DRAGON_IDLE`, then behaves normally

## 9. Game Modes

### 9.1 Game 1 - Beginner

- About 15 rooms
- No maze complexity
- No dark rooms
- Reduced item set
- Reduced dragon set
- No bat

### 9.2 Game 2 - Standard

- Full room set
- Full item set
- All three dragons and bat
- Fixed placements

### 9.3 Game 3 - Randomized

- Same room set and entity types as Game 2
- Uses curated valid spawn tables, not naive random placement

Randomization constraints:

- No spawn inside walls or closed gates
- Chalice cannot spawn in throne room
- A key cannot spawn in a location that soft-locks progression
- Secret dot is never randomized
- Player always starts in the gold castle start room

## 10. Audio

- No background music for first version
- Use short retro-styled sound effects
- Prefer asset-backed `.wav` or `.ogg` files via SDL3_mixer
- Procedural synthesis is optional later

Asset plan:

- Place sounds under `assets/sfx/`
- Name by event, such as `pickup.wav`, `drop.wav`, `dragon_bite.wav`, `dragon_death.wav`, `gate_open.wav`, `gate_close.wav`, `bat_swap.wav`, `win.wav`, `wall_bump.wav`, `bridge.wav`
- Existing `assets/sfx_bump.ogg` may be removed or repurposed

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

- Render with SDL_ttf
- Use `assets/fonts/press_start_2p/PressStart2P-Regular.ttf`
- Use the same font family for title, HUD, and victory text
- Show mode options and basic controls

Recommended text:

- `ADVENTURE`
- `1 - BEGINNER`
- `2 - STANDARD`
- `3 - RANDOMIZED`
- `MOVE: WASD OR ARROWS`
- `INTERACT: SPACE / ENTER`

### 11.2 In-game HUD

- Show elapsed time in top-right of playfield
- Format as `MM:SS`
- Timer advances only during `SCENE_PLAYING`
- No score display
- No inventory panel in first version

### 11.3 Victory screen

- Centered victory message
- Completion time
- Game mode label
- Prompt to return to menu

Recommended time format:

- `MM:SS.t`

### 11.4 Death freeze

- Freeze on last gameplay frame
- No mandatory overlay
- Optional brief red flash is acceptable

### 11.5 Pause overlay

- Show `PAUSED` centered on playfield
- Show `RESUME: ESC / P`
- Show `QUIT: Q`
- Dim the playfield with a translucent black overlay

## 12. Technical Architecture

### 12.1 Reused components

Keep and reuse where still appropriate:

- `Transform`
- `Velocity`
- `Collider`
- `Collision`
- `Shape`
- `Input` as the singleton component name only

Note:

- The current `Input` fields are Pong-specific and must be replaced

### 12.2 Input singleton

Recommended structure:

```c
typedef struct Input {
    bool quit;
    bool pause_pressed;
    bool interact_pressed;
    bool debug_toggle_collision_pressed;
    bool debug_toggle_room_info_pressed;
    bool debug_toggle_entity_list_pressed;
    float move_x;
    float move_y;
} Input;
```

Rules:

- `move_x` and `move_y` are raw directional intent in `[-1, 0, 1]`
- Diagonal normalization happens in gameplay logic
- `*_pressed` fields are edge-triggered for one frame
- `process_input` remains the only system that writes the singleton

### 12.3 New components

| Component | Fields | Purpose |
|-----------|--------|---------|
| `RoomOccupant` | `int room_id` | Current room membership |
| `Player` | `ecs_entity_t carried_entity` | Player tag plus carry state |
| `Dragon` | `type`, `state`, `home_room`, `speed`, ranges, timers | Dragon AI |
| `Bat` | `ecs_entity_t carried_entity`, `heading`, `retarget_timer`, `swap_cooldown` | Bat AI |
| `Item` | `type` | Item classification |
| `Carryable` | `player_can_carry`, `bat_can_carry` | Carry permissions |
| `Carried` | `ecs_entity_t carrier`, `vector2 local_offset` | Active attachment |
| `Gate` | `room_id`, `direction`, `key_type`, `open` | Gate state |
| `Glow` | `SDL_Color base_color`, `int layers`, `float radius` | Glow rendering |
| `ParticleEmitter` | `type`, `rate`, `lifetime`, `burst_count` | FX source or helper |

Tags:

- `Dead`
- `Invisible`
- `PlayerOnlyCarry`

### 12.4 Non-ECS game state

Use a small central `GameState` struct:

```c
typedef enum GameScene {
    SCENE_MENU,
    SCENE_PLAYING,
    SCENE_PAUSED,
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
    bool debug_show_room_info;
    bool debug_show_entity_list;
} GameState;
```

Notes:

- `current_room` caches the player's room for rendering
- Timer advances only during `SCENE_PLAYING`
- Pause and death-freeze live in `GameState.scene`, not as ECS tags

### 12.5 Event bus updates

The current event bus is Pong-specific. Replace its event list with gameplay-oriented events such as:

- `EVENT_PLAY_SOUND`
- `EVENT_PLAYER_DIED`
- `EVENT_DRAGON_KILLED`
- `EVENT_GATE_STATE_CHANGED`
- `EVENT_GAME_WON`

Audio should remain event-driven. Scene changes may be direct or event-based.

### 12.6 System pipeline

Recommended system flow:

| Phase | System | Purpose |
|-------|--------|---------|
| `EcsOnLoad` | `process_input` | Read keyboard state into input singleton |
| `EcsPreUpdate` | `collision_clear` | Remove transient collision data |
| `EcsPreUpdate` | `scene_update` | Advance death/pause timers and scene gates |
| `EcsOnUpdate` | `player_intent` | Convert input to movement and action intent |
| `EcsOnUpdate` | `dragon_ai` | Update dragon behavior |
| `EcsOnUpdate` | `bat_ai` | Update bat movement and swaps |
| `EcsOnUpdate` | `magnet_pull` | Pull valid items toward active magnet |
| `EcsOnUpdate` | `carry_sync` | Attach carried entities to carriers |
| `EcsOnUpdate` | `move_entities` | Apply velocity |
| `EcsPostUpdate` | `collision_detect` | Populate `Collision` components |
| `EcsOnValidate` | `resolve_player_interactions` | Pickup, drop, swap, sword hits |
| `EcsOnValidate` | `resolve_gate_state` | Open or close gates on change |
| `EcsOnValidate` | `room_transition` | Move entities between rooms |
| `EcsOnValidate` | `check_victory` | Detect chalice delivery |
| `EcsPreStore` | `render_clear` | Clear window |
| `EcsOnStore` | `render_room` | Draw room geometry and gates |
| `EcsOnStore` | `render_entities` | Draw visible entities |
| `EcsOnStore` | `render_fog` | Draw dark-room overlay if needed |
| `EcsOnStore` | `render_particles` | Draw active particles |

UI rendering remains outside ECS in `ui_render()` after `ecs_progress()`.

Scene gating rule:

- Systems that mutate gameplay must early-out unless `scene == SCENE_PLAYING`
- Render systems continue to run during pause, death freeze, and victory
- `scene_update` is the only scene-management system that may run while not playing

### 12.7 Collision layers

Use bitmask filtering from the existing collision system:

```c
#define COL_LAYER_PLAYER  (1 << 0)
#define COL_LAYER_WALL    (1 << 1)
#define COL_LAYER_DRAGON  (1 << 2)
#define COL_LAYER_ITEM    (1 << 3)
#define COL_LAYER_BAT     (1 << 4)
```

| Entity | Layer | Mask |
|--------|-------|------|
| Player | `PLAYER` | `WALL | DRAGON | ITEM | BAT` |
| Dragon | `DRAGON` | `WALL | PLAYER | BAT` |
| Item | `ITEM` | `WALL | PLAYER | BAT` |
| Bat | `BAT` | `WALL | PLAYER | DRAGON | ITEM` |
| Wall | `WALL` | none required |

Notes:

- Items do not collide with each other
- Items do not collide with dragons
- Carried entities disable their collider while attached
- Gate wall spans use `COL_LAYER_WALL`

### 12.8 Interaction priority

When multiple interactions happen in the same frame, use this priority:

1. `Sword kills dragon`
2. `Dragon bites player`
3. `Bat steal / swap`
4. `Player interact`

This prevents the player from dying on the same frame a sword kill should have succeeded.

### 12.9 Room data structures

Recommended room definitions:

```c
#define ROOM_NONE (-1)
#define MAX_ROOM_WALLS 16
#define MAX_ROOM_BRIDGE_SOCKETS 4
#define ROOM_TRANSITION_INSET 12.0f

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
    ROOM_FLAG_NONE   = 0,
    ROOM_FLAG_DARK   = 1 << 0,
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

- Interior walls are stored separately from generated boundary walls
- Exit spans are authored in room-local coordinates
- Room ids must remain stable across all game modes

### 12.10 Rendering notes

- Extend the current rectangle rendering path instead of replacing it
- Glow is a rendering concern, not a gameplay entity type
- Fog overlay must be clipped to the playfield only
- Room-local coordinates must be translated by `PLAYFIELD_X` and `PLAYFIELD_Y` before SDL draw calls

### 12.11 Debug controls

Retain and extend debug helpers:

- `F1`: toggle collision overlay
- `F2`: toggle room info overlay
- `F3`: toggle entity list overlay

Debug overlays render in `ui_render()` above the gameplay frame.

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
- `CLAUDE.md`
- `AGENTS.md`

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
- Window matches the playfield size exactly
- Player moves in 8 directions with normalized diagonals
- Player cannot walk through walls or closed gates
- Room transitions use the correct destination and edge behavior
- Player can pick up, drop, and swap objects intentionally
- Carried objects follow the player or bat cleanly
- Sword kills dragons reliably
- Dragon bites trigger death freeze and respawn
- Bat roams and swaps without overlap thrashing
- Keys open and close the correct gates
- Magnet pulls only valid items and respects walls
- Bridge activates only authored sockets
- Dark rooms apply fog only inside playfield
- Chalice carried into throne room triggers victory
- Victory screen shows time and mode
- Game 3 randomization respects spawn constraints
- Secret room remains reachable
- Removed Pong systems and score UI are no longer active

## 15. Implementation Priorities

Recommended delivery order:

1. Room data, playfield framing, player movement, and room transitions
2. Carry system, keys, gates, chalice, and victory
3. Dragons and sword combat
4. Bat and magnet
5. Bridge sockets and Easter egg route
6. Fog, glow polish, and particles

This order gets a playable vertical slice early and pushes the highest-chaos systems later, when the core world rules are already stable.
