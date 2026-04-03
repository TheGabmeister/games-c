# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (Visual Studio 18 2026 generator, x64)
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build --config Release

# Run
./build/Release/hello_sdl.exe
```

Dependencies (SDL3, SDL3_image, SDL3_mixer, SDL3_ttf) live in `vendor/` and are built via `add_subdirectory`.

When adding new `.c` files, add them to the `add_executable()` call in CMakeLists.txt.

## Architecture

SDL3 game template in C17 with a scalable module structure.

**platform.c/h** — Abstraction layer over SDL3:
- Window lifecycle: `init_window()`, `close_window()`, `window_should_close()`
- Frame timing: `get_deltatime()`, `get_fps()`
- Rendering access: `get_renderer()` returns the SDL_Renderer (no direct access to globals)
- Input polling with edge detection: `is_key_pressed()` (single-frame) vs `is_key_down()` (held), same for mouse
- All state in a single internal `Globals` struct — not exposed externally

**game_state.c/h** — Scene/state machine:
- Register states with `game_state_register(STATE_ID, callbacks)`
- Each state has `init`, `update(dt)`, `draw`, `cleanup` callbacks (any may be NULL)
- Switch states with `game_state_switch()` — calls cleanup on old, init on new
- Main loop calls `game_state_update(dt)` and `game_state_draw()`

**resources.c/h** — Asset cache (load once, lookup by path):
- `res_load_texture()`, `res_load_font()`, `res_load_sound()`, `res_load_music()`
- `res_free_all()` at shutdown — do not manually free cached assets
- Audio loading requires `audio_init()` first

**drawing.c/h** — Rendering helpers using `get_renderer()`:
- `draw_texture()`, `draw_texture_region()`, `draw_texture_ex()`
- `draw_text()` — creates texture per call (fine for dynamic text)
- `draw_rect()`, `draw_rect_outline()`, `draw_line()`, `draw_circle()`

**audio.c/h** — SDL3_mixer 3.x wrappers:
- `audio_init()` / `audio_shutdown()` manage the MIX_Mixer
- `play_sound()` — fire-and-forget via `MIX_PlayAudio()`
- `play_music()` / `stop_music()` / `pause_music()` — dedicated music track
- `get_mixer()` for direct SDL3_mixer API access

**main.c** — Entry point wiring everything together. Game loop order:
1. `engine_begin_frame()` — snapshots input
2. `SDL_PollEvent()` + `engine_process_event()` — SDL events
3. `get_deltatime()` → `game_state_update(dt)`
4. Clear → `game_state_draw()` → `SDL_RenderPresent()`
5. Loop exits when `window_should_close()` returns true

**Startup/shutdown order**: `init_window()` → `audio_init()` → register states → main loop → `game_state_shutdown()` → `res_free_all()` → `audio_shutdown()` → `close_window()`
