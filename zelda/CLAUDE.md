# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A 2D top-down action-adventure game inspired by The Legend of Zelda (1986), built in C with raylib. The game design is specified in SPEC.md. The project is in early development — most game systems are stubs.

## Build

```bash
# Build (uses existing build directory)
cmake --build build

# Full reconfigure + build
cmake -S . -B build
cmake --build build

# Release build
cmake --build build --config Release
```

The executable lands in `build/zelda/Debug/zelda.exe` (or `Release/`). Assets from `src/assets/` are copied to the executable's directory at build time, so runtime paths are relative: `assets/coin.wav`.

Generator: Visual Studio 18 2026, x64.

## Architecture

Single `Game` struct holds all state (defined in `game.h`). The main loop in `main.c` calls `game_init` → `game_update`/`game_draw` per frame → cleanup on exit.

Modules are header/source pairs:
- **game.c/.h** — state, init, update, draw (currently just a movable circle)
- **input.c/.h** — keyboard + gamepad abstraction (WASD, arrows, gamepad with deadzone)
- **sounds.c/.h** — sound loading/playback (stub — functions exist but are empty)
- **textures.c/.h** — texture management (stub — completely empty)
- **game_config.h** — shared constants (window size, FPS, lives, deadzone)

Note: `game.h` and `game_config.h` both define `WINDOW_WIDTH`/`WINDOW_HEIGHT` with different values (1280x720 vs 1200x900). `main.c` includes `game.h`, so the 1280x720 values are what the window actually uses.

## Coding principles

- **C game programming best practices** — prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime. Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job. But a plain `if` that must be copy-pasted into every new feature is not simple — it's a maintenance trap.
- **YAGNI** — don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.
- **Locality of change** — adding a new entity, tile, or feature should require changes in as few files as possible. Prefer data-driven dispatch (flags, vtables) over centralized type switches when the set of types is expected to grow. If a new enemy requires editing the orchestrator, the abstraction is missing.

When in doubt: for code one person owns and rarely changes, lean KISS. For interfaces many contributors touch, lean locality of change.


## Asset Pipeline

- **Sprites**: write SVG, convert to PNG with Inkscape (`inkscape input.svg -o output.png -w 20 -h 20`). Cell size is 20px. Store both SVG and PNG in `src/assets/`.
- **Sounds**: generate with rfxgen (`"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav`). Presets: coin, laser, explosion, powerup, hit, jump, blip. Store WAV in `src/assets/`.
- Sound loading is resilient — missing files are skipped.
