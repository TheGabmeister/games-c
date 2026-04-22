# Pac-Man

A modernized Pac-Man clone written in C using [raylib](https://www.raylib.com/) for rendering, input, and audio.

Features the classic Pac-Man gameplay — four ghosts with unique AI personalities, scatter/chase timing, frightened mode, ghost house logic, fruit bonuses, and progressive difficulty — rendered with a modern neon aesthetic: glowing walls, particle effects, and procedural graphics (no sprite sheets).

## Screenshot

*(Add a screenshot here)*

## Building

Requires CMake and a C compiler (tested with MSVC on Windows).

```bash
cmake -B build
cmake --build build
```

The build copies `src/resources/` into the output directory automatically. raylib is vendored under `vendor/raylib/` and built alongside the project.

## Running

```bash
# MSVC puts the executable under Debug/
./build/pac_man/Debug/pac_man
```

## Controls

| Action | Keyboard | Gamepad |
|--------|----------|---------|
| Move | Arrow keys / WASD | Left stick / D-pad |
| Start game | Enter | Any face button |
| Pause | Esc / P | Start |

All input methods work simultaneously.

## Project Structure

```
src/
  common.h        Shared types, constants, colors, inline helpers
  maze.h/c        28x36 tile grid, walkability functions
  pacman.h/c      Pac-Man movement, input, animation, drawing
  ghost.h/c       Ghost AI, pathfinding, state machine, drawing
  fruit.h/c       Fruit spawning, types, scoring
  particles.h/c   Particle effects system
  game.h/c        Top-level game state, orchestration, HUD
  main.c          Entry point: window/audio init, game loop
  resources/      Sound effects (WAV, generated with rfxgen)
vendor/
  raylib/         Vendored raylib library
```

## Architecture

- **One `Game` struct** holds all state and is passed by pointer. No heap allocation.
- **Modular design:** each module (`pacman`, `ghost`, `fruit`, `particles`) owns its struct and logic. `game.c` orchestrates them, passing only the fields each module needs.
- **Tile-based movement and collision:** 28x36 grid, 24px tiles. Entities track floating-point pixel positions but collide and make decisions on tile boundaries.
- **Ghost AI:** at each intersection, evaluate non-reverse directions, pick the one with smallest Euclidean distance to the target tile. Tie-break priority: Up > Left > Down > Right. Each ghost has a unique targeting strategy (Blinky chases directly, Pinky ambushes, Inky uses Blinky's position as a reference, Clyde retreats when close).
- **Data-driven difficulty:** speed tables, scatter/chase timing, frightened durations, and ghost house thresholds are `static const` arrays indexed by level tier.
- **Procedural graphics:** all visuals are drawn with raylib primitives — no image assets. Sound effects are WAV files generated with [rfxgen](https://raylibtech.itch.io/rfxgen).

## Game States

```
TITLE -> READY -> PLAYING -> DYING -> READY (or GAME_OVER -> TITLE)
                    |
                    +-> LEVEL_COMPLETE -> READY (next level)
                    +-> PAUSED
```

## License

This project is for personal/educational use.
