# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (Visual Studio 18 2026 generator, x64)
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build --config Release

# Run
./build/Release/galaxian.exe
```

Dependencies (SDL3, SDL3_image, SDL3_mixer, SDL3_ttf) live in `vendor/` and are built via `add_subdirectory`.

When adding new `.c` files, add them to the `add_executable()` call in CMakeLists.txt.

## Architecture

Modernized Galaxian clone in C17 on SDL3. 600x800 portrait window. All gameplay rendering uses primitives (no texture assets); font rendering goes through the resource layer, preferring a bundled asset path with system-font fallback.

### Engine layer (template modules)

**platform.c/h** — SDL3 abstraction: window lifecycle, frame timing (`get_deltatime()`), renderer access (`get_renderer()`), input with edge detection (`is_key_pressed()` vs `is_key_down()`), `request_close()`. All state in an internal `Globals` struct.

**game_state.c/h** — Scene machine: register states with callbacks (`init`, `update(dt)`, `draw`, `cleanup`), switch with `game_state_switch()`. States: `STATE_MENU`, `STATE_GAMEPLAY`, `STATE_PAUSE` (reserved), `STATE_GAME_OVER`.

**resources.c/h** — Asset cache keyed by path. `res_load_texture/font/sound/music()`, `res_free_all()` at shutdown. Do not manually free cached assets.

**drawing.c/h** — Rendering helpers: `draw_texture()`, `draw_text()`, `draw_rect()`, `draw_line()`, `draw_circle()`, `draw_circle_outline()`, `draw_triangle()` (via `SDL_RenderGeometry`).

**audio.c/h** — SDL3_mixer wrappers. `audio_init/shutdown()`, `play_sound()`, `play_music()`. No audio assets in v1.

**main.c** — Entry point. Registers all three game states, starts at `STATE_MENU`. Loop: `engine_begin_frame()` → poll events → `game_state_update(dt)` → clear → `game_state_draw()` → present. Background clear color is `#090B16`. Blend mode set to `SDL_BLENDMODE_BLEND`.

### Game layer

**galaxian.h** — Central header. All constants (playfield bounds, speeds, timing, palette colors as compound-literal macros like `COL_PLAYER`), enums (`EnemyType`, `EnemyState`, `DivePathType`), structs (`Player`, `Enemy`, `Bullet`, `Particle`, `Star`, `BezierPath`, `DiffParams`), and all shared function declarations.

**galaxian.c** — Shared game logic consumed by all states:
- Session state accessors (`gx_high_score()`, `gx_last_score()`, `gx_is_new_high()`) — persists across state transitions
- Difficulty table: 5-stage progression (attack interval, bullet speed, max divers, shots per dive), clamped at stage 5
- Formation setup: populates 46 enemies across 6 rows (2 Flagship, 6 Escort, 8 Raider, 30 Drone)
- Cubic Bezier path system: `gx_dive_path()` (4 path families with random jitter), `gx_return_path()`, `bezier_eval()`
- Attack selection: `gx_pick_solo_diver()` (prefers outer-edge), `gx_pick_convoy()` (flagship + up to 2 escorts)
- Convoy scoring: `gx_convoy_score()` (200/300/800 based on escort deaths)
- Starfield: 3-layer parallax, 80 stars, shared by all states

**particles.c/h** — Stateless particle system operating on caller-owned arrays. Spawn with random angles/speeds, update with drag/decay, draw as alpha-faded circles.

**state_menu.c** — Title screen with starfield, attract-mode enemy diamonds, pulsing title, high score, controls hint.

**state_gameplay.c** — Core gameplay. Player movement/firing (one bullet max, edge-triggered), formation sway, Bezier dive attacks, convoy tracking, swarm mode (≤3 enemies or all drones/raiders gone), AABB collision, scoring (formation vs in-flight vs convoy bonuses), respawn with invulnerability (0.9s delay + 2.0s flash at 12Hz), screen shake, stage intro/clear overlays, HUD.

**state_gameover.c** — Final score display, new high score callout, restart prompt.

### Key data flow

- `galaxian.c` session state bridges data between states (score, high score, new-high flag)
- Each state manages its own starfield and loads fonts via `res_default_font_path()` + `res_load_font()` (cached)
- Gameplay entity drawing (player ship, enemy shapes, bullets) is done with static helpers in `state_gameplay.c` using `drawing.h` primitives

### Startup/shutdown order

`init_window()` → `audio_init()` → register states → main loop → `game_state_shutdown()` → `res_free_all()` → `audio_shutdown()` → `close_window()`
