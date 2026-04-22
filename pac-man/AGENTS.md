# AGENTS.md

Guidance for AI coding agents working in this repository.

## Project

Modernized Pac-Man clone written in C using raylib for rendering, input, and audio. See `SPEC.md` for full game mechanics and implementation details.

## Build Commands

```bash
# Configure from the project root
cmake -B build

# Build
cmake --build build

# Run
./build/pac_man/pac_man
```

The build copies `src/resources/` into the output directory automatically.

## Architecture

- Language: C only. Do not introduce C++.
- Build system: CMake.
- Rendering/input/audio: raylib, vendored under `vendor/raylib/`.
- Source layout: all `.c` and `.h` files live under `src/` and are recursively globbed.
- Assets: game assets live in `src/resources/`.
- Window: 750x1000, 24px tile grid (28x36), 60 FPS.
- Input: keyboard (arrow keys, WASD) and gamepad (left stick, D-pad) simultaneously.
- Visual style: modernized — neon glow, rounded walls, particle effects, dark background. Not pixel-art retro.

## Coding Principles

- C game programming best practices: prefer stack allocation over heap, use fixed-size arrays where possible, keep hot data contiguous, avoid unnecessary indirection. Global or file-scoped state is fine for game systems.
- KISS: prefer the simplest clear implementation. Avoid clever patterns where plain control flow is enough.
- YAGNI: do not build abstractions, configuration, or extension points for hypothetical future needs.
- DRY: remove real duplication, but avoid extracting code merely because it has a similar shape.

When in doubt, lean KISS over DRY. A small amount of readable repetition is better than the wrong shared helper.

## Sprite Generation

Write SVG markup, then convert to PNG with Inkscape.

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

## Agent Notes

- Consult `SPEC.md` before implementing any game mechanic — it has precise values for timing, scoring, speeds, and ghost AI.
- Keep changes focused on the requested behavior.
- Match existing C style and project structure before adding new patterns.
- Do not move or rename assets unless the task explicitly requires it.
- Run the relevant CMake build after code changes when feasible.
