# AGENTS.md

Guidance for AI coding agents working in this repository.

## Project

Modernized Super Mario Bros clone written in C using raylib for rendering, input, and audio. The gameplay follows classic SMB mechanics, while visuals use modern PNG sprites created from SVGs via Inkscape.

Use `README.md` for the project overview. There is currently no `SPEC.md` in the repo, so prefer the existing code and README as the source of truth.

## Build Commands

```bash
# Configure from the project root
cmake -B build

# Build
cmake --build build

# Run on MSVC / Visual Studio generators
./build/super_mario_bros/Debug/super_mario_bros
```

The build copies `src/resources/` into the output directory automatically.

## Architecture

- Language: C only. Do not introduce C++.
- Build system: CMake.
- Rendering/input/audio: raylib, vendored under `vendor/raylib/`.
- Source layout: all `.c` and `.h` files live under `src/` and are recursively globbed.
- Assets: sprites (PNG) and sounds (WAV) live in `src/resources/`. SVG sources live in `src/resources/svg/`.
- Window: 800x600, 16px tile grid, 60 FPS.
- Input: keyboard (arrow keys, WASD, Space, Shift) and gamepad (left stick, D-pad, face buttons) simultaneously.
- Visual style: modernized clean sprites with particle effects. Not pixel-art retro.

## Code Structure

- `common.h`: shared constants, colors, types, direction helpers.
- `particles.c` / `particles.h`: fixed-size particle effects.
- `game.c` / `game.h`: top-level game state, state machine, scoring, HUD, and orchestration.
- `main.c`: window/audio initialization, main loop, cleanup.

## Design Patterns

- One `Game` struct owns top-level state and is passed by pointer.
- Prefer fixed-size arrays and stack/file-scoped state over heap allocation.
- Keep module ownership clear: `game.c` orchestrates, but movement/AI/drawing details live in their modules.
- Side-scrolling camera follows Mario. Levels are tile-based (16x16 pixels).

## Coding Principles

- KISS: prefer the simplest clear implementation. Plain control flow is usually better than clever abstraction.
- YAGNI: do not add configuration, extension points, or systems for hypothetical future needs.
- DRY: remove real duplication, but avoid extracting code just because it has a similar shape.
- Keep changes focused on the requested behavior.
- Match the existing C style and project structure before adding new patterns.

When in doubt, lean KISS over DRY. A small amount of readable repetition is better than the wrong shared helper.

## Asset Pipeline

Sprites are authored as SVGs and converted to PNGs using Inkscape:

```bash
# Convert SVG to PNG
"C:/Program Files/Inkscape/bin/inkscape.exe" input.svg -o output.png -w 16 -h 16
```

Commit both SVG sources and generated PNG files.

## Resources

Sound effects are WAV files in `src/resources/`, generated with rfxgen.

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o src/resources/sound.wav
```

Commit generated WAV files. Do not add rfxgen to the game build.

## Verification

- Run `cmake --build build` after code changes when feasible.
- If audio code changes, ensure `InitAudioDevice()` and `CloseAudioDevice()` still remain balanced.
- Avoid editing vendored raylib unless explicitly requested.
