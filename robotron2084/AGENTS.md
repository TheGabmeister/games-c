# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Purpose

This repository is a raylib C recreation of a Robotron 2084-style twin-stick
arena shooter, built with CMake. Follow `SPEC.md` and implement the game by
milestones so each slice can be played and tuned before adding more systems.

Assets under `src/assets/` are copied into the executable output directory by
`CMakeLists.txt`. Runtime asset paths should be relative to that output
directory, for example `assets/player.png` or `assets/player_shoot.wav`.

This is an original recreation. Do not use the original ROM, ripped arcade
sprites, ripped cabinet art, or sampled arcade audio.

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

The executable is emitted under `build/template/Debug/` for the default Visual
Studio generator configuration.

## Current structure

- `src/main.c` owns raylib initialization, the main loop, and shutdown.
- `src/game.h` contains shared constants, enums, fixed-size entity structs, and
  the central `Game` state.
- `src/game.c` owns high-level game state transitions, top-level update/draw
  orchestration, HUD text, and overlays.
- `src/world.c` / `src/world.h` own playfield simulation: wave spawning,
  entity updates, collisions, scoring helpers, primitive/sprite drawing,
  particles, and gameplay event hooks.
- `src/textures.c` / `src/textures.h` load optional PNG sprites and skip missing
  files. Gameplay should keep primitive fallbacks when textures are absent.
- `src/sounds.c` / `src/sounds.h` load optional WAV sounds and skip missing
  files. Gameplay should still work silently when sounds are absent.
- `src/assets/` stores source SVGs, generated PNGs, and generated WAVs.

Keep `Game` as the owner of fixed arrays. Prefer simple, contiguous, fixed-size
data over dynamic allocation or a generic entity system.

## Coding principles

- **C game programming best practices** -- prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime. Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** -- simplest thing that works. No clever patterns where a plain `if` does the job.
- **YAGNI** -- don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** -- remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.

When in doubt, lean KISS over DRY.

## Milestone workflow

- Work from `SPEC.md`.
- Keep each milestone playable before adding the next one.
- Do not add later enemy families while still tuning the current milestone.
- Keep presentation effects subordinate to readability.
- Build after code changes with `cmake --build build`.

## Sprite Generation

Workflow: write SVG markup, then convert to PNG with Inkscape.

```bash
inkscape input.svg -o output.png -w 20 -h 20
```

Store both SVGs and PNGs in `src/assets/`. Cell size is 20px.

On this Windows machine, Inkscape may not be on `PATH`. Use:

```powershell
& "C:/Program Files/Inkscape/bin/inkscape.com" --export-type=png --export-filename="src/assets/player.png" --export-width=20 --export-height=20 "src/assets/player.svg"
```

## Sound Generation

Generate game sounds with rfxgen (by the raylib author).

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

Store WAV files in `src/assets/`. Sound loading is resilient -- missing files are
skipped, present files play normally.

rfxgen may not write directly to nested output paths. If needed, generate in the
repo root, then copy into `src/assets/`:

```powershell
& "D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g blip -o player_shoot.wav
Copy-Item -LiteralPath player_shoot.wav -Destination src/assets/player_shoot.wav -Force
Remove-Item -LiteralPath player_shoot.wav -Force
```

Repeated firing sounds must be low-fatigue. Prefer softer/lower presets for
`player_shoot.wav`; avoid harsh high-pitched laser sounds for rapid fire.
