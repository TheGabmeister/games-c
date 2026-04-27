# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Project

This is a 2D top-down action-adventure game inspired by The Legend of Zelda
(1986), built in C with raylib and CMake. The project is in early development:
some foundational modules exist, while most gameplay systems are still planned
or stubbed.

Key documents:

- `SPEC.md` is the source of truth for game behavior, mechanics, items,
  enemies, dungeons, tuning values, and progression.
- `IMPLEMENTATION.md` describes the planned code architecture, data
  structures, module boundaries, file formats, and implementation phases.
- `AGENTS.md` gives practical repository guidance for coding agents.

Before implementing a new gameplay system, read the relevant section of
`SPEC.md` and the architecture notes in `IMPLEMENTATION.md`.

## Build

Use the existing build directory when present:

```bash
cmake --build build
```

If the build directory is missing or stale, configure first:

```bash
cmake -S . -B build
cmake --build build
```

For a release build:

```bash
cmake --build build --config Release
```

The default Visual Studio generator emits the executable under
`build/zelda/Debug/zelda.exe` or `build/zelda/Release/zelda.exe`. Assets under
`src/assets/` are copied beside the executable by `CMakeLists.txt`, so runtime
asset paths should be relative to that output directory, for example
`assets/coin.wav`.

## Current Structure

- `src/main.c` owns raylib initialization, the main loop, and shutdown.
- `src/game_config.h` is the canonical home for compile-time constants such as
  window size, tile size, target FPS, and gamepad deadzone.
- `src/game.h` / `src/game.c` own the shared `Game` struct and top-level
  init/update/draw flow.
- `src/input.h` / `src/input.c` abstract keyboard and gamepad input.
- `src/sounds.h` / `src/sounds.c` load optional sounds and skip missing files.
- `src/textures.h` / `src/textures.c` own texture loading and cleanup.
- `src/assets/` contains runtime assets copied into the executable output
  directory.

Planned systems such as tilemaps, enemies, projectiles, collision, events,
HUD, inventory, save/load, and screen transitions are described in
`IMPLEMENTATION.md`. Add them as focused header/source pairs rather than
growing unrelated modules.

## Core Constraints

- Window and logical resolution: 1024x960 at 1:1 scale.
- Tile size: 64x64 pixels.
- HUD: 1024x224 pixels.
- Divider: 32 pixels.
- Play area: 1024x704 pixels, or 16x11 tiles.
- Main loop target: 60 FPS.
- Movement is continuous pixel movement, not tile-stepped.
- Runtime asset paths are relative to the executable output directory.

## Coding Principles

- **C game programming best practices**: prefer stack/static allocation for
  fixed-size data, use heap only when size varies at runtime. Keep hot data
  contiguous and avoid unnecessary indirection.
- **KISS**: choose the simplest clear implementation. A plain `if` is good
  when behavior is local and stable.
- **YAGNI**: do not add abstraction layers for hypothetical future use.
- **DRY**: remove real duplication, not merely shape-similar code. The wrong
  abstraction is worse than a little repetition.
- **Locality of change**: adding a new entity, tile, or feature should require
  edits in as few files as practical.

Dispatch guidance:

- Use function-pointer definition tables for large or growing type sets such
  as enemies and projectiles.
- Use `switch` statements for small, stable state machines such as game state,
  player state, and item-use dispatch.
- Avoid central orchestrator switches for every enemy/projectile type. Adding
  a new enemy should mainly mean adding that enemy's update/draw functions and
  one row in a definition table.

When in doubt, lean KISS for isolated code and locality of change for systems
many features will plug into.

## Asset Pipeline

Sprites are spritesheets: one SVG per entity or tile set, with all frames laid
out on a 64px grid. Export the full sheet as one PNG.

```bash
# single sprite (1x1 cell)
inkscape item_heart.svg -o item_heart.png -w 64 -h 64

# spritesheet (for example, player: 4 columns x 4 rows)
inkscape player.svg -o player.png -w 256 -h 256
```

The code should load one `Texture2D` per sheet and index frames by row/column
source rectangles. Use one sheet per category: player, each enemy type, tiles,
items, HUD, and projectiles.

Store both SVGs and PNGs in `src/assets/sprites/`.

Generate game sounds with rfxgen:

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

Store WAV files in `src/assets/`. Sound loading is resilient: missing files are
skipped, and present files play normally.

Screen and room data should use the plain text formats described in
`IMPLEMENTATION.md`, stored under `src/assets/screens/`,
`src/assets/dungeons/<n>/`, and `src/assets/caves/`.

## Implementation Order

Follow the phases in `IMPLEMENTATION.md`. Each phase should produce a playable
build that can be tested before moving on. Do not skip ahead to later systems
unless the current request explicitly asks for it.

Keep changes scoped to the requested phase or feature, build after code
changes when practical, and avoid rewriting unrelated files.
