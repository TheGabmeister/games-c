# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Project overview

Lode Runner (1983) recreation in C11 with raylib. 1200x900 window, 60 fps, 30x22 tile grid at 36px per cell. Full design spec is in `SPEC.md`.

## Build

```bash
cmake --build build
```

If the build directory is missing, configure first: `cmake -S . -B build`. The executable lands at `build/template/Debug/template.exe`. Assets are copied automatically.

## Module layout

```
main.c          Entry point, lifecycle (window, audio, textures, sounds)
game.h          Game struct, screen enum, shared includes
game.c          Game state machine, update logic (no rendering)
render.h/.c     All drawing: tiles, actors, HUD, overlays, title screen
world.h/.c      Tile grid, level loading, BFS pursuit table, shared tile queries
player.h/.c     Player movement, Actor struct, shared actor utilities
guard.h/.c      Guard AI, pathfinding, gold carry/drop
input.h/.c      Keyboard + gamepad input (standalone)
particles.h/.c  Fixed-pool particle system
sounds.h/.c     SoundID enum, load/unload/play (decoupled from Game struct)
textures.h/.c   TextureID enum, sprite loading from assets/sprites/
game_config.h   All constants (speeds, sizes, scoring, capacities)
```

## Architecture notes

- **Shared tile queries** live in `world.h`: `world_can_enter()`, `world_has_support()`, `world_tile_is_climbable()`, `world_can_step_side()`. Do not duplicate these in other modules.
- **Shared actor utilities** live in `player.h`: `actor_is_resting()`, `actor_pixel_position()`. The `Actor` struct is defined there too.
- **Sounds and textures** are decoupled from the Game struct. Their APIs take `Sound[]` / `Texture2D[]` arrays, not `Game*`.
- **Rendering is separated** from game logic. `game.c` handles update, `render.c` handles draw. Both operate on the `Game` struct.
- Player and guard modules report results via POD return structs (`PlayerTickResult`, `GuardTickResult`). `game.c` is the sole writer of score/lives/screen state.

## Coding principles

- KISS over DRY. Prefer stack/static allocation.
- Constants live in `game_config.h`. Tile size is 36px.
- Asset paths are relative to the output directory: `assets/coin.wav`, `assets/sprites/player.png`, `assets/levels/level01.txt`.
- Sprites: SVG sources in `src/assets/svg/`, PNGs in `src/assets/sprites/`. Convert via: `inkscape input.svg -o output.png -w 36 -h 36`. Inkscape is at `"C:/Program Files/Inkscape/bin/inkscape.exe"`.
- Sounds: WAV via rfxgen (`"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav`), or generated procedurally at runtime as fallback.
