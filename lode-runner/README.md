# Lode Runner

A modernized recreation of Doug Smith's *Lode Runner* (1983, Broderbund), built in C with [raylib](https://www.raylib.com/).

You play a treasure hunter trapped in a multi-storey vault. Each level is a static screen of bricks, ladders, and rope-bars. Collect every gold piece, then escape through a hidden ladder that appears at the top of the screen. Guards from the Bungeling Empire pursue you using a deterministic AI that takes the shortest traversal-graph path -- not the straight-line path -- producing routes that look deliberate but occasionally counter-intuitive.

You cannot jump or attack directly. Your only offensive tool is **digging** a temporary hole into a brick to pass through it or trap a guard. Dug bricks refill after 6 seconds, killing anything still inside.

## Build

```bash
cmake --build build
```

If the build directory is missing, configure first: `cmake -S . -B build`

The executable lands at `build/template/Debug/template.exe`. Assets are copied to `build/template/Debug/assets/` automatically by CMake.

## Architecture

**Module dependency rule (one direction only):**
```
main.c ──> game.c, render.c, sounds.c, textures.c
game.c ──> player.c, guard.c, world.c, input.c, particles.c, sounds.c
render.c ──> game.h, player.h, guard.h, particles.c, textures.h
player.c ──> world.c, input.c
guard.c ──> world.c (reads Player as const for tile position)
input.c ──> (standalone, no game dependencies)
```

`player.c` and `guard.c` do NOT know about each other or `ScreenID`. They report what happened via POD return structs (`PlayerTickResult`, `GuardTickResult`), and `game.c` is the sole writer of score/lives/screen state. Rendering is fully separated in `render.c`.

**Key design patterns:**
- **Tile-based movement with continuous interpolation**: actors are always resting on a tile or traversing between two adjacent tiles. `Actor.t` interpolates 0→1; input/AI re-evaluate only on tile arrival.
- **Shared tile/actor queries**: movement predicates (`world_can_enter`, `world_has_support`, etc.) live in `world.h`; actor utilities (`actor_is_resting`, `actor_pixel_position`) live in `player.h`. No duplication across modules.
- **No dynamic allocation in the game loop**: particles use a fixed pool (`MAX_PARTICLES`), guards use a fixed array (`MAX_GUARDS`). Heap allocation is only in init (texture/sound loading).
- **Sprite-based rendering**: tiles and actors are drawn from 36×36 PNG sprites (SVG sources in `src/assets/svg/`). Animated effects (gold sparkle, hole warnings, exit reveal) are overlaid procedurally.
- **Procedural sound fallback**: `sounds.c` generates sine-wave tones at init when WAV files are missing. Existing WAV files take priority.

**Guard AI**: BFS flood-fill from player tile builds a pursuit table (`PursuitDir[GRID_ROWS][GRID_COLS]`). Guards score candidate directions using pursuit match (+100), vertical/horizontal bias, stacking penalty (-15), and per-guard jitter. Table is rebuilt when the player commits a new tile or holes open/close.

## Gameplay

- **10 hand-designed levels** with progressive difficulty: tutorial (no guards) through a boss-feel finale with 4 guards and tight passages.
- **Guard AI** uses BFS pursuit with stacking penalties, producing emergent behaviors like splitting around obstacles and moving away on shared ladders.
- Guards can pick up gold (50% chance), carry it, and drop it on death, when falling into holes, or randomly. Gold in a guard's pocket doesn't count toward level completion -- you must collect every piece yourself.
- After clearing all 10 levels, the game shows a victory screen and loops back to level 1 with score and lives preserved.

## Controls

| Action | Keyboard | Gamepad |
|---|---|---|
| Move | Arrows / WASD | D-pad / Left stick |
| Dig left | Z / J | LB |
| Dig right | X / K | RB |
| Pause | P | Start |
| Restart level (costs a life) | R | Back |
| Confirm (menus) | Enter | A |
| Back / Quit | Esc | B |

## Building

Requires CMake 3.24+ and a C11 compiler. raylib is vendored.

```bash
cmake -S . -B build
cmake --build build
```

Run the executable from `build/template/Debug/`.

## Level format

Levels are plain-text files in `src/assets/levels/`, one character per tile, 30 columns by 22 rows:

```
.  Empty          B  Brick (diggable)    S  Solid (indestructible)
H  Ladder         -  Rope                $  Gold
E  Exit ladder    T  Trapdoor            P  Player spawn (exactly 1)
G  Guard spawn
```

The number of `G` tiles determines the guard count for that level. At least one `E` tile must appear in the top 3 rows.

## Visual and audio

- **Sprite-based rendering**: all tiles and actors use 36×36 PNG sprites (SVG sources in `src/assets/svg/`, converted via Inkscape)
- Pulsing sparkle overlay on gold sprites
- Dig dust particles with additive blending
- Exit ladder sprite fade-in when all gold is collected
- Camera shake on player death
- Scrolling parallax cave-wall background (procedurally generated)
- Brick refill warning: orange pulse at 1.5s remaining, red flash at 0.5s
- Player and guard sprites flip horizontally based on facing direction, tinted by state
- Procedurally generated sound effects as fallback when WAV files are missing

## Scoring

| Event | Points |
|---|---|
| Gold collected | +100 |
| Guard killed by refill | +250 |
| Level cleared | +1500 |

Start with 5 lives. +1 life per level cleared.

## Sources

- [Lode Runner -- Wikipedia](https://en.wikipedia.org/wiki/Lode_Runner)
- [Lode Runner Gameplay -- StrategyWiki](https://strategywiki.org/wiki/Lode_Runner/Gameplay)
- [Championship Lode Runner: Guard psychology -- Data Driven Gamer](https://datadrivengamer.blogspot.com/2023/01/championship-lode-runner-guard.html)
- [Classic Lode Runner controls -- entropymine.com](https://entropymine.com/jason/lr/misc/controls.html)
