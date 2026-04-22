# AGENTS.md

Guidance for AI coding agents working in this repository.

## Project

Pac-Man clone written in C using raylib for rendering, input, and audio.

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

## Coding Principles

- KISS: prefer the simplest clear implementation. Avoid clever patterns where plain control flow is enough.
- YAGNI: do not build abstractions, configuration, or extension points for hypothetical future needs.
- DRY: remove real duplication, but avoid extracting code merely because it has a similar shape.

When in doubt, lean KISS over DRY. A small amount of readable repetition is better than the wrong shared helper.

## Sprite Generation

Write SVG markup, then convert to PNG with Inkscape.

```bash
inkscape input.svg -o output.png -w 32 -h 32
```

Store both SVGs and PNGs in `src/resources/`. The build copies this folder to the output directory automatically.

## Agent Notes

- Keep changes focused on the requested behavior.
- Match existing C style and project structure before adding new patterns.
- Do not move or rename assets unless the task explicitly requires it.
- Run the relevant CMake build after code changes when feasible.
