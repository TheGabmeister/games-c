# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Scope

Project details, file layout, architecture notes, gameplay reference, and
tuning guidance live in `README.md`. Keep this file focused on agent workflow:
build steps, asset pipeline, and coding principles.

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

The executable is emitted under `build/robotron/Debug/` for the default Visual
Studio generator configuration.

## Coding principles

- **C game programming best practices** -- prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime. Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** -- simplest thing that works. No clever patterns where a plain `if` does the job.
- **YAGNI** -- don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** -- remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.

When in doubt, lean KISS over DRY.

Keep `Game` as the owner of fixed arrays. Prefer simple, contiguous, fixed-size
data over dynamic allocation or a generic entity system.

Enemy-family behavior should stay explicit in the relevant update/draw loops.
Shared enemy-family policy, such as whether a family counts for wave clear,
kills the player on contact, or uses the normal bullet-kill path, should be
centralized in small helpers rather than repeated in every caller. Do not
replace the fixed arrays with a generic ECS or object registry.

## Milestone workflow

- Work from `README.md`.
- Keep each milestone playable before adding the next one.
- Keep presentation effects subordinate to readability.
- Build after code changes with `cmake --build build`.

## Sprite Generation

Workflow: write SVG markup, then convert to PNG with Inkscape.

Assets under `src/assets/` are copied into the executable output directory by
`CMakeLists.txt`. Runtime asset paths should be relative to that output
directory, for example `assets/player.png` or `assets/player_shoot.wav`.

```bash
inkscape input.svg -o output.png -w 20 -h 20
```

Store both SVGs and PNGs in `src/assets/`. Cell size is 20px.

If a texture ID is registered in `src/textures.c`, the corresponding PNG should
exist under `src/assets/`. Keep source SVGs next to generated PNGs.

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

If a sound ID is registered in `src/sounds.c`, the corresponding WAV should
exist under `src/assets/`.

rfxgen may not write directly to nested output paths. If needed, generate in the
repo root, then copy into `src/assets/`:

```powershell
& "D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g blip -o player_shoot.wav
Copy-Item -LiteralPath player_shoot.wav -Destination src/assets/player_shoot.wav -Force
Remove-Item -LiteralPath player_shoot.wav -Force
```

Repeated firing sounds must be low-fatigue. Prefer softer/lower presets for
`player_shoot.wav`; avoid harsh high-pitched laser sounds for rapid fire.
