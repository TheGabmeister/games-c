# Robotron 2084 (raylib C recreation)

A modernized twin-stick arena shooter inspired by the 1982 arcade game, built
in C on top of raylib. The player fights waves of robots, rescues humans for
escalating bonuses, and survives across a single non-scrolling screen.

This is an original recreation. No original ROM, ripped sprites, cabinet art,
or sampled audio is used.

- Resolution: `1200x900` at `60 FPS`.
- Controls: `WASD` to move, arrow keys to fire (8-direction), `Enter` to
  start/restart, `P` to pause, `Esc` to quit. Gamepad supported (left stick
  move, right stick fire).

## Project Files

```
src/
  main.c              raylib init, main loop, shutdown
  game_config.h       compile-time tuning constants
  game.h / game.c     enums, structs, central Game state, top-level state
                      transitions, HUD, overlays
  world.h / world.c   playfield simulation entry points
  world_internal.h    shared declarations between world_*.c files
  world_entities.c    enemy / human / spawner update + AI
  world_projectiles.c bullets, sparks, shells, cruise missiles
  world_collisions.c  circle-vs-circle resolution and ordering
  world_effects.c     particles, screen shake, floating score text
  world_draw.c        sprite + primitive rendering for the playfield
  textures.h / .c     required PNG sprite loading
  sounds.h / .c       optional WAV sound loading
  assets/             source SVGs, generated PNGs, generated WAVs
vendor/raylib/        bundled raylib source
CMakeLists.txt        CMake project file
AGENTS.md             agent-facing build, asset, and coding notes
```

## Architecture

`Game` (in [src/game.h](src/game.h)) owns fixed-size arrays for enemies,
humans, bullets, projectiles, electrodes, particles, and floating score text.
The code prefers contiguous, fixed-capacity data over dynamic allocation or a
generic ECS.

`world.c` coordinates playfield update order: player/bullets, humans, enemy
families, projectiles, effects, collision phases, wave-clear checks, and draw
entry. The specialized `world_*.c` files keep behavior grouped by concern
without moving ownership away from `Game`.

Texture loading fails loud when a registered PNG is missing or invalid. Sound
loading is resilient; missing sounds are skipped and gameplay continues
silently.

## Where To Edit

- **Tuning numbers** (player speed, fire rate, bullet lifetime, max counts,
  scoring values, spawn timers, wave speed scaling, presentation timings):
  [src/game_config.h](src/game_config.h).
- **Game state machine** (title, wave intro, playing, paused, player dead,
  game over): [src/game.c](src/game.c).
- **Wave contents and difficulty scaling**: wave-building functions in
  [src/world.c](src/world.c) / [src/world_entities.c](src/world_entities.c).
- **Enemy AI** (Grunt, Hulk, Spheroid, Enforcer, Quark, Tank, Brain, Prog):
  [src/world_entities.c](src/world_entities.c).
- **Bullets and enemy projectiles** (sparks, bouncing shells, homing cruise
  missiles): [src/world_projectiles.c](src/world_projectiles.c).
- **Collision order and rules**: [src/world_collisions.c](src/world_collisions.c).
- **Particles, screen shake, hit flash, floating score text**:
  [src/world_effects.c](src/world_effects.c).
- **Sprite/primitive rendering**: [src/world_draw.c](src/world_draw.c).
- **Texture / sound IDs and load lists**:
  [src/textures.c](src/textures.c) and [src/sounds.c](src/sounds.c).

## Gameplay Reference

- **Wave clear**: all non-Hulk enemies destroyed. Remaining Hulks, humans,
  electrodes, bullets, and hostile projectiles are removed during the
  transition.
- **Human rescue ladder**: `1000`, `2000`, `3000`, `4000`, then `5000` each.
  Resets on new wave and on player death.
- **Lives**: start at `3`, extra life every `25000` points.
- **Brain waves**: every fifth wave. Brains convert humans to Progs and fire
  homing cruise missiles.
- **Electrodes**: kill the player and Grunts on contact, can be shot for `0`
  points.
- **Hulks**: invulnerable, hunt humans, knocked back / slowed by bullets, do
  not block wave clear.
