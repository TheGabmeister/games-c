# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Modernized Super Mario Bros clone written in C using raylib for rendering/input/audio. See `README.md` for project overview, controls, and game states.

## Build Commands

```bash
# Configure (from project root)
cmake -B build

# Build
cmake --build build

# Run (MSVC puts exe under Debug/)
./build/super_mario_bros/Debug/super_mario_bros
```

No tests or linter — the build is the only verification step. The build copies `src/resources/` into the output directory automatically.

## Architecture

- **Language:** C (no C++), linked against raylib
- **Build system:** CMake, with raylib vendored under `vendor/raylib/`
- **Source layout:** All `.c` and `.h` files live under `src/` (recursively globbed by CMake). Game assets go in `src/resources/`.
- **Window:** 800x600, 16px tile grid, 60 FPS
- **Input:** keyboard (arrow keys, WASD, Space, Shift) and gamepad (left stick, D-pad, face buttons) simultaneously
- **Visual style:** modernized — clean sprites, particle effects, smooth animations. Not pixel-art retro.

### Key design patterns

- One `Game` struct holds all state, passed by pointer. No heap allocation.
- Each module owns its struct and logic; `game.c` orchestrates them. Modules take only the fields they need.
- `game_update()` dispatches to one handler function per game state (`update_title`, `update_playing`, etc.). `game_draw()` has a similar per-state switch.
- Side-scrolling camera follows Mario horizontally.
- Tile-based levels with 16x16 pixel tiles for collision and rendering.
- Named constants for all tunable values live in `common.h` (`#define`). New magic numbers should be added there, not hardcoded inline.

### Important caveats

- `DIR_NONE` is -1. Always guard with `if (dir != DIR_NONE)` before using direction as an array index.

## Asset Pipeline

Sprites are authored as SVGs and converted to PNGs using Inkscape:

```bash
# Convert SVG to PNG (example: 16x16 tile)
"C:/Program Files/Inkscape/bin/inkscape.exe" input.svg -o output.png -w 16 -h 16
```

Store SVGs in `src/resources/svg/` and PNGs in `src/resources/`. Only PNGs are loaded at runtime.

## Sound Generation

Generate game sounds with rfxgen (by the raylib author).

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

Store WAV files in `src/resources/`.

## Coding principles

- **C game programming best practices** — prefer stack allocation over heap, use fixed-size arrays where possible, keep hot data contiguous, avoid unnecessary indirection.
- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job.
- **YAGNI** — don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.

When in doubt, lean KISS over DRY.
