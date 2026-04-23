# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Centipede (1981 arcade) clone written in C using raylib for rendering/input/audio.

## Build Commands

```bash
# Configure (from project root)
cmake -B build

# Build
cmake --build build

# Run (executable outputs to build/centipede/Debug/ on MSVC)
./build/centipede/Debug/centipede.exe
```

The build copies `src/resources/` into the output directory automatically. CMake auto-discovers all `.c`/`.h` files in `src/` via `GLOB_RECURSE` -- no CMakeLists.txt changes needed when adding new source files.

## Architecture

- **Language:** C (no C++), linked against raylib
- **Build system:** CMake, with raylib vendored under `vendor/raylib/`
- **Source layout:** All `.c` and `.h` files live under `src/` (flat, no subdirectories). Game assets go in `src/resources/`.

### Central header: `game.h`

Every module includes `game.h`. It contains all `#define` constants, enums (`GameState`, `Direction`, `SoundID`), struct definitions (`Game`, `Player`, `Dart`, `Segment`, `Spider`, `Flea`, `Scorpion`, `MushroomGrid`), and inline coordinate conversion helpers (`col_to_px`, `row_to_px`, `px_to_col`, `px_to_row`). The `Game` struct is the single top-level state container -- passed by pointer to all subsystems.

### Module dependency graph

```
main.c → game.h, sounds.h
game.c → game.h, player.h, mushroom.h, centipede.h, enemies.h, sounds.h
player.c → player.h → game.h
centipede.c → centipede.h → game.h
enemies.c → enemies.h → game.h, mushroom.h
mushroom.c → mushroom.h → game.h
sounds.c → sounds.h → game.h
```

`game.c` is the orchestrator: it owns the state machine, calls all subsystem update/draw functions, and implements all collision detection as static helpers.

### Grid system

30 columns x 31 rows, 20px cells. Row 0 is reserved for the HUD (score/lives/level rendered in the 40px offset above the grid). Player is restricted to the bottom 5 rows (26-30). Window size: 600x660.

### Mushroom encoding

`MushroomGrid.cells[row][col]` is a single `unsigned char`: 0 = empty, 1-4 = normal mushroom with that HP, 5-8 = poisoned mushroom (subtract 4 for HP). Use the `MUSH_*` macros to read/write.

### Centipede segments

Stored in a flat pool (`Segment segments[64]`). Chains are linked via `next`/`prev` indices. Each segment moves independently on a step timer (`CELL_SIZE / speed` interval), bouncing off walls and mushrooms. Splitting on hit: deactivate the hit segment, place a mushroom, promote the next segment to head.

### State machine

`STATE_TITLE` → `STATE_READY` (2s) → `STATE_PLAYING` → `STATE_DYING` (1.5s) → `STATE_RESTORING` → `STATE_READY` or `STATE_GAME_OVER`. Level clear: `STATE_PLAYING` → `STATE_LEVEL_COMPLETE` (2s) → `STATE_READY`. All transitions live in `game_update()`.

## Coding principles

- **C game programming best practices** -- prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime. Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** -- simplest thing that works. No clever patterns where a plain `if` does the job.
- **YAGNI** -- don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** -- remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.

When in doubt, lean KISS over DRY.

## Sprite Generation

Workflow: write SVG markup, then convert to PNG with Inkscape.

```bash
inkscape input.svg -o output.png -w 20 -h 20
```

Store both SVGs and PNGs in `src/resources/`. Cell size is 20px.

## Sound Generation

Generate game sounds with rfxgen (by the raylib author).

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

Store WAV files in `src/resources/`. Sound loading is resilient -- missing files are skipped, present files play normally.
