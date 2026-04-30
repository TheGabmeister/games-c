# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A 2D top-down action-adventure game inspired by The Legend of Zelda (1986), built in C with raylib. Phases 1 (Player and Tiles), 2 (World Navigation), 3 (Combat), 4 (Projectiles and Items), 5 (Dungeons), and 6 (World Systems) are complete.

Key documents:
- **SPEC.md** — complete game design specification (mechanics, items, enemies, dungeons, tuning values). Source of truth for game behavior.
- **IMPLEMENTATION.md** — code architecture, data structures, module design, file formats, phased implementation plan with test checklists. Read this before implementing any new system.
- **AGENTS.md** — additional guidance for coding agents (asset pipeline details, rfxgen commands).

## Build

```bash
# Build (uses existing build directory)
cmake --build build

# Full reconfigure + build (needed when adding new .c/.h files)
cmake -S . -B build
cmake --build build

# Release build
cmake --build build --config Release
```

Generator: Visual Studio 18 2026, x64. Executable: `build/zelda/Debug/zelda.exe`. Assets from `assets/` are copied beside the executable at build time — runtime paths are relative (e.g., `assets/sprites/player.png`). CMake uses `GLOB_RECURSE` with `CONFIGURE_DEPENDS` to discover source files, but adding new files may require a reconfigure.

Strict compiler warnings are enabled (`/W4` on MSVC, `-Wall -Wextra` on GCC/Clang). Implicit function declarations, incompatible pointer types, and missing return values are promoted to errors. This is critical in C — a missing `#include` for a function like `Clamp` silently breaks float argument passing on x64 rather than failing to compile.

## Architecture

Window: 1024x960. Logical resolution: 1024x960 (1:1, no scaling). Tile size: 64x64 pixels.

Screen layout (top to bottom): HUD 1024x224 (3.5 tiles), 32px divider, play area 1024x704 (16x11 tiles). Play area starts at y=256 (`PLAY_AREA_Y`).

`Game` struct holds gameplay state (defined in `game.h`). Self-contained modules own their own static state: sounds (`sounds.c`), music (`music.c`), VFX (`vfx.c`), textures (`textures.c`). The main loop in `main.c` calls `enemy_defs_init` -> `game_init` -> `game_update`/`game_draw` per frame -> cleanup on exit. Game state machine: `STATE_TITLE` (save slot selection), `STATE_PLAY` (normal gameplay), `STATE_TRANSITION` (scroll/fade between screens), `STATE_DEATH` (death pause + fade), `STATE_CONTINUE` (press enter to respawn), `STATE_PAUSE` (inventory/pause screen, game logic frozen), `STATE_ITEM_GET` (dungeon item pickup ceremony, 2-second hold with jingle), `STATE_DIALOGUE` (NPC text display), `STATE_SHOP` (shop purchase UI).

### Shared types

`game_config.h` is the canonical source for compile-time constants (window size, tile size, combat tuning, projectile tuning) and the `Direction` enum/`opposite_dir()` utility — used by player, camera, enemies, and projectiles. Other shared enums live in their own headers: `ItemType` and `item_type_names`/`item_display_names` tables in `items.h`, `SoundID` in `sounds.h`.

### Update loop order (STATE_PLAY)

`player_update` -> `enemies_update` -> `projectiles_update` -> `combat_check_bombs` -> `vfx_update` -> `combat_check` -> `pickups_update` -> `combat_check_pickups` -> `dungeon_check_locked_door` -> `dungeon_check_shutter_room` -> `dungeon_check_push_block` -> `dungeon_check_items` -> `world_check_push_rock` -> death check -> low health beep -> `world_check_cave_interaction` -> `nav_check_warp` -> `nav_check_edge_transition`.

Game logic is split across focused modules: `combat.c` (collision, bombs, pickups, enemy death/drops), `navigation.c` (screen loading, transitions, warps), `dungeon_room.c` (doors, shutters, push blocks, dungeon items), `world_state.c` (overworld persistent state, cave NPC services, heavy rocks, candle/bush burning). `game.c` retains `game_init`, the state machine in `game_update`, and `game_draw`.

Combat collision (`combat_check` in combat.c) handles sword-vs-enemy, player-projectile-vs-enemy (with boomerang stun), enemy-contact-vs-player, and enemy-projectile-vs-player (with shield blocking). Enemy-specific hit behavior (e.g., slime splitting) is handled via `on_hit` callbacks in the enemy vtable — combat.c calls `try_on_hit()` before applying default damage.

### Enemy system

Enemies use **vtable-style dispatch**: `EnemyDef` holds a `name` string, stat fields (`health`, `contact_damage`, `defense`, `speed`, `hitbox_w/h`, `drop_group`), and function pointers for `update`, `draw`, `on_spawn`, `on_death`, and `on_hit`, indexed by `EnemyType`. Each enemy .c file provides a single `_def()` function (e.g., `slime_def()`) that returns a populated `EnemyDef`. The def table `enemy_defs[]` is initialized at startup by `enemy_defs_init()` in `enemy.c`. Adding a new enemy means writing its .c file in `src/enemy/`, adding a `_def()` declaration in `enemy.h`, and one line in `enemy_defs_init()`. The screen file parser and game loop need no changes — the parser looks up enemy names from the def table automatically.

`on_spawn` handles type-specific initialization (e.g., dragon sets random velocity); `on_death` handles type-specific death behavior (e.g., dragon marks boss defeated); `on_hit` handles type-specific hit reactions (e.g., slime splits instead of taking damage). All three are NULL for most enemies. `defense` reduces incoming damage (min 1). `drop_group` selects from a `DropTable` array in combat.c — group 0 is the default (35% nothing, 20% rupee, 15% each heart/arrow/bomb), other groups have different distributions.

Enemy files live in `src/enemy/`. Each enemy type is a separate .c file (slime.c, bat.c, charging_snake.c, rock_spitter.c, spear_thrower.c, dragon.c) with all its functions `static` — only the `_def()` function is exposed. AI is implemented as a state machine using `EnemyState` and `state_timer`. The shared `Enemy` struct has `velocity`, `ai_timer`, and `subtype` for type-specific scratch state, plus `uint8_t extra[16]` for larger per-instance data. `variant` (int) supports stat/visual variants — enemies with `variant > 0` get doubled HP on spawn. Dragon boss uses `subtype` for its own AI states (patrol/windup/fire/cooldown) to avoid extending the shared `EnemyState` enum.

The enemy `update` function signature includes projectile array access (`Projectile *projectiles, int *projectile_count`) so ranged enemies (rock_spitter, spear_thrower) can spawn projectiles. Non-ranged enemies ignore these params.

Spawn data is parsed from screen metadata (`enemy: type col row [variant]` lines in .txt files) into `EnemySpawn` array in the `Screen` struct. The variant field is optional (defaults to 0). Enemy type names are looked up from the `name` field in `enemy_defs[]` — no hardcoded string mapping in the parser. Runtime `Enemy` instances live in `Game.enemies[]` and are re-spawned fresh on each screen load.

### Projectile system

Projectiles use the same **vtable-style dispatch** as enemies: `ProjectileDef` holds stat fields (`speed`, `damage`, `max_range`, `hitbox_w/h`, shield-blocking flags) and function pointers for `update` and `draw`, indexed by `ProjectileType`. Hitbox dimensions are data-driven — directional projectiles swap w/h for N/S facing automatically. Types: `PROJ_ARROW`, `PROJ_BOOMERANG`, `PROJ_BOMB`, `PROJ_ROCK`, `PROJ_SPEAR`, `PROJ_DRAGON_BEAM`.

Arrow, rock, and spear share a generic `linear_update`/`linear_draw` in `projectile.c`. Boomerang has unique return-to-player homing. Bomb is stationary with a fuse timer — explosion damage is handled by `combat_check_bombs` in combat.c.

Each projectile has an `owner` field (`OWNER_PLAYER` or `OWNER_ENEMY`) that determines collision layers. Player projectiles hit enemies; enemy projectiles hit the player (with shield blocking based on facing direction and shield tier).

### Pickup and drop system

`pickup.h/c` manages a fixed array of pickups (max 16 per screen). Types: `PICKUP_RUPEE`, `PICKUP_HEART`, `PICKUP_BOMB`, `PICKUP_ARROW`. When enemies die, the `on_death` vtable callback dispatches per-type behavior (e.g., `dragon_on_death` marks boss defeated). Enemies without an `on_death` callback get default drop logic in `combat.c` via data-driven `DropTable` groups (selected by `drop_group` in `EnemyDef`; group 0: 35% nothing, 20% rupee, 15% each heart/arrow/bomb). The player collects pickups by walking over them. Pickups flash and despawn after 10 seconds. Cleared on screen transition.

### VFX system

`vfx.h/c` is a self-contained visual effects module with its own static array (max 16 effects). Fire-and-forget: call `vfx_spawn()`, and the effect updates and draws itself until it expires. Currently has `VFX_EXPLOSION` (expanding 3-layer fireball, 20 frames). Adding new effect types means adding a `VfxType` enum value and a draw function in vfx.c — nothing in game.h/.c changes.

### Player combat and items

`PlayerState` includes `PSTATE_ATTACKING` (8-frame sword swing with cooldown), `PSTATE_USING_ITEM` (12-frame item use with cooldown), and `PSTATE_KNOCKBACK` (8-frame push on damage). Invulnerability lasts 60 frames with a visual flash.

`player_update` takes projectile array params so the player can spawn projectiles on item use. Item dispatch is a switch on `inventory.equipped` in `try_use_item()` within player.c. Arrows cost 1 arrow (separate ammo, not rupees). Boomerang allows only one active at a time. Bombs cost 1 bomb, max one active.

### Inventory and pause screen

`inventory.h/c` owns the pause screen and item granting. `STATE_PAUSE` freezes game logic and draws a dimmed overlay with a 5x2 item grid. Cursor navigation + confirm equips an item. The `Inventory` struct in `player.h` tracks: `items` bitfield (`uint64_t`, accessed via `item_bit()` which returns `1ULL << item`), `sword_tier`, `shield_tier`, `armor_tier`, `rupees`, `bombs`/`bomb_capacity`, `arrows`/`arrow_capacity`, `keys`, `relic_fragments`, `equipped` item. `inventory_grant()` is the single entry point for giving the player any item — used by cave gifts, shops, and dungeon item pickups. When `in_dungeon`, the pause screen also renders a dungeon map view.

### Transitions and screen loading

`trans_type` in Game struct tracks what kind of transition was started (scroll vs fade) because `camera.type` is reset to `TRANS_NONE` when the camera finishes — completion logic needs the original type to decide whether to swap `next_screen` (scroll) or not (fade, which loads directly into `current_screen` at midpoint). Cave enter/exit uses a return-stack that saves origin screen + tile position. Enemies, projectiles, pickups, and VFX are all cleared on every screen load.

### Dungeon system

`dungeon.h/c` manages dungeon state. `DungeonState` struct holds per-dungeon data: current room position, visited/cleared/lit room bitfields (`uint64_t`), permanently-unlocked door bitfields (`uint64_t doors_unlocked[4]`), item-collected bitfields (`uint64_t items_collected[4]`), map/compass/boss-defeated/fragment-collected flags. Single-word bitfield helpers use `dungeon_room_bit(rx, ry)`. Multi-word bitfields have dedicated helpers: `dungeon_door_is_unlocked`/`dungeon_door_set_unlocked` for doors, `dungeon_item_is_collected`/`dungeon_item_set_collected` for items.

The `Game` struct holds `dungeon_saves[MAX_DUNGEONS]` for persistent cross-dungeon state. On dungeon entry, permanent state (unlocked doors, collected items, boss defeated) is loaded from `dungeon_saves[id]` while visit-scoped state is cleared. On dungeon exit or death, permanent state is saved back. This allows progress in all 9 dungeons to persist independently.

Dungeon rooms are `.txt` files in `assets/dungeons/<n>/` using the same format as overworld screens plus new metadata: `door:` (direction, position, type: open/locked/shutter), `shutter:` (true), `dark:` (true), `boss:` (true), `item:` (type, col, row for dungeon-specific items like keys, map, compass, heart_container, fragment).

Three location modes are mutually exclusive: overworld (`!in_cave && !in_dungeon`), cave (`in_cave`), dungeon (`in_dungeon`). Dungeon room transitions use scroll (same as overworld) triggered at screen edges where open door tiles exist. **Dungeon Y-axis convention**: room Y increases northward (entrance at y=0, boss at higher y). This is opposite to the overworld where Y increases southward. `can_transition_dungeon` and `start_scroll_transition` flip N/S for dungeons; the HUD and pause screen minimaps flip Y when rendering dungeon rooms. Warps: `dungeon_<n>` enters a dungeon from overworld, `droom_XX_YY` warps between dungeon rooms (push block stairs), `return` exits to saved overworld position.

Door mechanics: locked and shutter doors have D tiles replaced with `TILE_DOOR_CLOSED` on load (impassable, distinct barred-door sprite). Key consumption opens locked doors permanently (both sides). Shutter rooms close all doors on entry; doors reopen when all enemies are defeated. Push blocks reveal stairs after sustained player contact (12 frames) when all enemies are dead. After scroll transitions into dungeon rooms, the player position is nudged inward if it overlaps a blocked tile (prevents spawning inside walled-off shutter doors).

Dark rooms draw a black overlay hiding everything; candle item use lights the room for the current dungeon visit. `STATE_ITEM_GET` freezes gameplay for 2 seconds to display collected dungeon items (map, compass, heart container, fragment) with a jingle — display names come from the `item_display_names` table in `items.h`. Dungeon items render on the floor using dedicated sprites (`item_key.png`, `item_map.png`, `item_compass.png`, `item_heart_container.png`, `item_fragment.png`); missing sprites trigger an assert (no silent fallbacks). Boss room items (heart container, fragment) are hidden until `boss_defeated` is set.

**Dragon guardian boss** (`src/enemy/dragon.c`): 2x2 tile enemy using `subtype` field for AI states (patrol/windup/fire/cooldown). 12 HP, fires `PROJ_DRAGON_BEAM` projectiles at player. On death, marks `boss_defeated` in DungeonState; does not drop loot (rewards are floor items revealed after boss death).

Death in dungeon respawns at the dungeon entrance room with 3 hearts. Visit-scoped state (cleared rooms, opened shutters, lit rooms) resets; permanent state (unlocked doors, collected items, boss defeated) persists.

HUD and pause screen show dungeon room minimap when `in_dungeon`. Map item reveals all rooms; compass marks the boss room.

Dungeon 1 (Forest Shrine): 8 rooms at `assets/dungeons/1/`. Layout: entrance(0,0) → key room(0,1) → shutter room(1,1), map room(0,2) → compass room(1,2) → dark room(1,3), push block room(0,3) → boss room(0,4).

### Input abstraction

`input.h/c` wraps all player input (keyboard + gamepad) behind named functions. Held-state functions (`input_left`, `input_right`, `input_up`, `input_down`) for player movement. Single-press functions (`input_left_pressed`, `input_right_pressed`, `input_up_pressed`, `input_down_pressed`) for menu cursor navigation. Action functions (`input_attack`, `input_confirm`, `input_pause`, etc.) for discrete actions. All game code uses these instead of raw raylib key checks. Gamepad deadzone is `GAMEPAD_DEADZONE` in `game_config.h`.

### Animation system

`anim.h/c` provides a lightweight frame animation system. `AnimDef` describes an animation (first frame, count, duration, looping). `Anim` tracks playback state. Used by the player sprite system (`player_idle_anims`, `player_walk_anims` arrays indexed by `Direction`).

### Title screen and save system

`title.h/c` manages the title/save-select screen (`STATE_TITLE`). Displays 3 save slots with summary info (health, rupees, location). Player selects a slot to load or start a new game.

`save.h/c` handles binary save/load. `SaveData` struct (version 2) is written as a flat binary blob to `saves/slot_N.sav` with magic number (`0x3144565A`) and version validation. Saves player position, inventory, `WorldState`, `dungeon_saves[MAX_DUNGEONS]` (all dungeon persistent state), and active dungeon state. Saving during gameplay is triggered from the pause screen (F5/restart key). Caves are saved as the overworld screen the player entered from (never saves mid-cave). Any structural change to saved types (Inventory, WorldState, DungeonState) requires bumping `SAVE_VERSION`.

### World interaction system

`world_state.h/c` owns persistent overworld state via `WorldState`: bitfield arrays for bombed walls and burned bushes (per-screen, `uint64_t[WORLD_FLAG_WORDS]`), plus service flag arrays for gifts taken, shops spent, NPCs triggered, and upgrades taken (`uint64_t[WORLD_SERVICE_WORDS]`, capacity `WORLD_SERVICE_MAX` = 128 per category). All indexed by screen position or service ID.

Key behaviors:
- **Bombable walls**: when a bomb explodes near a `TILE_BOMBABLE_WALL`, the wall becomes floor and the screen is flagged so it stays open on revisit.
- **Bush burning**: candle use removes all `TILE_BUSH` tiles on the current screen and flags it permanently.
- **Heavy rocks**: bracelet item lets the player push `TILE_HEAVY_ROCK` tiles (12-frame sustained contact, same pattern as dungeon push blocks). Rocks are only pushable once per screen visit.
- **Cave NPC services**: when in a cave with an NPC, pressing confirm near the NPC triggers dialogue, gift, upgrade, or shop interaction based on cave metadata. Each service has a unique ID tracked in `WorldState` to prevent re-collection.

`world_apply_screen_flags` re-applies persistent state (bombed walls, burned bushes) after loading a screen — called from `nav_load_screen` (which handles both direct loads and save-game restores).

### Cave NPC services (dialogue, gift, upgrade, shop)

Cave screen files support four service metadata types, each with a unique ID for persistence:

- `dialogue: <id> | <text>` — NPC shows text when spoken to.
- `gift: <id> <item_name> <amount> | <text>` — one-time item reward. Items resolved via `item_type_names` table.
- `upgrade: <id> <type> <tier> <required_max_health> | <text>` — equipment upgrade gated by health requirement. Type is one of: `sword`, `shield`, `armor`, `bomb_capacity`, `arrow_capacity`.
- `shop: <id> <item1> <price1> [<item2> <price2> ...]` — up to 3 items for sale. Tracked to prevent repeat purchases.

Text after `|` is the NPC dialogue shown to the player. `dialogue.h/c` handles `STATE_DIALOGUE` (text box display). `shop.h/c` handles `STATE_SHOP` (cursor-based item selection, rupee cost, inventory grants).

Cave NPCs render at a data-driven position (`npc_col`/`npc_row` fields in `Screen`, default col 8 row 5, overridable via `npc: col row` in screen files) using `TEX_NPC_OLD_MAN` with a colored-rectangle fallback.

### Item-gated tile passability

`TileDef` has `pass_requires` and `requires_interaction` fields. `pass_requires` specifies which item unlocks the tile: `TILE_WATER` requires `ITEM_RAFT` (must enter from a `TILE_DOCK`), `TILE_GAP` requires `ITEM_LADDER`. `requires_interaction` (true for `TILE_BUSH` and `TILE_HEAVY_ROCK`) means the tile blocks movement even when the player has the required item — the item enables an interaction (candle burns bushes, bracelet pushes rocks) rather than direct passage. Player movement uses `screen_tile_blocked_for_items` which checks both fields. Additional tile chars in screen files: `=` (dock), `G` (gap), `R` (heavy rock), `T` (bush).

### Debug overlay

`debug.c/.h` provides an F3-toggled overlay showing enemy count, alive/dead status, per-enemy type/state/position, player state, HP, and active projectile count. Yellow wireframe boxes mark enemy positions. Off by default.

## Coding Principles

- **C game programming best practices** — prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime. Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job. But a plain `if` that must be copy-pasted into every new feature is not simple — it's a maintenance trap.
- **YAGNI** — don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.
- **Locality of change** — adding a new entity, tile, or feature should require changes in as few files as possible. Prefer data-driven dispatch (flags, vtables) over centralized type switches when the set of types is expected to grow.
- **Dispatch strategy** — function pointers (vtable-style def tables) for large/growing type sets (enemies, projectiles). Switch statements for small/stable state machines (game state, player state).

When in doubt: for code one person owns and rarely changes, lean KISS. For interfaces many contributors touch, lean locality of change.

### Include discipline

Files in `src/enemy/` use relative paths for project headers (`../tilemap.h`, `../textures.h`) but standard include paths for vendor headers (`"raylib.h"`, `"raymath.h"`). Any file that calls raylib math functions like `Clamp` **must** include `"raymath.h"` — without it, MSVC silently treats the call as an implicit int function, passing float arguments through integer registers (garbage in, garbage out). The `/we4013` flag now catches this at compile time.

## Asset Pipeline

- **Sprites**: each entity/item gets its own SVG and PNG file on a 64px grid, exported via Inkscape. Single-frame sprites are 64x64; directional sprites (arrow, spear) are 256x64 (4 frames: S, N, E, W). Store SVG and PNG in `assets/sprites/`.
  - Enemies: `slime.png`, `bat.png`, `snake.png`, `rock_spitter.png` (placeholder), `spear_thrower.png` (placeholder)
  - Projectiles: `arrow.png` (4-dir), `boomerang.png`, `bomb.png`, `rock.png`, `spear.png` (4-dir)
  - Pickups: `pickup_rupee.png`, `pickup_heart.png`, `pickup_bomb.png`, `pickup_arrow.png`
  - Dungeon items: `item_key.png`, `item_map.png`, `item_compass.png`, `item_heart_container.png`, `item_fragment.png`
  - NPCs: `npc_old_man.png` (cave NPC, colored-rectangle fallback if missing)
  - Tiles: `tiles.png` is a spritesheet (4 columns). Indices: 0=floor, 1=wall, 2=water1, 3=water2, 4=door, 5=pushblock/heavy rock, 6=stairs, 7=bombable wall, 8=closed door, 9=dock, 10=gap. Bush reuses index 1 (wall sprite).
  - Dungeon item draw functions assert on missing textures. Other draw functions (enemies, projectiles, pickups) have colored-rectangle fallbacks.
  - Inkscape path: `"/c/Program Files/Inkscape/bin/inkscape.exe"`. Export: `inkscape input.svg --export-type=png --export-filename=output.png -w 64 -h 64`.
- **Sounds**: generate with rfxgen (`"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav`). Presets: coin, laser, explosion, powerup, hit, jump, blip. Store WAV in `assets/`.
- **Music**: OGG files in `assets/music/`. Managed by `music.c` (module-owned static state). API: `music_init`, `music_update`, `music_enter_dungeon(id)`, `music_exit_dungeon`, `music_set_boss(bool)`, `music_set_biome(int biome_id)`, `music_cleanup`. Dungeon streams are properly unloaded before loading new ones on re-entry (no leaks). `music_set_biome` switches overworld tracks by loading `overworld_<id>.ogg` (falls back to `overworld.ogg` for biome 0). Composition pipeline: Python scripts in `tools/music/` use `midiutil` to generate MIDI -> FluidSynth renders with a soundfont to WAV -> ffmpeg converts to OGG. See `tools/music/compose_overworld.py` for the pattern. Tool paths: `D:/fluidsynth-v2.5.4-win10-x64-cpp11/bin/fluidsynth.exe`, `D:/ffmpeg-8.1-essentials_build/bin/ffmpeg.exe`, soundfont `D:/8bitsf.SF2`. Render command: `fluidsynth -ni -F out.wav soundfont.sf2 input.mid`.
- **Screen data**: plain text files in `assets/screens/` (overworld), `assets/dungeons/<n>/` (dungeons), `assets/caves/` (caves). Format: optional metadata lines (`warp:`, `enemy:`, `door:`, `item:`, `shutter:`, `dark:`, `boss:`, `dialogue:`, `gift:`, `upgrade:`, `shop:`, `npc:`, `#` comments), then 11 rows of **exactly 16** tile characters (W=wall, .=floor, ~=water, D=door, P=pushblock, S=stairs, B=bombable wall, ==dock, G=gap, R=heavy rock, T=bush). Rows shorter than 16 characters are silently skipped by the parser, causing `screen_load` to return false. Enemy spawn format: `enemy: type col row [variant]`. Door format: `door: direction position type`. Item format: `item: type col row`. Warp format: `warp: type col row -> dest`. Cave service formats: `dialogue: <id> | <text>`, `gift: <id> <item> <amount> | <text>`, `upgrade: <id> <type> <tier> <health_req> | <text>` (type: sword/shield/armor/bomb_capacity/arrow_capacity), `shop: <id> <item1> <price1> [<item2> <price2> ...]`. NPC position format: `npc: col row` (defaults to 8 5 if omitted). **Door tile consistency**: every `door:` metadata line must have matching `D` tiles in the tile grid at the correct edge position, and vice versa.
- Sound and music loading is resilient — missing files are skipped.
