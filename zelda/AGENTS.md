# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Project

This is a 2D top-down action-adventure game inspired by The Legend of Zelda
(1986), built in C with raylib and CMake.

Phases 1 and 2 are complete:

- Phase 1: player movement, animation, HUD, tile rendering, and tile collision.
- Phase 2: overworld screen loading, scroll transitions, cave/fade transitions,
  and music playback.

Phase 3, Combat, is next. Do not skip ahead to later gameplay systems unless
the current request explicitly asks for it.

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
Assets under `src/assets/` are copied beside the executable by
`CMakeLists.txt`, so runtime asset paths should be relative to that output
directory, for example `assets/sprites/player.png`.

`CMakeLists.txt` uses `GLOB_RECURSE` with `CONFIGURE_DEPENDS` to discover
source files, but adding new files may still require a reconfigure.

## Current Architecture

Window and logical resolution are 1024x960 at 1:1 scale. Tiles are 64x64
pixels. The screen layout is:

- HUD: 1024x224 pixels.
- Divider: 1024x32 pixels.
- Play area: 1024x704 pixels, or 16x11 tiles.
- Play area origin: `PLAY_AREA_Y` = 256.
- Target frame rate: 60 FPS.

`main.c` owns raylib initialization, the main loop, and shutdown. A single
`Game` struct, defined in `game.h`, holds the top-level game state and is
passed by pointer to systems. The main loop calls `game_init`, then
`game_update` / `game_draw` each frame, then cleanup on exit.

The current top-level game states are:

- `STATE_PLAY`: normal gameplay.
- `STATE_TRANSITION`: scroll or fade transition between screens.

The broader planned state machine, including title, pause, death, and item-get
states, is documented in `IMPLEMENTATION.md`.

## Implemented Modules

- `src/game_config.h` is the canonical home for compile-time constants such as
  window size, tile size, HUD dimensions, target FPS, and gamepad deadzone.
- `src/game.h` / `src/game.c` own the shared `Game` struct, top-level state
  machine, screen loading by grid coordinates and cave names, edge detection,
  scroll/fade transition orchestration, cave enter/exit return data, and music
  update.
- `src/main.c` owns raylib initialization, the main loop, and shutdown.
- `src/input.h` / `src/input.c` abstract keyboard and gamepad input.
- `src/player.h` / `src/player.c` own player position, facing, state, health,
  inventory data, animation, 4-directional movement, grid assist, and
  separate-axis tile collision.
- `src/tilemap.h` / `src/tilemap.c` own tile definitions, screen data, screen
  file parsing, metadata skipping, warp lookup, tile rendering with offsets,
  and tile collision.
- `src/anim.h` / `src/anim.c` own `AnimDef` and `Anim`, used by the player and
  future animated entities/tiles.
- `src/camera.h` / `src/camera.c` own scroll transitions and fade-to-black
  transitions, including screen offsets and fade alpha.
- `src/textures.h` / `src/textures.c` own texture loading, unloading, texture
  IDs, and frame rectangle helpers.
- `src/hud.h` / `src/hud.c` own HUD rendering: hearts, rupees, keys, bombs,
  equipped item area, and overworld minimap position.
- `src/sounds.h` / `src/sounds.c` load optional sounds and music, skip missing
  files, play sound effects, and update streamed music.
- `src/assets/` contains runtime assets copied into the executable output
  directory.

Planned systems such as enemies, projectiles, combat collision, event queues,
inventory/pause screen, save/load, shops, NPC dialogue, dungeons, and title
screen are described in `IMPLEMENTATION.md`. Add them as focused header/source
pairs rather than growing unrelated modules.

## World Data

The overworld is a 16x8 grid. Screen files are named by grid coordinate:

```text
src/assets/screens/XX_YY.txt
```

For example, screen column 7, row 4 is `src/assets/screens/07_04.txt`.
Current test screens exist around position `(7, 4)`.

Cave screens live in:

```text
src/assets/caves/
```

Caves are loaded by name, for example `cave_01`. Edge transitions are disabled
while `in_cave` is true.

Cave entry stores the origin overworld screen and tile position in the `Game`
struct. A cave warp with destination `return` sends the player back to that
saved overworld location. This lets multiple overworld screens share one cave
file.

Screen files are plain text with optional metadata lines followed by 11 rows of
16 tile characters. Supported metadata includes comments and lines such as
`warp:`, `enemy:`, and `item:`. Current tile characters include:

- `W`: wall.
- `.`: floor.
- `~`: water.
- `D`: door.
- `P`: pushblock.
- `S`: stairs.

Warp destination formats:

- `XX_YY`: overworld screen.
- `cave_XX`: cave file, e.g. `assets/caves/cave_XX.txt`.
- `return`: exit cave to the saved overworld position.

See `IMPLEMENTATION.md` for the full file format.

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

On this machine, Inkscape is commonly available at:

```bash
"C:/Program Files/Inkscape/bin/inkscape.exe"
```

The code should load one `Texture2D` per sheet and index frames by row/column
source rectangles. Use one sheet per category: player, each enemy type, tiles,
items, HUD, and projectiles.

Store both SVGs and PNGs in:

```text
src/assets/sprites/
```

Generate game sounds with rfxgen:

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

Store WAV files in `src/assets/`. Sound loading is resilient: missing files are
skipped, and present files play normally.

Music lives in:

```text
src/assets/music/
```

Music files are OGG streams loaded through raylib `LoadMusicStream` and updated
every frame. The composition pipeline lives under `tools/music/`: Python
scripts generate MIDI, FluidSynth renders WAV from a soundfont, and ffmpeg
converts WAV to OGG. See `tools/music/compose_overworld.py` for the pattern.
`tools/music/sampler.py` generates an audition file cycling through soundfont
instruments.

Screen and room data should use the plain text formats described in
`IMPLEMENTATION.md`, stored under:

- `src/assets/screens/`
- `src/assets/dungeons/<n>/`
- `src/assets/caves/`

## Implementation Order

Follow the phases in `IMPLEMENTATION.md`. Each phase should produce a playable
build that can be tested before moving on.

Current status:

- Phase 1, Player and Tiles: complete.
- Phase 2, World Navigation: complete.
- Phase 3, Combat: next.

For Phase 3, focus on the systems listed in `IMPLEMENTATION.md`: sword attack,
enemy definition table, starter enemies, combat collision, damage/knockback,
invulnerability, death handling, combat events, and combat sounds.

Keep changes scoped to the requested phase or feature, build after code
changes when practical, and avoid rewriting unrelated files.

