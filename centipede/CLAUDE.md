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

# Run (executable outputs to build/pac_man/)
./build/pac_man/pac_man
```

The build copies `src/resources/` into the output directory automatically.

## Architecture

- **Language:** C (no C++), linked against raylib
- **Build system:** CMake, with raylib vendored under `vendor/raylib/`
- **Source layout:** All `.c` and `.h` files live under `src/` (recursively globbed). Game assets go in `src/resources/`.

## Coding principles

- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job. If a class is under 50 lines and clear, don't split it.
- **YAGNI** — don't build for hypothetical needs. No interfaces with one implementation, no config knobs with one value, no abstraction layers "for later." Write what this phase needs; refactor when a second use case actually shows up.
- **DRY** — remove real duplication, not shape-similar code. Three copies of the same logic → extract. Two functions that happen to both take a `Vector3` → leave alone. Wrong abstraction costs more than repetition.

When in doubt, lean KISS over DRY. A bit of repetition is cheaper to read and change than the wrong shared helper.

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