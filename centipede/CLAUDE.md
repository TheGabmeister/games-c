# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Pac-Man clone written in C using raylib for rendering/input/audio.

## Build Commands

```bash
# Configure (from project root)
cmake -B build

# Build
cmake --build build

# Run (executable outputs to build/centipede/)
./build/centipede/centipede
```

The build copies `src/resources/` into the output directory automatically.

## Architecture

- **Language:** C (no C++), linked against raylib
- **Build system:** CMake, with raylib vendored under `vendor/raylib/`
- **Source layout:** All `.c` and `.h` files live under `src/` (recursively globbed). Game assets go in `src/resources/`.

## Coding principles

- **C game programming best practices** — prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime. Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job. 
- **YAGNI** — don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.

When in doubt, lean KISS over DRY.

## Sprite Generation

Workflow: write SVG markup, then convert to PNG with Inkscape.

```bash
inkscape input.svg -o output.png -w 32 -h 32
```

Store both SVGs and PNGs in `src/resources/`.

## Sound Generation

Generate game sounds with rfxgen (by the raylib author).

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

Store WAV files in `src/resources/`.