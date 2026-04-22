# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Modernized Pac-Man clone written in C using raylib for rendering/input/audio. See `SPEC.md` for full game mechanics and implementation details.

## Build Commands

```bash
# Configure (from project root)
cmake -B build

# Build
cmake --build build

# Run (MSVC puts exe under Debug/)
./build/pac_man/Debug/pac_man
```

The build copies `src/resources/` into the output directory automatically.

## Architecture

- **Language:** C (no C++), linked against raylib
- **Build system:** CMake, with raylib vendored under `vendor/raylib/`
- **Source layout:** All `.c` and `.h` files live under `src/` (recursively globbed). Game assets go in `src/resources/`.
- **Window:** 750x1000, 24px tile grid (28x36), 60 FPS
- **Input:** keyboard (arrow keys, WASD) and gamepad (left stick, D-pad) simultaneously
- **Visual style:** modernized — neon glow, rounded walls, particle effects, dark background. Not pixel-art retro.

### Code structure

- `maze.h/c` — 28x36 `const int` tile grid. Tile types: `WALL`, `DOT`, `POWER_PELLET`, `EMPTY`, `GHOST_DOOR`, `TUNNEL`. This is read-only; mutable dot state lives in `Game.dot_eaten[][]`.
- `game.h` — All types (`PacMan`, `Ghost`, `Game`, enums), constants, color palette, and speed/timing tables.
- `game.c` — All game logic and rendering in one file, organized in sections: maze helpers → initialization → input → Pac-Man movement → ghost AI/movement → scatter/chase timing → frightened mode → ghost house exit → dot consumption → collision → game state machine → drawing.
- `main.c` — Entry point only: init window, loop `game_update()`/`game_draw()`, save high score on exit.

### Key design patterns

- One `Game` struct holds all state, passed by pointer. No heap allocation.
- Ghost AI uses tile-based pathfinding: at each intersection, evaluate non-reverse directions, pick smallest Euclidean distance to target. Tie-break: Up > Left > Down > Right.
- Speed tables, scatter/chase timing, and frightened durations are `static const` arrays indexed by level tier.
- Maze walkability has three variants: `maze_is_walkable` (general), `maze_is_walkable_pacman` (blocks ghost door), `maze_is_walkable_ghost` (allows ghost door for EXITING/EATEN modes).

## Coding principles

- **C game programming best practices** — prefer stack allocation over heap, use fixed-size arrays where possible, keep hot data contiguous, avoid unnecessary indirection. Global or file-scoped state is fine for game systems.
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