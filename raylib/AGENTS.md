# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Purpose

This repository is a small raylib game template built with CMake. 

Resources under `src/assets/` are copied into the executable output
directory by `CMakeLists.txt`.

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

Store both SVGs and PNGs in `src/assets/`. Cell size is 20px.

## Sound Generation

Generate game sounds with rfxgen (by the raylib author).

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

Store WAV files in `src/assets/`. Sound loading is resilient -- missing files are skipped, present files play normally.
