# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Project

This is a 2D top-down action-adventure game inspired by The Legend of Zelda
(1986), built in C with raylib and CMake.

Phases 1 through 5 are complete:

- Phase 1: player movement, animation, HUD, tile rendering, and tile collision.
- Phase 2: overworld screen loading, scroll transitions, cave/fade transitions,
  and music playback.
- Phase 3: combat, enemies, sword attacks, damage, knockback, drops, and combat
  sounds.
- Phase 4: projectiles, active items, bombs, pickups, pause inventory, and
  shield blocking.
- Phase 5: Dungeon 1, locked/shutter doors, push blocks, dark rooms, dungeon
  item pickups, boss flow, dungeon HUD/map, and dungeon music.

Phase 6, World Systems, is next. Do not skip ahead to later content unless the
current request explicitly asks for it.

Key documents:

- `SPEC.md` is the source of truth for game behavior, mechanics, items,
  enemies, dungeons, tuning values, and progression.
- `IMPLEMENTATION.md` describes the planned code architecture, data
  structures, module boundaries, file formats, implementation phases, and test
  checklists.
- `AGENTS.md` gives practical repository guidance for coding agents.

Before implementing a new gameplay system, read the relevant section of
`SPEC.md` and the architecture notes in `IMPLEMENTATION.md`.

## Build

Use the existing build directory when present:

```bash
cmake --build build
```

If the build directory is missing, stale, or new `.c` / `.h` files were added,
configure first:

```bash
cmake -S . -B build
cmake --build build
```

For a release build:

```bash
cmake --build build --config Release
```

The default generator is Visual Studio 18 2026, x64. The executable is emitted
under `build/zelda/Debug/zelda.exe` or `build/zelda/Release/zelda.exe`.
Assets under `assets/` are copied beside the executable by `CMakeLists.txt`, so
runtime asset paths should be relative to that output directory, for example
`assets/sprites/player.png`.

`CMakeLists.txt` uses `GLOB_RECURSE` with `CONFIGURE_DEPENDS` to discover
source files, but adding new files may still require a reconfigure.

Strict compiler warnings are enabled (`/W4` on MSVC, `-Wall -Wextra` on
GCC/Clang). Some warnings are promoted to errors. In C, missing prototypes can
break x64 argument passing, especially for float math helpers such as `Clamp`,
so include the owning header instead of relying on implicit declarations.

## Current Architecture

Window and logical resolution are 1024x960 at 1:1 scale. Tiles are 64x64
pixels. The screen layout is:

- HUD: 1024x224 pixels.
- Divider: 1024x32 pixels.
- Play area: 1024x704 pixels, or 16x11 tiles.
- Play area origin: `PLAY_AREA_Y` = 256.
- Target frame rate: 60 FPS.

`main.c` owns raylib initialization, the main loop, and shutdown. A single
`Game` struct, defined in `game.h`, holds the top-level gameplay state and is
passed by pointer to systems. The main loop calls `game_init`, then
`game_update` / `game_draw` each frame, then cleanup on exit.

The current top-level game states are:

- `STATE_PLAY`: normal gameplay.
- `STATE_TRANSITION`: scroll or fade transition between screens.
- `STATE_DEATH`: death pause/fade and respawn.
- `STATE_PAUSE`: inventory/pause screen, with game logic frozen.
- `STATE_ITEM_GET`: dungeon item pickup ceremony, with game logic frozen.

`game_config.h` is the canonical home for compile-time constants, combat
tuning, projectile tuning, and the shared `Direction` enum plus
`opposite_dir()`.

## Update Order

During `STATE_PLAY`, the update order is:

1. `player_update`
2. `enemies_update`
3. `projectiles_update`
4. `combat_check_bombs`
5. `vfx_update`
6. `combat_check`
7. `pickups_update`
8. `combat_check_pickups`
9. `dungeon_check_locked_door`
10. `dungeon_check_shutter_room`
11. `dungeon_check_push_block`
12. `dungeon_check_items`
13. death and low-health checks
14. `nav_check_warp`
15. `nav_check_edge_transition`

This order matters. Keep new interactions deterministic and place them near the
system they affect.

## Implemented Modules

- `src/game_config.h` owns compile-time constants, tuning values, `Direction`,
  and `opposite_dir()`.
- `src/game.h` / `src/game.c` own the shared `Game` struct, top-level state
  machine, frame dispatch, death/item-get/pause transitions, and drawing.
- `src/main.c` owns raylib initialization, the main loop, and shutdown.
- `src/input.h` / `src/input.c` abstract keyboard and gamepad input.
- `src/player.h` / `src/player.c` own player position, facing, movement,
  animation, health, inventory data, sword attacks, item use, knockback, and
  invulnerability.
- `src/tilemap.h` / `src/tilemap.c` own tile definitions, screen/room parsing,
  metadata parsing, warp lookup, tile rendering, and tile collision.
- `src/navigation.h` / `src/navigation.c` own screen loading, overworld/cave/
  dungeon transition setup, warps, and transition completion.
- `src/camera.h` / `src/camera.c` own scroll and fade transitions, including
  screen offsets and fade alpha.
- `src/combat.h` / `src/combat.c` own hitbox overlap checks between player,
  sword, enemies, projectiles, bombs, pickups, and shield blocking.
- `src/enemy/enemy.h` / `src/enemy/enemy.c` own enemy shared data, definition
  table, spawn lookup, update/draw dispatch, and default drop behavior.
- `src/enemy/*.c` contains one enemy implementation per enemy type: slime, bat,
  charging snake, rock spitter, spear thrower, and dragon.
- `src/projectile.h` / `src/projectile.c` own projectile shared data,
  definition table, update/draw dispatch, arrows, boomerang, bombs, rocks,
  spears, and dragon beams.
- `src/pickup.h` / `src/pickup.c` own screen-local pickups, collection, flashing,
  and despawn timing.
- `src/vfx.h` / `src/vfx.c` own fire-and-forget visual effects such as bomb
  explosions.
- `src/items.h` owns `ItemType` plus item name/display-name tables.
- `src/inventory.h` / `src/inventory.c` own pause screen state, item selection,
  and dungeon map rendering.
- `src/dungeon.h` / `src/dungeon.c` own dungeon state, room flags, permanent
  unlocks, item collection, boss state, and dungeon room helpers.
- `src/dungeon_interact.h` / `src/dungeon_interact.c` own locked doors,
  shutter rooms, push blocks, dark-room lighting, and dungeon item pickups.
- `src/anim.h` / `src/anim.c` own `AnimDef` and `Anim`.
- `src/textures.h` / `src/textures.c` own texture loading, unloading, texture
  IDs, and frame rectangle helpers.
- `src/hud.h` / `src/hud.c` own HUD rendering: hearts, rupees, keys, bombs,
  equipped item area, overworld minimap, and dungeon minimap.
- `src/sounds.h` / `src/sounds.c` load optional sound effects, skip missing
  files, and play one-shot sounds.
- `src/music.h` / `src/music.c` own streamed music state for overworld,
  dungeon, and boss tracks.
- `src/debug.h` / `src/debug.c` provide the F3 debug overlay.

Planned Phase 6 systems such as shops, NPC dialogue, save/load, title/file
select, item gates, and continue-after-death should be added as focused
header/source pairs rather than growing unrelated modules.

## Enemy and Projectile Dispatch

Enemies use vtable-style definition tables. `EnemyDef` stores a screen-file
name, stats, and function pointers for update/draw/spawn/death behavior. Adding
a new enemy should mainly mean adding its implementation file under
`src/enemy/` and one row in `enemy_defs[]`.

Projectiles use the same pattern through `ProjectileDef`. Generic straight-line
projectiles share common behavior in `projectile.c`; unique projectiles such as
boomerang and bomb own their own update logic.

Use function-pointer definition tables for large or growing type sets such as
enemies and projectiles. Use `switch` statements for small, stable state
machines such as game state, player state, and item-use dispatch. Avoid central
orchestrator switches for every enemy/projectile type.

## World Data

The overworld is a 16x8 grid. Screen files are named by grid coordinate:

```text
assets/screens/XX_YY.txt
```

For example, screen column 7, row 4 is `assets/screens/07_04.txt`.

Cave screens live in:

```text
assets/caves/
```

Caves are loaded by name, for example `cave_01`. Edge transitions are disabled
while `in_cave` is true. Cave entry stores the origin overworld screen and tile
position in the `Game` struct. A cave warp with destination `return` sends the
player back to that saved overworld location.

Dungeon rooms live in:

```text
assets/dungeons/<n>/
```

Dungeon room files use the same `XX_YY.txt` naming as overworld screens.
Dungeon room Y increases northward; this is intentionally opposite the
overworld convention where Y increases southward. Transition, HUD, and pause
map logic account for this.

Screen files are plain text with optional metadata lines followed by 11 rows of
exactly 16 tile characters. Supported metadata includes comments and lines such
as `warp:`, `enemy:`, `door:`, `item:`, `shutter:`, `dark:`, and `boss:`.
Current tile characters include:

- `W`: wall.
- `.`: floor.
- `~`: water.
- `D`: door.
- `P`: pushblock.
- `S`: stairs.
- `B`: bombable wall.

Warp destination formats:

- `XX_YY`: overworld screen.
- `cave_XX`: cave file, e.g. `assets/caves/cave_XX.txt`.
- `dungeon_<n>`: enter dungeon.
- `droom_XX_YY`: dungeon room.
- `return`: exit cave/dungeon to the saved overworld position.

Door metadata and door tiles must agree. Every `door:` line should have
matching `D` tiles at the correct edge, and every functional dungeon door
should have corresponding metadata.

## Core Constraints

- Window and logical resolution: 1024x960 at 1:1 scale.
- Tile size: 64x64 pixels.
- HUD: 1024x224 pixels.
- Divider: 32 pixels.
- Play area: 1024x704 pixels, or 16x11 tiles.
- Play area begins at y=256.
- Main loop target: 60 FPS.
- Movement is continuous pixel movement, not tile-stepped.
- Diagonal input resolves to one cardinal direction.
- Runtime asset paths are relative to the executable output directory.
- Screen-local arrays are fixed-size; prefer stack/static allocation for fixed
  capacity gameplay data.

## Coding Principles

- **C game programming best practices**: prefer stack/static allocation for
  fixed-size data, and use heap allocation only when size varies at runtime.
  Keep hot data contiguous and avoid unnecessary indirection.
- **KISS**: choose the simplest clear implementation. A plain `if` is good
  when behavior is local and stable.
- **YAGNI**: do not add abstraction layers for hypothetical future use.
- **DRY**: remove real duplication, not merely shape-similar code. The wrong
  abstraction is worse than a little repetition.
- **Locality of change**: adding a new entity, tile, or feature should require
  edits in as few files as practical.

Include discipline:

- Include the header that declares every function you call.
- Files in `src/enemy/` use relative paths for project headers such as
  `../tilemap.h`, but standard include paths for vendor headers such as
  `"raylib.h"` and `"raymath.h"`.
- Any file that calls raylib math functions such as `Clamp` must include
  `"raymath.h"`.

When in doubt, lean KISS for isolated code and locality of change for systems
many features will plug into.

## Asset Pipeline

Sprites are PNGs exported from SVGs, usually on a 64px grid. Store both SVG and
PNG files in:

```text
assets/sprites/
```

The code should load one `Texture2D` per sheet or sprite and index frames by
row/column source rectangles where appropriate.

Current sprite conventions:

- Player: `player.png`.
- Enemies: `slime.png`, `bat.png`, `snake.png`, plus placeholder sprites for
  ranged enemies as needed.
- Projectiles: `arrow.png`, `boomerang.png`, `bomb.png`, `rock.png`,
  `spear.png`.
- Pickups: `pickup_rupee.png`, `pickup_heart.png`, `pickup_bomb.png`,
  `pickup_arrow.png`.
- Dungeon items: `item_key.png`, `item_map.png`, `item_compass.png`,
  `item_heart_container.png`, `item_fragment.png`.
- Tiles: `tiles.png` is a 256x192 sheet (4 columns x 3 rows): floor, wall,
  water frames, door, pushblock, stairs, bombable wall, closed door.

Single-frame export example:

```bash
inkscape item_heart.svg -o item_heart.png -w 64 -h 64
```

Spritesheet export example:

```bash
inkscape player.svg -o player.png -w 256 -h 256
```

On this machine, Inkscape is commonly available at:

```bash
"C:/Program Files/Inkscape/bin/inkscape.exe"
```

Generate game sounds with rfxgen:

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

Store WAV files in `assets/`. Sound loading is resilient: missing files are
skipped, and present files play normally.

Music lives in:

```text
assets/music/
```

Music files are OGG streams loaded through raylib `LoadMusicStream` and updated
every frame by `music.c`. The composition pipeline lives under `tools/music/`:
Python scripts generate MIDI, FluidSynth renders WAV from a soundfont, and
ffmpeg converts WAV to OGG. See `tools/music/compose_overworld.py` for the
pattern.

Screen and room data should use the plain text formats described in
`IMPLEMENTATION.md`, stored under:

- `assets/screens/`
- `assets/dungeons/<n>/`
- `assets/caves/`

## Implementation Order

Follow the phases in `IMPLEMENTATION.md`. Each phase should produce a playable
build that can be tested before moving on.

Current status:

- Phase 1, Player and Tiles: complete.
- Phase 2, World Navigation: complete.
- Phase 3, Combat: complete.
- Phase 4, Projectiles and Items: complete.
- Phase 5, Dungeons: complete.
- Phase 6, World Systems: next.

For Phase 6, focus on the systems listed in `IMPLEMENTATION.md`:

- Shop system and rupee transactions.
- NPC dialogue, hint caves, and gift caves.
- Cave room behavior updates.
- Item gates such as raft, ladder, bracelet, fire, and bombs.
- Sword upgrades and health-gated master sword.
- Save/load with three slots and atomic writes.
- Title screen with file select.
- Continue-after-death flow.
- Shop and NPC sounds.

Keep changes scoped to the requested phase or feature, build after code
changes when practical, and avoid rewriting unrelated files.
