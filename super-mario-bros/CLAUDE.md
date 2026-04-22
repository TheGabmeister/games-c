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
- **Window:** 1280x960, 64px tile grid (20x15 visible), 60 FPS. No framebuffer scaling — renders directly at window resolution.
- **Input:** keyboard (arrow keys, WASD, Space, Shift) and gamepad (left stick, D-pad, face buttons) simultaneously
- **Visual style:** modernized — clean sprites, particle effects, smooth animations. Not pixel-art retro.

### Key design patterns

- One `Game` struct holds all state, passed by pointer.
- **Tagged entity array with vtables:** all dynamic objects (Mario, enemies, items, projectiles, debris) live in a flat `Entity entities[MAX_ENTITIES]` array. Each entity has a `type` tag and a pointer to a `static const EntityVtab` (function pointers: `update`, `draw`, `touch`, `stomped`, `hit_by_fire`, `hit_by_shell`, `hit_by_star`, `bumped`, `kill`). The engine calls these callbacks — the entity type defines its own behavior.
- **Collision flags on entities:** `stompable`, `damages_mario`, `fire_immune`, `shell_killable`, `star_killable`, `destructible`. Set at spawn time. The engine checks flags to decide *what kind* of interaction, then calls the vtable callback for the *type-specific response*.
- **Tile handler table:** tile-entity interactions (hitting blocks from below) are dispatched via a function pointer table indexed by tile type. Each tile type defines its own handler (brick breaks, question block spawns item, etc.).
- `game.c` is the orchestrator: runs update/collision loops and calls callbacks. It does not contain entity-specific or tile-specific logic.
- Each entity type has its own file(s) defining its vtable, spawn function, and behavior. Enemy files live under `src/enemies/`.
- `game_update()` dispatches to one handler function per game state (`update_title`, `update_playing`, etc.). `game_draw()` has a similar per-state switch.
- Side-scrolling camera follows Mario horizontally, never scrolls backward.
- Tile-based levels with 16x16 pixel tiles for collision and rendering.
- Named constants for all tunable values live in `common.h` (`#define`). New magic numbers should be added there, not hardcoded inline.
- See `SPEC.md` for full architecture details, vtable definitions, update/draw flow, and collision system.

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

- **C game programming best practices** — prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime (e.g. level tile grids). Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job.
- **YAGNI** — don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.

When in doubt, lean KISS over DRY.
