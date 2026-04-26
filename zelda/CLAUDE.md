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

## Conventions

- **C style**: snake_case functions, UPPER_CASE constants, PascalCase for raylib types and enum values (e.g., `SoundID`)
- **Memory**: prefer stack/static allocation; heap only when size varies at runtime
- **Design**: KISS over DRY. No abstraction layers "for later." Simplest thing that works.
- CMake globs `src/*.c` and `src/*.h` — new source files are picked up automatically

## Asset Pipeline

- **Sprites**: write SVG, convert to PNG with Inkscape (`inkscape input.svg -o output.png -w 20 -h 20`). Cell size is 20px. Store both SVG and PNG in `src/assets/`.
- **Sounds**: generate with rfxgen (`"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav`). Presets: coin, laser, explosion, powerup, hit, jump, blip. Store WAV in `src/assets/`.
- Sound loading is resilient — missing files are skipped.
